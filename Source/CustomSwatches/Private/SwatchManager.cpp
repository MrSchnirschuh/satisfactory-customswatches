#include "SwatchManager.h"
#include "AutoSwatchSystem.h"
#include "PaintModes.h"

#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Buildables/FGBuildableGenerator.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableRailroadTrack.h"
#include "Buildables/FGBuildableVehicle.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "FGGameUserSettings.h"
#include "FGColoredInstanceMesh.h"
#include "FGBuildableSubsystem.h"
#include "FGPlayerController.h"
#include "FGHUD.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunPaint.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Json.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// File path for user swatch config
static FString GetSwatchConfigPath()
{
    return FPaths::ProjectSavedDir() / TEXT("CustomSwatches") / TEXT("user_swatches.json");
}

// Singleton
static USwatchManager* ManagerInstance = nullptr;

USwatchManager* USwatchManager::Instance()
{
    if (!ManagerInstance)
    {
        ManagerInstance = NewObject<USwatchManager>();
        ManagerInstance->AddToRoot(); // prevent GC
        ManagerInstance->ActivePaintMode = EBlueprintPaintMode::Default;
        ManagerInstance->UnpaintedMarkerSlotIndex = -1; // -1 = disabled, user must set
        
        // Reserve space
        ManagerInstance->CustomSwatches.Reserve(MaxTotalSwatches - BaseGameSwatchCount);
    }
    return ManagerInstance;
}

void USwatchManager::Initialize()
{
    USwatchManager* Mgr = Instance();
    
    // Register default category -> swatch mappings
    Mgr->RegisterDefaultCategoryMappings();
    
    // Load user-defined swatches from disk
    ReloadUserSwatches();
    
    // Hook into the paint system
    HookPaintSystem();
    
    // Initialize subsystems
    AutoSwatchSystem::Initialize();
    PaintModes::Initialize();
    
    // Optionally run auto-assignment on existing buildables
    ApplyAutoAssignmentToAll();
    
    UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Initialized. %d custom swatches loaded."),
        Mgr->CustomSwatches.Num());
}

int32 USwatchManager::RegisterCustomSwatch(const FSwatchDefinition& Definition)
{
    USwatchManager* Mgr = Instance();
    
    int32 CurrentCount = BaseGameSwatchCount + Mgr->CustomSwatches.Num();
    if (CurrentCount >= MaxTotalSwatches)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CustomSwatches] Cannot register more than %d swatches!"), MaxTotalSwatches);
        return -1;
    }
    
    int32 NewIndex = BaseGameSwatchCount + Mgr->CustomSwatches.Num();
    Mgr->CustomSwatches.Add(Definition);
    
    UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Registered custom swatch at slot %d"), NewIndex);
    return NewIndex;
}

int32 USwatchManager::GetTotalSwatchCount()
{
    USwatchManager* Mgr = Instance();
    return BaseGameSwatchCount + Mgr->CustomSwatches.Num();
}

FSwatchDefinition USwatchManager::GetSwatchForSlot(int32 SlotIndex)
{
    USwatchManager* Mgr = Instance();
    
    if (SlotIndex < BaseGameSwatchCount)
    {
        // TODO: Read from game's actual color slot definitions
        // We return a default here; the actual colors are in UFGColorSlot
        FSwatchDefinition Def;
        Def.DisplayName = FText::FromString(FString::Printf(TEXT("Default Slot %d"), SlotIndex));
        Def.bIsUserDefined = false;
        return Def;
    }
    
    int32 CustomIndex = SlotIndex - BaseGameSwatchCount;
    if (CustomIndex >= 0 && CustomIndex < Mgr->CustomSwatches.Num())
    {
        return Mgr->CustomSwatches[CustomIndex];
    }
    
    return FSwatchDefinition();
}

void USwatchManager::UpdateSwatch(int32 SlotIndex, const FSwatchDefinition& NewDefinition)
{
    USwatchManager* Mgr = Instance();
    
    if (SlotIndex < BaseGameSwatchCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CustomSwatches] Cannot modify built-in swatch slot %d"), SlotIndex);
        return;
    }
    
    int32 CustomIndex = SlotIndex - BaseGameSwatchCount;
    if (CustomIndex >= 0 && CustomIndex < Mgr->CustomSwatches.Num())
    {
        Mgr->CustomSwatches[CustomIndex] = NewDefinition;
    }
}

bool USwatchManager::RemoveCustomSwatch(int32 SlotIndex)
{
    USwatchManager* Mgr = Instance();
    
    if (SlotIndex < BaseGameSwatchCount) return false;
    
    int32 CustomIndex = SlotIndex - BaseGameSwatchCount;
    if (CustomIndex >= 0 && CustomIndex < Mgr->CustomSwatches.Num())
    {
        Mgr->CustomSwatches.RemoveAt(CustomIndex);
        
        // If we removed the unpainted marker, reset it
        if (Mgr->UnpaintedMarkerSlotIndex == SlotIndex)
        {
            Mgr->UnpaintedMarkerSlotIndex = -1;
        }
        return true;
    }
    return false;
}

EBuildingCategory USwatchManager::GetCategoryForBuildable(AActor* Buildable)
{
    return AutoSwatchSystem::DetermineCategory(Buildable);
}

bool USwatchManager::AutoAssignSwatch(AActor* Buildable)
{
    return AutoSwatchSystem::ApplyAutoSwatch(Buildable);
}

int32 USwatchManager::GetSwatchSlotForCategory(EBuildingCategory Category)
{
    USwatchManager* Mgr = Instance();
    
    if (const int32* Slot = Mgr->CategorySlotMap.Find(Category))
    {
        return *Slot;
    }
    return -1; // no mapping -> don't auto-assign
}

void USwatchManager::SetCategoryMapping(EBuildingCategory Category, int32 SlotIndex)
{
    USwatchManager* Mgr = Instance();
    Mgr->CategorySlotMap[Category] = SlotIndex;
}

TMap<EBuildingCategory, int32> USwatchManager::GetCategoryMappings()
{
    return Instance()->CategorySlotMap;
}

void USwatchManager::RegisterDefaultCategoryMappings()
{
    USwatchManager* Mgr = Instance();
    
    // Default mappings: each category -> preferred game swatch slot
    // Game swatch slots 0-7: standard palette (0=blue default, 1=orange, 2=red, 3=green, 4=purple, 5=white, 6=yellow, 7=black)
    // Slot 8: Vehicle (extra)
    // Slot 9: Pipes (extra)
    // Slot 10: Custom 1
    // Slot 11: Custom 2
    // We'll map categories to these + our custom ones
    
    Mgr->CategorySlotMap.Add(EBuildingCategory::None, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Power, -1);       // user-configurable, default -1 = no auto
    Mgr->CategorySlotMap.Add(EBuildingCategory::PowerGen, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Production, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Logistics, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Storage, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Transport, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Foundations, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Architecture, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Miner, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Defense, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Custom_00, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Custom_01, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Custom_02, -1);
    Mgr->CategorySlotMap.Add(EBuildingCategory::Custom_03, -1);
}

void USwatchManager::HookPaintSystem()
{
    // ============================================================
    // Hook 1: AFGBuildGunPaint::PaintBuildable
    // Intercept paint calls to redirect through paint modes
    // ============================================================
    
#if !WITH_EDITOR
    // In a real mod, we'd hook AFGBuildGunPaint like this:
    // SUBSCRIBE_METHOD_AFTER(AFGBuildGunPaint::PaintBuildable, [](auto& Scope, AFGBuildGunPaint* Gun, AActor* Actor)
    // {
    //     // If we have a non-default paint mode active, redirect
    //     USwatchManager* Mgr = Instance();
    //     if (Mgr->ActivePaintMode != EBlueprintPaintMode::Default)
    //     {
    //         // Get the selected swatch slot
    //         int32 Slot = Gun->GetSelectedColorSlot();
    //         
    //         // Apply with mode logic
    //         int32 Painted = PaintModes::PaintWithMode(Actor, Slot, Mgr->ActivePaintMode);
    //         
    //         // Cancel base paint if we handled it
    //         if (Painted > 0)
    //         {
    //             Scope.Cancel();
    //         }
    //     }
    // });
#endif
}

void USwatchManager::ApplyAutoAssignmentToAll()
{
#if !WITH_EDITOR
    // Get the game world
    // For each UWorld, find AFGBuildableSubsystem and iterate
    // In SML: subscribe to AFGBuildableSubsystem::PostLoad or use World subsystem
    
    // See AutoSwatchSystem::ApplyToAllBuildables
#endif
}

// --- Paint Mode API ---

void USwatchManager::SetPaintMode(EBlueprintPaintMode NewMode)
{
    Instance()->ActivePaintMode = NewMode;
    
    UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Paint mode set to %d"), (int32)NewMode);
    
    // TODO: Update HUD to show the current mode
}

EBlueprintPaintMode USwatchManager::GetPaintMode()
{
    return Instance()->ActivePaintMode;
}

int32 USwatchManager::ApplySwatchWithMode(AActor* Actor, int32 TargetSlot, EBlueprintPaintMode Mode)
{
    return PaintModes::PaintWithMode(Actor, TargetSlot, Mode);
}

int32 USwatchManager::GetUnpaintedMarkerSlot()
{
    return Instance()->UnpaintedMarkerSlotIndex;
}

void USwatchManager::SetUnpaintedMarkerSlot(int32 SlotIndex)
{
    Instance()->UnpaintedMarkerSlotIndex = SlotIndex;
}

// --- Config loading/saving ---

void USwatchManager::ReloadUserSwatches()
{
    USwatchManager* Mgr = Instance();
    
    FString FilePath = GetSwatchConfigPath();
    FString JsonStr;
    
    if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] No user swatch config found at %s"), *FilePath);
        return;
    }
    
    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
    
    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[CustomSwatches] Failed to parse user swatch config"));
        return;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* SwatchesArray;
    if (!RootObj->TryGetArrayField(TEXT("swatches"), SwatchesArray))
    {
        return;
    }
    
    for (const auto& Val : *SwatchesArray)
    {
        const TSharedPtr<FJsonObject>* Obj;
        if (!Val->TryGetObject(Obj)) continue;
        
        FSwatchDefinition Def;
        Def.DisplayName = FText::FromString((*Obj)->GetStringField(TEXT("name")));
        Def.PrimaryColor = FLinearColor(
            (float)(*Obj)->GetNumberField(TEXT("r")),
            (float)(*Obj)->GetNumberField(TEXT("g")),
            (float)(*Obj)->GetNumberField(TEXT("b")),
            1.0f
        );
        Def.SecondaryColor = FLinearColor(
            (float)(*Obj)->GetNumberField(TEXT("sr")),(float)(*Obj)->GetNumberField(TEXT("sg")),(float)(*Obj)->GetNumberField(TEXT("sb")),1.0f);
        Def.EmissiveIntensity = (float)(*Obj)->GetNumberField(TEXT("emissive"));
        Def.Roughness = (float)(*Obj)->GetNumberField(TEXT("roughness"));
        Def.bIsUserDefined = true;
        
        if ((*Obj)->HasField(TEXT("category")))
        {
            Def.AutoAssignCategory = (*Obj)->GetStringField(TEXT("category"));
        }
        
        RegisterCustomSwatch(Def);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Loaded %d user-defined swatches"), SwatchesArray->Num());
}

void USwatchManager::SaveUserSwatches()
{
    USwatchManager* Mgr = Instance();
    
    TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
    TArray<TSharedPtr<FJsonValue>> SwatchesArray;
    
    for (const FSwatchDefinition& Def : Mgr->CustomSwatches)
    {
        if (!Def.bIsUserDefined) continue;
        
        TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject());
        Obj->SetStringField(TEXT("name"), Def.DisplayName.ToString());
        Obj->SetNumberField(TEXT("r"), Def.PrimaryColor.R);
        Obj->SetNumberField(TEXT("g"), Def.PrimaryColor.G);
        Obj->SetNumberField(TEXT("b"), Def.PrimaryColor.B);
        Obj->SetNumberField(TEXT("sr"), Def.SecondaryColor.R);
        Obj->SetNumberField(TEXT("sg"), Def.SecondaryColor.G);
        Obj->SetNumberField(TEXT("sb"), Def.SecondaryColor.B);
        Obj->SetNumberField(TEXT("emissive"), Def.EmissiveIntensity);
        Obj->SetNumberField(TEXT("roughness"), Def.Roughness);
        Obj->SetStringField(TEXT("category"), Def.AutoAssignCategory);
        
        SwatchesArray.Add(MakeShareable(new FJsonValueObject(Obj)));
    }
    
    RootObj->SetArrayField(TEXT("swatches"), SwatchesArray);
    
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);
    
    FString FilePath = GetSwatchConfigPath();
    FFileHelper::SaveStringToFile(JsonStr, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Saved %d user-defined swatches to %s"),
        SwatchesArray.Num(), *FilePath);
}
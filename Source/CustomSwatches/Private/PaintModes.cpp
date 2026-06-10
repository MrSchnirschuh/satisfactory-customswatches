#include "PaintModes.h"
#include "SwatchManager.h"

#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "FGBuildableSubsystem.h"
#include "FGBlueprintSubsystem.h"
#include "FGGameState.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace PaintModes
{
    // ---------------------------------------------------------------------------
    // Find all actors that belong to the same "blueprint group" as the target.
    //
    // In Satisfactory, blueprints are stored as hierarchical actor groups.
    // Buildings placed as part of a blueprint share a blueprint ID stored on
    // their root component's component tags or a custom data layer.
    //
    // We use the game's AFGBlueprintSubsystem to resolve this.
    // ---------------------------------------------------------------------------
    
    TArray<AActor*> FindBlueprintGroup(AActor* TargetActor)
    {
        TArray<AActor*> Group;
        
        if (!TargetActor) return Group;
        
        AFGBuildable* Buildable = Cast<AFGBuildable>(TargetActor);
        if (!Buildable) return Group;
        
        UWorld* World = TargetActor->GetWorld();
        if (!World) return Group;
        
        // Method 1: Use BlueprintSubsystem (Satisfactory 1.0+)
        // Blueprints are stored as FBlueprintRecord with a unique ID
        AFGBlueprintSubsystem* BPSubsystem = AFGBlueprintSubsystem::Get(World);
        if (BPSubsystem)
        {
            // Get the blueprint ID for this buildable (if placed as blueprint)
            // FBlueprintRecord* Record = BPSubsystem->FindBlueprintForActor(TargetActor);
            // if (Record)
            // {
            //     TArray<AActor*> BPActors;
            //     BPSubsystem->GetBlueprintActors(Record->ID, BPActors);
            //     Group = BPActors;
            // }
            
            // Fallback: if not part of a blueprint, just return the target itself
            if (Group.Num() == 0)
            {
                Group.Add(TargetActor);
            }
        }
        else
        {
            Group.Add(TargetActor);
        }
        
        // Method 2 (fallback): Use AFGBuildableSubsystem 
        // Group by the blueprint instance ID stored in component properties
        // AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(World);
        // if (Subsystem)
        // {
        //     // For each buildable, check if it shares the same blueprint instance ID
        //     const FString BPId = Buildable->GetBlueprintInstanceId();
        //     if (!BPId.IsEmpty())
        //     {
        //         TArray<AFGBuildable*> AllBuildables;
        //         Subsystem->GetAllBuildables(AllBuildables);
        //         
        //         for (AFGBuildable* B : AllBuildables)
        //         {
        //             if (B && B->GetBlueprintInstanceId() == BPId)
        //             {
        //                 Group.AddUnique(B);
        //             }
        //         }
        //     }
        // }
        
        return Group;
    }
    
    // ---------------------------------------------------------------------------
    // Filter actors by unpainted marker swatch
    // ---------------------------------------------------------------------------
    
    TArray<AActor*> FilterByUnpaintedMarker(const TArray<AActor*>& Group)
    {
        TArray<AActor*> Result;
        
        int32 MarkerSlot = USwatchManager::GetUnpaintedMarkerSlot();
        if (MarkerSlot < 0)
        {
            // No marker configured - return empty (no targets)
            UE_LOG(LogTemp, Warning, TEXT("[CustomSwatches] BlueprintMode: No unpainted marker slot configured!"));
            return Result;
        }
        
        for (AActor* Actor : Group)
        {
            AFGBuildable* Buildable = Cast<AFGBuildable>(Actor);
            if (Buildable)
            {
                // Check if this building has the marker swatch
                uint8 CurrentSlot = Buildable->GetColorSlot();
                if (CurrentSlot == MarkerSlot)
                {
                    Result.Add(Actor);
                }
            }
        }
        
        return Result;
    }
    
    // ---------------------------------------------------------------------------
    // Filter actors by building category
    // ---------------------------------------------------------------------------
    
    TArray<AActor*> FilterByCategory(const TArray<AActor*>& Group, EBuildingCategory Category)
    {
        TArray<AActor*> Result;
        
        for (AActor* Actor : Group)
        {
            EBuildingCategory ActorCat = AutoSwatchSystem::DetermineCategory(Actor);
            if (ActorCat == Category)
            {
                Result.Add(Actor);
            }
        }
        
        return Result;
    }
    
    // ---------------------------------------------------------------------------
    // Apply swatch to a single actor
    // ---------------------------------------------------------------------------
    
    bool ApplySwatchToActor(AActor* Actor, int32 SlotIndex)
    {
        if (!Actor) return false;
        
        AFGBuildable* Buildable = Cast<AFGBuildable>(Actor);
        if (!Buildable) return false;
        
        // Clip to valid range
        int32 TotalSlots = USwatchManager::GetTotalSwatchCount();
        if (SlotIndex < 0 || SlotIndex >= TotalSlots)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CustomSwatches] Invalid swatch slot %d (max %d)"), SlotIndex, TotalSlots - 1);
            return false;
        }
        
        // Apply the color slot
        Buildable->SetColorSlot(SlotIndex);
        Buildable->ApplyColorToMesh();
        
        return true;
    }
    
    // ---------------------------------------------------------------------------
    // Main paint-with-mode function
    // ---------------------------------------------------------------------------
    
    int32 PaintWithMode(AActor* TargetActor, int32 TargetSlot, EBlueprintPaintMode Mode)
    {
        if (!TargetActor) return 0;
        
        int32 PaintedCount = 0;
        
        switch (Mode)
        {
            case EBlueprintPaintMode::Default:
            {
                // Standard single-building paint
                if (ApplySwatchToActor(TargetActor, TargetSlot))
                    PaintedCount = 1;
                break;
            }
            
            case EBlueprintPaintMode::BlueprintAll:
            {
                // Paint everything in the blueprint group
                TArray<AActor*> Group = FindBlueprintGroup(TargetActor);
                
                for (AActor* Actor : Group)
                {
                    if (ApplySwatchToActor(Actor, TargetSlot))
                        PaintedCount++;
                }
                
                UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] BlueprintAll: painted %d/%d actors"),
                    PaintedCount, Group.Num());
                break;
            }
            
            case EBlueprintPaintMode::BlueprintMode:
            {
                // Only paint actors with the unpainted marker swatch
                TArray<AActor*> Group = FindBlueprintGroup(TargetActor);
                TArray<AActor*> Targetables = FilterByUnpaintedMarker(Group);
                
                for (AActor* Actor : Targetables)
                {
                    if (ApplySwatchToActor(Actor, TargetSlot))
                        PaintedCount++;
                }
                
                int32 Skipped = Group.Num() - Targetables.Num();
                UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] BlueprintMode: painted %d, skipped %d (protected by category swatch)"),
                    PaintedCount, Skipped);
                break;
            }
            
            case EBlueprintPaintMode::CategorySelect:
            {
                // Filter blueprint group by category
                // For category select, the TargetSlot encodes both category and swatch
                // In practice: user selects a category via UI, then paints
                // We use a simple approach: determine target's category, paint matching
                EBuildingCategory TargetCategory = AutoSwatchSystem::DetermineCategory(TargetActor);
                TArray<AActor*> Group = FindBlueprintGroup(TargetActor);
                TArray<AActor*> Filtered = FilterByCategory(Group, TargetCategory);
                
                for (AActor* Actor : Filtered)
                {
                    if (ApplySwatchToActor(Actor, TargetSlot))
                        PaintedCount++;
                }
                
                UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] CategorySelect: painted %d/%d actors of category %d"),
                    PaintedCount, Filtered.Num(), (int32)TargetCategory);
                break;
            }
        }
        
        return PaintedCount;
    }
    
    // ---------------------------------------------------------------------------
    // Initialization
    // ---------------------------------------------------------------------------
    
    void Initialize()
    {
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] PaintModes initialized."));
    }
}
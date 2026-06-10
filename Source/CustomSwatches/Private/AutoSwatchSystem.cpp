#include "AutoSwatchSystem.h"
#include "SwatchManager.h"

#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableGenerator.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Buildables/FGBuildablePowerTower.h"
#include "Buildables/FGBuildableCircuitBridge.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildableStorage.h"
#include "Buildables/FGBuildableSplitter.h"
#include "Buildables/FGBuildableTrainPlatform.h"
#include "Buildables/FGBuildableRailroadTrack.h"
#include "Buildables/FGBuildableVehicle.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"
#include "Buildables/FGBuildablePillar.h"
#include "Buildables/FGBuildableStair.h"
#include "Buildables/FGBuildableWalkway.h"
#include "Buildables/FGBuildableDecor.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Buildables/FGBuildableResourceSink.h"
#include "Buildables/FGBuildableFrackingActivator.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "Buildables/FGBuildableSignBase.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableDroneStation.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildableJumpPad.h"
#include "Buildables/FGBuildablePowerStorage.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"
#include "Buildables/FGBuildableSnowCannon.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "Engine/World.h"

namespace AutoSwatchSystem
{
    // ---------------------------------------------------------------------------
    // Category determination - the heart of the auto-swatch system
    // ---------------------------------------------------------------------------
    
    EBuildingCategory DetermineCategory(AActor* Buildable)
    {
        if (!Buildable) return EBuildingCategory::None;
        
        UClass* Class = Buildable->GetClass();
        
        // --- POWER INFRASTRUCTURE ---
        // Power poles, towers, circuit bridges, priority switches
        if (Class->IsChildOf(AFGBuildablePowerPole::StaticClass()) ||
            Class->IsChildOf(AFGBuildablePriorityPowerSwitch::StaticClass()))
        {
            return EBuildingCategory::Power;
        }
        
        if (Class->IsChildOf(AFGBuildablePowerTower::StaticClass()))
        {
            return EBuildingCategory::Power;
        }
        
        if (Class->IsChildOf(AFGBuildableCircuitBridge::StaticClass()))
        {
            return EBuildingCategory::Power;
        }
        
        if (Class->IsChildOf(AFGBuildablePowerStorage::StaticClass()))
        {
            return EBuildingCategory::Power;
        }
        
        // --- POWER GENERATION ---
        // Coal, fuel, nuclear, geothermal generators
        if (Class->IsChildOf(AFGBuildableGenerator::StaticClass()))
        {
            return EBuildingCategory::PowerGen;
        }
        
        // --- MINERS / EXTRACTORS ---
        if (Class->IsChildOf(AFGBuildableResourceExtractor::StaticClass()))
        {
            return EBuildingCategory::Miner;
        }
        
        if (Class->IsChildOf(AFGBuildableFrackingActivator::StaticClass()))
        {
            return EBuildingCategory::Miner;
        }
        
        // --- PRODUCTION ---
        // Smelters, constructors, assemblers, manufacturers, refineries, particle accelerators, etc.
        if (Class->IsChildOf(AFGBuildableManufacturer::StaticClass()))
        {
            return EBuildingCategory::Production;
        }
        
        // Catch-all for other factories (packagers, blenders, etc.)
        if (Class->IsChildOf(AFGBuildableFactory::StaticClass()))
        {
            return EBuildingCategory::Production;
        }
        
        // --- LOGISTICS ---
        if (Class->IsChildOf(AFGBuildableConveyorBase::StaticClass()))
        {
            return EBuildingCategory::Logistics;
        }
        
        if (Class->IsChildOf(AFGBuildablePipeline::StaticClass()))
        {
            return EBuildingCategory::Logistics;
        }
        
        if (Class->IsChildOf(AFGBuildableSplitter::StaticClass()))
        {
            return EBuildingCategory::Logistics;
        }
        
        if (Class->IsChildOf(AFGBuildableResourceSink::StaticClass()))
        {
            return EBuildingCategory::Logistics;
        }
        
        // --- STORAGE ---
        if (Class->IsChildOf(AFGBuildableStorage::StaticClass()))
        {
            return EBuildingCategory::Storage;
        }
        
        // --- TRANSPORT ---
        if (Class->IsChildOf(AFGBuildableTrainPlatform::StaticClass()) ||
            Class->IsChildOf(AFGBuildableRailroadTrack::StaticClass()) ||
            Class->IsChildOf(AFGBuildableVehicle::StaticClass()) ||
            Class->IsChildOf(AFGBuildableDroneStation::StaticClass()))
        {
            return EBuildingCategory::Transport;
        }
        
        // Hypertubes, jump pads
        if (Class->IsChildOf(AFGBuildablePipeHyper::StaticClass()) ||
            Class->IsChildOf(AFGBuildableJumpPad::StaticClass()))
        {
            return EBuildingCategory::Transport;
        }
        
        // --- FOUNDATIONS ---
        if (Class->IsChildOf(AFGBuildableFoundation::StaticClass()))
        {
            return EBuildingCategory::Foundations;
        }
        
        // --- ARCHITECTURE ---
        if (Class->IsChildOf(AFGBuildableWall::StaticClass()) ||
            Class->IsChildOf(AFGBuildablePillar::StaticClass()) ||
            Class->IsChildOf(AFGBuildableStair::StaticClass()) ||
            Class->IsChildOf(AFGBuildableWalkway::StaticClass()) ||
            Class->IsChildOf(AFGBuildableDecor::StaticClass()) ||
            Class->IsChildOf(AFGBuildableSignBase::StaticClass()) ||
            Class->IsChildOf(AFGBuildableLightsControlPanel::StaticClass()))
        {
            return EBuildingCategory::Architecture;
        }
        
        // --- DEFENSE ---
        if (Class->IsChildOf(AFGBuildableSnowCannon::StaticClass()))
        {
            return EBuildingCategory::Defense;
        }
        
        // Radar tower is special - it could be "Miner" (resource discovery) or None
        if (Class->IsChildOf(AFGBuildableRadarTower::StaticClass()))
        {
            return EBuildingCategory::None;
        }
        
        // Default: not classified
        return EBuildingCategory::None;
    }
    
    // ---------------------------------------------------------------------------
    // Apply auto-swatch to a single buildable
    // ---------------------------------------------------------------------------
    
    bool ApplyAutoSwatch(AActor* Buildable)
    {
        if (!Buildable) return false;
        
        // Get the category for this buildable
        EBuildingCategory Category = DetermineCategory(Buildable);
        if (Category == EBuildingCategory::None) return false;
        
        // Get the configured swatch slot for this category
        int32 Slot = USwatchManager::GetSwatchSlotForCategory(Category);
        if (Slot < 0) return false; // no mapping configured
        
        // Check if this buildable already has a custom/non-default swatch
        AFGBuildable* FGBuildable = Cast<AFGBuildable>(Buildable);
        if (FGBuildable)
        {
            // Only auto-assign if the building still has the default swatch (slot 0)
            // This prevents overwriting manual color choices
            if (FGBuildable->GetColorSlot() != 0)
            {
                return false; // already painted manually
            }
            
            // Apply the swatch
            FGBuildable->SetColorSlot(Slot);
            
            // Force visual update
            FGBuildable->ApplyColorToMesh();
            
            return true;
        }
        
        return false;
    }
    
    // ---------------------------------------------------------------------------
    // Apply to all buildables in a world
    // ---------------------------------------------------------------------------
    
    void ApplyToAllBuildables(UWorld* World)
    {
        if (!World) return;
        
        AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(World);
        if (!Subsystem) return;
        
        TArray<AFGBuildable*> AllBuildables;
        Subsystem->GetAllBuildables(AllBuildables);
        
        int32 AppliedCount = 0;
        for (AFGBuildable* Buildable : AllBuildables)
        {
            if (ApplyAutoSwatch(Buildable))
            {
                AppliedCount++;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Auto-assigned %d/%d buildables"),
            AppliedCount, AllBuildables.Num());
    }
    
    // ---------------------------------------------------------------------------
    // Initialization - hook the buildable placement event
    // ---------------------------------------------------------------------------
    
    void Initialize()
    {
#if !WITH_EDITOR
        // Hook: On building placed -> auto-assign swatch
        // SUBSCRIBE_METHOD_AFTER(AFGBuildableSubsystem::AddBuildable, [](auto& Scope, AFGBuildableSubsystem* Self, AFGBuildable* Buildable)
        // {
        //     if (Buildable)
        //     {
        //         ApplyAutoSwatch(Buildable);
        //     }
        // });
        
        // Hook: On game load -> apply to all
        // SUBSCRIBE_METHOD_AFTER(AFGGameState::PostLoadGame, [](auto& Scope, AFGGameState* State, int32 SaveVersion, int32 GameVersion)
        // {
        //     ApplyToAllBuildables(State->GetWorld());
        // });
#endif
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] AutoSwatchSystem initialized."));
    }
}
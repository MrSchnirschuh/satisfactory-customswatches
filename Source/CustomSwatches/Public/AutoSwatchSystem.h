#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Buildables/FGBuildable.h"
#include "SwatchManager.h"

/**
 * Automatic swatch assignment system.
 * 
 * Detects building categories from the native game class hierarchy
 * and assigns the configured swatch slot automatically.
 *
 * Category detection rules (mirrors the game's internal classification):
 * 
 * - AFGBuildableFactory              -> Production (smelters, constructors, assemblers, manufacturers, refineries, etc.)
 * - AFGBuildableManufacturer         -> Production (more specific)
 * - AFGBuildableGenerator            -> PowerGen (coal, fuel, nuclear, geothermal)
 * - AFGBuildablePowerPole            -> Power (poles, towers, wall outlets)
 * - AFGBuildablePowerTower           -> Power
 * - AFGBuildableCircuitBridge        -> Power / Circuit
 * - AFGBuildablePipeline             -> Logistics (pipes / fluids)
 * - AFGBuildableConveyorBase         -> Logistics (belts / lifts)
 * - AFGBuildableStorage              -> Storage
 * - AFGBuildableSplitter             -> Logistics (splitters / mergers)
 * - AFGBuildableTrainPlatform        -> Transport (stations / freight platforms)
 * - AFGBuildableRailroadTrack        -> Transport  
 * - AFGBuildableVehicle              -> Transport
 * - AFGBuildableFoundation           -> Foundations
 * - AFGBuildableWall                 -> Foundations / Architecture
 * - AFGBuildablePillar               -> Architecture
 * - AFGBuildableStair                -> Architecture
 * - AFGBuildableWalkway              -> Architecture
 * - AFGBuildableDecor                -> Architecture
 * - AFGBuildableResourceExtractor    -> Miner (miners, oil extractors, water extractors)
 * - AFGBuildableResourceSink         -> Logistics / Special
 * - AFGBuildableFrackingActivator    -> Miner
 * - AFGBuildableRadarTower           -> None (special)
 * - AFGBuildableSignBase             -> Decor / Architecture
 * - AFGBuildableLightSource          -> Power / Decor
 * - AFGBuildableDroneStation         -> Transport
 * - AFGBuildablePipeHyper            -> Transport (hypertubes)
 * - AFGBuildableJumpPad              -> Transport
 * - AFGBuildablePowerStorage         -> Power
 * - AFGBuildablePriorityPowerPole     -> Power
 * - AFGBuildableSnowCannon           -> Defense (silly, but added)
 */
namespace AutoSwatchSystem
{
    /** 
     * Initialize the auto-swatch system.
     * Hooks building placement and load events.
     */
    void Initialize();

    /** 
     * Determine the category for a given buildable based on its class hierarchy.
     * This is the main classification function.
     */
    EBuildingCategory DetermineCategory(AActor* Buildable);

    /** 
     * Check if an actor's class inherits from a specific type.
     * Uses UE5's IsChildOf / Cast semantics.
     */
    template<typename T>
    bool IsOfCategoryType(AActor* Buildable)
    {
        if (!Buildable) return false;
        return Buildable->IsA<T>();
    }

    /** 
     * Apply the configured swatch to a single buildable based on auto-detected category.
     * Returns true if a swatch was applied.
     */
    bool ApplyAutoSwatch(AActor* Buildable);

    /** 
     * Apply auto-swatches to all buildables in a given world.
     * Called on world load / after all buildings are registered.
     */
    void ApplyToAllBuildables(UWorld* World);
}
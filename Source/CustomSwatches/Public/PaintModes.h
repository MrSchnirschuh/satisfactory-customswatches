#pragma once

#include "CoreMinimal.h"
#include "SwatchManager.h"

/**
 * Blueprint paint mode system.
 *
 * Extends the vanilla build-gun paint tool with three "blueprint-aware" modes:
 *
 * MODE: Blueprint All
 *   Paints EVERY buildable in the same blueprint group as the target.
 *   No filtering - everything inside that blueprint ID gets the new swatch.
 *   Useful for: quickly recoloring an entire factory module.
 *
 * MODE: Blueprint Mode (unpainted-target)
 *   Paints ONLY buildables in the blueprint that have the "unpainted marker" swatch.
 *   Buildings that already have a non-marker swatch (e.g. Power infrastructure)
 *   are SKIPPED.
 *   Useful for: color-coding production lines within a shared blueprint
 *   while keeping power/structural swatches intact.
 * 
 *   Example: A smelter blueprint has:
 *     - 3 smelters (each has UnpaintedMarker swatch)
 *     - 1 power pole (has Power swatch)
 *     - 1 conveyor belt (has Logistics swatch)
 *   With Blueprint Mode, painting with "Caterium Yellow" only paints the
 *   3 smelters. Power pole and belt keep their category swatches.
 *
 * MODE: Category Select
 *   Paints only buildings of a specific category within the blueprint.
 *   e.g. "Only recolor the power generators in this blueprint"
 */

namespace PaintModes
{
    /** Initialize paint mode hooks */
    void Initialize();

    /** 
     * Core paint function - applies a swatch with mode awareness.
     * 
     * This is the function hooked into the build gun's paint action.
     */
    int32 PaintWithMode(AActor* TargetActor, int32 TargetSlot, EBlueprintPaintMode Mode);

    /** 
     * Find all buildables sharing a blueprint ID with the target actor.
     * Blueprint ID comes from the actor's root component / hierarchical group.
     */
    TArray<AActor*> FindBlueprintGroup(AActor* TargetActor);

    /** 
     * Filter a blueprint group by the "unpainted marker" swatch.
     * Only returns actors whose current swatch matches the unpainted marker slot.
     */
    TArray<AActor*> FilterByUnpaintedMarker(const TArray<AActor*>& Group);

    /** 
     * Filter a blueprint group by building category.
     */
    TArray<AActor*> FilterByCategory(const TArray<AActor*>& Group, EBuildingCategory Category);

    /** 
     * Apply a swatch slot to a single actor (internal helper).
     * Skips actors that don't implement the colorable interface.
     */
    bool ApplySwatchToActor(AActor* Actor, int32 SlotIndex);
}
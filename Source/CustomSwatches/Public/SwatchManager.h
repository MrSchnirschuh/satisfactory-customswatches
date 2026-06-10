#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/NoExportTypes.h"
#include "FGColoredInstanceMesh.h"
#include "Buildables/FGBuildable.h"
#include "SwatchManager.generated.h"

USTRUCT(BlueprintType)
struct FSwatchDefinition
{
    GENERATED_BODY()

    /** Display name shown in the paint UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    FText DisplayName;

    /** Primary color (albedo) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    FLinearColor PrimaryColor;

    /** Secondary color (emissive/trim) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    FLinearColor SecondaryColor;

    /** Emission / glow intensity (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    float EmissiveIntensity;

    /** Metallic roughness value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    float Roughness;

    /** Whether this swatch is user-defined (vs built-in) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    bool bIsUserDefined;

    /** Optional category tag for auto-assignment logic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
    FString AutoAssignCategory;

    FSwatchDefinition()
        : PrimaryColor(1.0f, 1.0f, 1.0f, 1.0f)
        , SecondaryColor(0.0f, 0.0f, 0.0f, 1.0f)
        , EmissiveIntensity(0.0f)
        , Roughness(0.5f)
        , bIsUserDefined(false)
    {
    }
};

/** 
 * Categories that buildings can be auto-assigned to.
 * Each category maps to a reserved swatch slot.
 */
UENUM(BlueprintType)
enum class EBuildingCategory : uint8
{
    None        UMETA(DisplayName = "None / Default"),
    Power       UMETA(DisplayName = "Power Infrastructure"),
    Production  UMETA(DisplayName = "Production / Smelters / Constructors"),
    Logistics   UMETA(DisplayName = "Logistics / Belts / Pipes / Conveyors"),
    Storage     UMETA(DisplayName = "Storage / Containers"),
    Transport   UMETA(DisplayName = "Transport / Vehicles / Trains"),
    Foundations UMETA(DisplayName = "Foundations / Walls / Architecture"),
    Architecture UMETA(DisplayName = "Pillars / Beams / Decor"),
    PowerGen    UMETA(DisplayName = "Power Generation"),
    Miner       UMETA(DisplayName = "Miners / Extractors"),
    Defense     UMETA(DisplayName = "Defense / Walls / Barriers"),
    Custom_00   UMETA(DisplayName = "Custom Category 1"),
    Custom_01   UMETA(DisplayName = "Custom Category 2"),
    Custom_02   UMETA(DisplayName = "Custom Category 3"),
    Custom_03   UMETA(DisplayName = "Custom Category 4"),
    MAX
};

/** Paint mode for blueprint interactions */
UENUM(BlueprintType)
enum class EBlueprintPaintMode : uint8
{
    Default         UMETA(DisplayName = "Default (single building)"),
    BlueprintAll    UMETA(DisplayName = "Blueprint All (entire blueprint)"),
    BlueprintMode   UMETA(DisplayName = "Blueprint Mode (unpainted-only)"),
    CategorySelect  UMETA(DisplayName = "Category Select (filtered)"),
};

/**
 * Manages custom swatch slots, auto-assignment rules, and paint modes.
 * This is the central coordinator for the mod.
 */
UCLASS(BlueprintType)
class USwatchManager : public UObject
{
    GENERATED_BODY()

public:
    /** Initialize the swatch manager - register custom slots */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static void Initialize();

    /** Register a custom swatch slot beyond the game's default slots */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static int32 RegisterCustomSwatch(const FSwatchDefinition& Definition);

    /** Get the total number of available swatch slots (default + custom) */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static int32 GetTotalSwatchCount();

    /** Get a swatch definition by slot index */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static FSwatchDefinition GetSwatchForSlot(int32 SlotIndex);

    /** Update a custom swatch definition at runtime */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static void UpdateSwatch(int32 SlotIndex, const FSwatchDefinition& NewDefinition);

    /** Remove a user-defined swatch */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static bool RemoveCustomSwatch(int32 SlotIndex);

    /** Get the assigned category for a buildable */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static EBuildingCategory GetCategoryForBuildable(AActor* Buildable);

    /** Auto-assign a swatch to a buildable based on its category */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static bool AutoAssignSwatch(AActor* Buildable);

    /** 
     * Get the mapping from category -> preferred swatch slot index.
     * Categories can be remapped by the user via config.
     */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static int32 GetSwatchSlotForCategory(EBuildingCategory Category);

    /** Set which swatch slot a category maps to */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static void SetCategoryMapping(EBuildingCategory Category, int32 SlotIndex);

    /** Get all known category mappings */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static TMap<EBuildingCategory, int32> GetCategoryMappings();

    /** Reload user-defined swatches from config file */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static void ReloadUserSwatches();

    /** Save current user-defined swatches to config file */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager")
    static void SaveUserSwatches();

    // --- Blueprint Paint Mode helpers ---

    /** Set the active blueprint paint mode */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager|PaintMode")
    static void SetPaintMode(EBlueprintPaintMode NewMode);

    /** Get current paint mode */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager|PaintMode")
    static EBlueprintPaintMode GetPaintMode();

    /** 
     * Apply a swatch to all buildables within a blueprint / selection,
     * respecting the current paint mode.
     * 
     * @param Actor         The actor that was clicked / painted (may be part of a blueprint)
     * @param TargetSlot    The swatch slot to apply
     * @param Mode          The paint mode to use
     * @return Number of buildings repainted
     */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager|PaintMode")
    static int32 ApplySwatchWithMode(AActor* Actor, int32 TargetSlot, EBlueprintPaintMode Mode);

    /** 
     * The "unpainted marker" swatch slot - buildings with this slot are
     * considered "tagged for unpainted repaint" in BlueprintMode.
     */
    UFUNCTION(BlueprintCallable, Category = "SwatchManager|PaintMode")
    static int32 GetUnpaintedMarkerSlot();

    UFUNCTION(BlueprintCallable, Category = "SwatchManager|PaintMode")
    static void SetUnpaintedMarkerSlot(int32 SlotIndex);

private:
    /** Singleton instance */
    static USwatchManager* Instance();

    /** Internal: hook into the game's color/paint system */
    static void HookPaintSystem();

    /** Internal: register default category mappings */
    static void RegisterDefaultCategoryMappings();

    /** Internal: apply auto-assignment to all existing buildables */
    static void ApplyAutoAssignmentToAll();

    /** Cached custom swatch definitions (indices beyond default slots) */
    TArray<FSwatchDefinition> CustomSwatches;

    /** Base game swatch count (typically 8 standard + 4 vehicle/pipes/extras) */
    static constexpr int32 BaseGameSwatchCount = 12;

    /** Maximum total swatches we support */
    static constexpr int32 MaxTotalSwatches = 64;

    /** Category -> SwatchSlot mapping */
    TMap<EBuildingCategory, int32> CategorySlotMap;

    /** Active paint mode */
    EBlueprintPaintMode ActivePaintMode;

    /** 
     * Special "unpainted marker" slot index.
     * Buildings painted with this slot are targeted by BlueprintMode painting.
     */
    int32 UnpaintedMarkerSlotIndex;
};
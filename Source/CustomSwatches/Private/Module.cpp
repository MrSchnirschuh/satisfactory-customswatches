#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SwatchManager.h"

#if !WITH_EDITOR
#include "SML/mod/SMLMod.h"
#include "SML/util/mod/ModConfiguration.h"
#endif

/**
 * CustomSwatches Mod
 * 
 * Satisfactory 1.2+ mod that extends the game's color swatch / paint system.
 * 
 * Features:
 * - Custom user-defined swatches (added via JSON config)
 * - Auto-swatch assignment by building category
 * - Blueprint-aware paint modes
 * - Configurable category -> swatch mappings
 * 
 * Build Requirements:
 * - Satisfactory Mod Loader (SML) 1.0+
 * - UE5 C++ toolchain
 * - Satisfactory SDK headers
 */

#if WITH_EDITOR
class FCustomSwatchesMod : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // In-editor: just log
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Loaded in editor mode."));
    }

    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FCustomSwatchesMod, CustomSwatches);
#else

/**
 * SML Mod class - Satisfactory 1.0+ mod entry point.
 * SML uses FICSIT_MOD or a derived SMLMod class for registration.
 */
class FCustomSwatchesMod : public FICSITMod
{
public:
    virtual void StartMod_Implementation() override
    {
        USwatchManager::Initialize();
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Mod started with %d total swatch slots."),
            USwatchManager::GetTotalSwatchCount());
    }

    virtual void RegisterConfiguration_Implementation() override
    {
        // Register configuration options
        // These show up in the SML config UI
        
        // Example: slot for "unpainted marker"
        /*
        RegisterIntEntry(
            TEXT("unpainted_marker_slot"),
            TEXT("Unpainted Marker Slot"),
            TEXT("Buildings with this swatch slot are targeted by 'Blueprint Mode' painting. -1 to disable."),
            -1,
            -1,
            63
        );
        */
        
        // Category-to-swatch mappings would be registered here
        // Users can configure which swatch each category maps to
    }

    virtual FVersion GetVersion_Implementation() const override
    {
        return {1, 0, 0};
    }
};

IMPLEMENT_FICSIT_MOD(FCustomSwatchesMod, CustomSwatches, TEXT("CustomSwatches"), TEXT("1.0.0"));
#endif
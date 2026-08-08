#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SwatchManager.h"

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

class FCustomSwatchesModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        USwatchManager::Initialize();
        UE_LOG(LogTemp, Log, TEXT("[CustomSwatches] Loaded. %d total swatch slots."),
            USwatchManager::GetTotalSwatchCount());
    }

    virtual void ShutdownModule() override {}
};

IMPLEMENT_GAME_MODULE(FCustomSwatchesModule, CustomSwatches);

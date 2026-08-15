# CustomSwatches - Satisfactory Mod

**Version:** 1.0.0  
**Satisfactory:** 1.2+ (build ≥ CL502094 / `>=491125` in the engine sense used by SML)  
**SML:** ≥ 3.10 (Satisfactory 1.2 customization API)

> Adds custom paint swatches, automatic category-based coloring, and extra blueprint paint modes to Satisfactory.

## What it does

### 1. Custom Swatches
Add your own color swatches via a simple JSON file. No C++ changes needed.
Define primary color, secondary color, emissive/glow, and roughness.

### 2. Automatic Swatch Assignment
New buildings still on the default swatch (slot 0) automatically get the right color based on their **category**:

| Category | Includes |
|----------|----------|
| Power | Power poles, towers, circuit bridges, power storage, priority switches |
| PowerGen | Coal/Fuel/Nuclear/Geothermal generators |
| Miner | Resource miners, oil extractors, water extractors, fracking activators |
| Production | Smelters, constructors, assemblers, manufacturers, refineries, particle accelerators, blenders, packagers |
| Logistics | Conveyor belts, lifts, pipes, splitters, mergers, resource sinks |
| Storage | Containers, fluid buffers |
| Transport | Train stations, tracks, locomotives, freight platforms, vehicles, drones, hypertubes, jump pads |
| Foundations | Foundations, ramps |
| Architecture | Walls, pillars, stairs, walkways, signs, lights, decor |
| Defense | Snow cannons (and modded defense buildings) |

Manually painted buildings keep their color.

### 3. Blueprint Paint Modes
Additional modes for the build gun's paint tool:

- **Blueprint All** — paints every building in the same blueprint group.
  Use case: instantly recolor an entire factory module.

- **Blueprint Mode (unpainted-target)** — only paints buildings with the
  "Unpainted Marker" swatch. Buildings with category-specific swatches
  (power poles, belts, foundations) are skipped.
  Use case: color-code production lines in a shared blueprint without
  repainting infrastructure.

  Example — smelter blueprint with 3 smelters on "Unpainted Marker", 1 power pole on Power swatch and 1 belt on Logistics swatch: painting with "Caterium Yellow" turns only the smelters yellow; infrastructure keeps its category colors.

- **Category Select** — only paints buildings of one category in the blueprint.
  Use case: recolor all power generators without touching production.

### 4. Configurable
- Add/remove custom swatches by editing `user_swatches.json`
- Map categories to specific swatch slots via `DefaultModConfig.ini` (or the SML config UI)
- Set the "unpainted marker" slot

## Installation

### Via Satisfactory Mod Manager (SMM) — recommended

1. Open the [Satisfactory Mod Manager](https://docs.ficsit.app/satisfactory-modding/latest/ForUsers/SatisfactoryModManager.html).
2. Search for **"CustomSwatches"** and click **Install**.
3. Launch the game — the mod loads automatically.

### Manual install

1. Download the latest `CustomSwatches.zip` from the [Releases](../../releases) page.
2. Extract it to your Satisfactory mods folder:
   ```
   %LOCALAPPDATA%\FactoryGame\Mods\
   ```
   The folder should contain `CustomSwatches.uplugin`, `DefaultModConfig.ini`, and the `Source/` tree.
3. Start Satisfactory.

### From source (for SML developers)

1. Set up the SML development environment (UE5 + Satisfactory SDK).
2. Clone this repo and place it under your SML tree:
   ```powershell
   git clone https://github.com/MrSchnirschuh/satisfactory-customswatches.git
   Copy-Item -Recurse satisfactory-customswatches\CustomSwatches SML\Mods\
   ```
3. Generate project files and build:
   ```powershell
   cd SML
   .\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe -projectfiles -project="SML\FactoryGame.uproject" -game -engine
   .\Engine\Build\BatchFiles\Build.bat FactoryEditor Win64 Development -project="SML\FactoryGame.uproject" -NoHotReload
   .\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject="SML\FactoryGame.uproject" PackagePlugin -Project="SML\FactoryGame.uproject" -dlcname=CustomSwatches -build -server -clientconfig=Shipping -serverconfig=Shipping -platform=Win64 -serverplatform=Win64 -nocompileeditor -installed
   ```
4. Packaged output lands in `SML\Saved\ArchivedPlugins\CustomSwatches\`.

### Configure swatches

Copy `data/default_swatches.json` to:
```
%LOCALAPPDATA%\FactoryGame\Saved\CustomSwatches\user_swatches.json
```

Edit the JSON to add your own colors:
```json
{
  "swatches": [
    {
      "name": "My Color Name",
      "r": 1.0, "g": 0.5, "b": 0.0,     // primary RGB (0-1)
      "sr": 0.6, "sg": 0.3, "sb": 0.0,   // secondary RGB
      "emissive": 0.2,                     // glow intensity (0-1)
      "roughness": 0.3,                    // 0=shiny, 1=matte
      "category": "production"             // optional auto-assign tag
    }
  ]
}
```

Restart the game to reload swatches. Game default slots stay in place; custom slots follow the order in your file.

### Included default swatches

| # | Name | Category | Notes |
|---|------|----------|-------|
| 0 | Caterium Yellow | production | |
| 1 | Copper Orange | production | |
| 2 | Iron Blue | production | |
| 3 | Steel Grey | production | |
| 4 | Quartz White | production | |
| 5 | Sulfur Green | production | |
| 6 | Uranium Green Glow | powergen | high emissive |
| 7 | Nitrogen Purple | production | |
| 8 | Bauxite Silver | production | |
| 9 | Coal Black | powergen | |
| 10 | Oil Red | production | |
| 11 | Nuclear Purple Glow | powergen | high emissive |
| 12 | **Unpainted Marker** | unpainted_marker | used by Blueprint Mode |

## Usage In-Game

1. Equip the Build Gun and select Paint mode
2. Use the standard color slot selector - custom slots appear after the defaults
3. Press a keybind (configurable) to cycle paint modes:
   - **Default** — single building, vanilla behavior
   - **Blueprint All** — every building in the same blueprint group
   - **Blueprint Mode (unpainted-target)** — only buildings with the Unpainted Marker swatch
   - **Category Select** — only buildings of the selected category
4. Paint a building — behavior changes based on the active mode.

### Default paint mode quick reference

The mode can be changed in `DefaultModConfig.ini` (`DefaultPaintMode=0..3`) or via the in-game SML config UI:

| Value | Mode |
|-------|------|
| 0 | Default (single building) |
| 1 | Blueprint All |
| 2 | Blueprint Mode (unpainted-target) |
| 3 | Category Select |

## Configuration

Place `DefaultModConfig.ini` in:
```
%LOCALAPPDATA%\FactoryGame\Saved\Config\WindowsNoEditor\
```

Key settings:
- `UnpaintedMarkerSlot` — swatch slot index considered "unpainted" by Blueprint Mode (`12` by default, `-1` to disable)
- `CategoryMapping_*` — map each building category to a swatch slot (`-1` disables auto-assignment)
- `DefaultPaintMode` — paint mode on game start

## Development Notes

### Source Structure
```
Source/CustomSwatches/
  Public/
    SwatchManager.h     - Main API, config, category mapping
    AutoSwatchSystem.h  - Category detection declarations
    PaintModes.h        - Blueprint paint modes declarations
  Private/
    Module.cpp          - Entry point (SML mod registration)
    SwatchManager.cpp   - Swatch management implementation
    AutoSwatchSystem.cpp- Category detection + auto-assignment
    PaintModes.cpp      - Blueprint paint mode implementation
  CustomSwatches.Build.cs - UE5 build rules
```

### API Notes
- The mod uses SML's `SUBSCRIBE_METHOD` hooks for non-invasive patching
- Category detection uses the UE class hierarchy (`IsChildOf`)
- Blueprint grouping uses `AFGBlueprintSubsystem` (Satisfactory 1.0+)
- Color application uses `AFGBuildable::SetColorSlot()` + `ApplyColorToMesh()`

### Version & release

- Version is defined in `CustomSwatches.uplugin` (`VersionName`/`SemVersion`) — single source of truth.
- The release workflow (`.github/workflows/release.yml`) builds the packaged mod archive and attaches it to a GitHub Release on every pushed `v*` tag.
- The current `build.yml` builds every push and uploads a CI artifact; use a `v*` tag to trigger a full release archive.

### TODO / Planned Features
- [ ] In-game UI for managing custom swatches (instead of JSON editing)
- [ ] Paint mode indicator on HUD
- [ ] Keybind configuration for mode cycling
- [ ] Per-world category mappings (saved in save file)
- [ ] Copy-paste swatch between buildings
- [ ] Random swatch mode for variety in foundations
- [ ] Conditional auto-assignment (e.g. "only if power > X MW")

## License
MIT
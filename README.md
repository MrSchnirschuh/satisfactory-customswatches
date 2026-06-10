# CustomSwatches - Satisfactory Mod

## What it does

Extends the vanilla paint/swatch system with:

### 1. Custom Swatches
Add your own color swatches via a simple JSON file. No C++ changes needed.
Define primary color, secondary color, emissive/glow, and roughness.

### 2. Automatic Swatch Assignment
Buildings automatically get the right swatch based on their **category**:

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

Auto-assignment only applies to buildings still on the default swatch (slot 0).
Manually painted buildings keep their color.

### 3. Blueprint Paint Modes
Additional modes for the build gun's paint tool:

- **Blueprint All**: Paints every building in the same blueprint group. 
  Use case: instantly recolor an entire factory module.

- **Blueprint Mode (unpainted-target)**: Only paints buildings with the
  "Unpainted Marker" swatch. Buildings with category-specific swatches
  (power poles, belts, foundations) are SKIPPED.
  Use case: color-code production lines in a shared blueprint without
  repainting infrastructure.

  Example - Smelter Blueprint:
  - 3 smelters each have the "Unpainted Marker" swatch
  - 1 power pole has Power swatch
  - 1 conveyor belt has Logistics swatch
  With Blueprint Mode + "Caterium Yellow": only smelters turn yellow.
  Power pole and belt keep their category colors.

- **Category Select**: Only paints buildings of one category in the blueprint.
  Use case: recolor all power generators without touching production.

### 4. Configurable
- Add/remove custom swatches by editing `user_swatches.json`
- Map categories to specific swatch slots via config
- Set the "unpainted marker" slot

## Installation

### Prerequisites
- Satisfactory 1.2+
- Satisfactory Mod Loader (SML) installed

### Build from source

1. Set up the SML development environment:
   - Install UE5 and the Satisfactory SDK
   - Clone the SML repository

2. Clone this mod:
   ```
   git clone <this-repo>
   ```

3. Copy to the SML mods source folder:
   ```
   cp -r CustomSwatches <SML>/Source/Mods/
   ```

4. Build with SML's build script:
   ```
   python build.py
   ```

5. The compiled `.dll` goes into:
   ```
   <Satisfactory>/FactoryGame/Mods/CustomSwatches/
   ```

### Configure swatches

Copy `data/user_swatches.json` to:
```
<Satisfactory>/FactoryGame/Saved/CustomSwatches/user_swatches.json
```

Edit the JSON to add your colors. The format is:
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

### Default swatches included
The mod comes with 13 pre-defined swatches:
- Caterium Yellow, Copper Orange, Iron Blue, Steel Grey
- Quartz White, Sulfur Green, Uranium Green Glow
- Nitrogen Purple, Bauxite Silver, Coal Black
- Oil Red, Nuclear Purple Glow
- **Unpainted Marker** (slot 12 - used by Blueprint Mode)

## Usage In-Game

1. Equip the Build Gun and select Paint mode
2. Use the standard color slot selector - custom slots appear after the defaults
3. Press a keybind (configurable) to cycle paint modes:
   - Default mode
   - Blueprint All mode
   - Blueprint Mode (unpainted-target)
   - Category Select mode
4. Paint a building - the behavior changes based on active mode

### Recommended workflow

1. Set up category auto-assignment:
   - Map `Power` category -> slot 0 or a custom slot
   - Map `Logistics` -> slot 9 (pipes slot)
   
2. For blueprints with multiple production lines:
   - Before saving the blueprint, paint machines with "Unpainted Marker"
   - Connect power poles (they auto-get Power swatch)
   - Connect belts (they auto-get Logistics swatch)
   - Save blueprint
   - Place blueprint, then paint with Blueprint Mode + color per production line

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
- Category detection uses the UE class hierarchy (IsChildOf)
- Blueprint grouping uses `AFGBlueprintSubsystem` (Satisfactory 1.0+)
- Color application uses `AFGBuildable::SetColorSlot()` + `ApplyColorToMesh()`

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
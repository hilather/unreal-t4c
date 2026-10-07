# Lighthaven Unreal design and agent implementation package

Prepared 2026-10-07 for the requested T4C-inspired game. **This package contains plans and visual starter material; it is not an Unreal project or playable build.**

The first complete slice is a Windows single-player game: create a human character, enter Lighthaven's church, explore its four basement floors, fight the researched enemy roster, develop the character, defeat Balork, return and save. Modern 3D graphics, running, elevated camera and controller support are planned. Multiplayer servers come later.

## Start reading

1. [Game design](docs/01-game-design.md) — character creation, progression, controls, combat and scope.
2. [Unreal architecture](docs/02-unreal-architecture.md) — C++/GAS, gameplay ownership, data, local saves and future network boundaries.
3. [World and encounters](docs/03-world-and-encounters.md) — church, four floors, full roster, respawn and city expansion.
4. [Art pipeline](docs/04-art-pipeline.md) — original references → enhanced concepts → rigged 3D assets.
5. [Agent implementation waves](docs/05-agent-waves.md) — seven gated waves, task IDs, dependencies, exclusive ownership and acceptance.
6. [Validation and delivery](docs/06-validation-and-delivery.md) — playable-build checks, persistence, progression and performance.
7. [Coordinator launch brief](agents/START-HERE.md) — ready-to-use execution and review instructions.

## Research and starter assets

- [Lighthaven evidence](research/lighthaven-evidence.md): source maps, NPCs, eleven creature definitions and uncertainties.
- [Character-rules evidence](research/character-rules-evidence.md): supported rules, creation flow, low-level spells and outstanding formulas.
- [Asset manifest](assets/asset-manifest.json): provenance, dimensions, hashes and intended usage.
- [Character/monster reference index](assets/references/character-monster-manifest.md).
- `assets/references/maps/`: town and all four temple-dungeon maps.
- `assets/references/characters/`: original human direction references plus separately identified operator promotional art.
- `assets/references/monsters/`: seven visual families covering the eleven planned enemy definitions.
- `assets/concepts/`: improved environment and character/bestiary concepts; art direction rather than game-ready models.
- [Data contracts](contracts/README.md): illustrative JSON inputs and native-schema responsibilities.

The bundle includes **32 downloaded reference images and 3 enhanced concept images**. The original player directions and monster pictures are preserved unchanged.

![Environment art direction](assets/concepts/temple-and-basement-concept.png)

![Starter characters and creatures](assets/concepts/starter-characters-and-creatures-concept.png)

![Deeper dungeon creature concepts](assets/concepts/deep-dungeon-creatures-concept.png)

## Decisions already made

| Topic | Decision |
|---|---|
| Engine | Unreal 5; pin the compatible installed stable version at kickoff |
| Language | C++ rules/gameplay, Blueprints/data assets for content/presentation |
| Networking now | None; local authoritative gameplay and local saves |
| Camera/combat | Elevated 3D, smooth running, stat-driven targeting |
| Stage 1A | Creation, church/forecourt, B1 loop, save/load |
| Stage 1B | All four floors, eleven enemy definitions, Balork, viable build/service routes |
| City | Essential services grayboxed in Stage 1B; full city completion follows |
| Platforms | Windows first; preserve controller/platform boundaries for later Steam/Xbox |
| Art | Downloaded originals remain references; improved concept examples supplied; modeling/rigging still required |
| Fidelity | Source-linked baseline with clearly recorded prototype deviations |

## Findings that prevent implementation mistakes

The church dungeon has four floors and is separate from the cemetery crypt and Lighthaven Cave. Do not fill it with skeletons or zombies based on the word “catacombs.” Preserve separate bat and goblin gameplay variants even when they share visual families. Dungeon Bat floor placement remains provisional.

Character development is stat/skill driven rather than permanently class-locked. Source-backed progression uses five primary attributes and grants five attribute points plus fifteen skill points per level. Exact creation roll distribution and several combat/growth numbers still need baseline verification; modern server settings differ. See the evidence ledgers instead of treating every internet table as a universal T4C rule.

Mage/ranged progression needs actual trainers and supplies. The plan includes graybox access to the relevant town services without requiring a complete city art pass. Ordinary enemy respawn prevents an exhausted dungeon from blocking XP, training money or the rat errand.

## Using this package

Unzip, read the coordinator brief, then assign Wave 0 against the actual project repository. Keep source references available to the map/art agents. The downloaded images and generated concepts are not full directional animation packs, skeletal meshes or validated Unreal assets. Complete those production tasks through the art waves.

The documents explicitly distinguish plans, verified source facts, reconstruction choices and checks that must later be run in Unreal. No compile, editor, packaging, console or gameplay verification is claimed here.

# W5-24 props import and UI icons — evidence candidate

Task ID: W5-24. Base revision: `8fa51dfd306fe8741bea3775c9101ee70288dc81`.
Contract revision: 1. Result revision: submission candidate (receipt records exact OID).
Owned paths: `build/build-props-art.sh`, `Source/Lighthaven/Visual/Props/`,
`Source/Lighthaven/UI/`, `Source/LighthavenTests/Visual/Props/`,
`Source/LighthavenTests/UI/`, and this document. No binary assets in deliverable.

## Behavior and integration

The separate props stage rebuilds the W5-23 kit and transparent icons serially
with pinned Blender, runs both standard-library and Blender re-import validation,
then imports via the engine PythonScript commandlet. Invocation:

```sh
UE_ROOT=/home/brewerm/Downloads/unreal \
BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64 \
bash build/build-props-art.sh
```

Coordinator integration line to add to `build/build-art.sh`:

```sh
bash "$LH_PROJECT_ROOT/build/build-props-art.sh"
```

`LH_PROPS_SKIP_EXPORT=1` reuses already generated, validated exports for importer
iteration. An unchanged receipt bypasses export entirely, so a hydrated fresh
clone needs no source-output directory. Receipt signatures contain authored
tracked source names/bytes and engine Build.version contents only; no exported
bytes, filesystem dates or absolute paths. Output package hashes are separate
integrity evidence. A failed import invalidates its previous receipt. Receipts
and packages belong to coordinator generation/LFS, not this source deliverable.

All GLB components are combined into one static mesh per file, retaining their
common pivot and material slots. Geometry is reflected once using the established
player-import basis; Unreal converts metres to centimetres. Bounds must match
source minima/maxima within 0.5 cm before saving a success receipt. Atlas base,
normal (+Y flipped for Unreal) and ORM are shared. The opaque two-sided atlas
material binds linear normal/ORM and sRGB base; texture-free two-sided effects
retain glTF emission and alpha blend. No lighting is baked into these materials.
Icons use UI texture group/compression, sRGB, retained alpha, no mipmaps and no
streaming. The stage reports actual asset bytes by Unreal class and refuses
receipts above 6,000,000 imported bytes.

`DA_Props` contains hard mesh/icon references; materials/textures are mesh
references. The native catalogue CDO hard-references that package for cook
inclusion. Items use exact catalog IDs; other meshes use documented file stems
(`LootSack`, `CorpseLootBadge`, `ChestClosed`, `ChestOpen` and effects).
Ability lookup returns the corresponding effect mesh collection. Unknown/missing
assets return null/empty and preserve existing gameplay/text behavior.

`SLHPropIcon` owns its brush, uses the style's 36px size and Raised (`#333333`)
backing, and collapses when its texture is absent. Inventory entries, selected
item, equipment, shop/sell/loot controls, spell rows and six HUD slots keep full
text labels and existing controls/navigation. Buy icons resolve offer subjects;
loot icons resolve persistent world containers, not just player inventory;
ability icons follow session reassignment. No sockets, world props or effect
life cycles are bound here.

Source-backed mechanics: none changed. Presentation tuning: existing W5-23
source geometry/materials and V-03 style tokens, including 36px icons and Raised
backing. No authentic mechanics values introduced.

## Checks

Host uid: 1000. Pinned Unreal 5.8.3 and Blender 5.2.2 were present; no
installation/download/system change was performed. Existing content was hydrated
from 442 verified local LFS cache objects (69,213,024 bytes) for checks only.

- Initial editor build failed for corrected include-order and snapshot-member
  issues (430.74 seconds). Corrected non-adaptive unity editor build succeeded
  (234.01 seconds); final editor rebuild with expanded tests succeeded
  (77.85 seconds). Game target succeeded (239.88 seconds). Exact farm flags:
  `-WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`,
  project-local XDG_CONFIG_HOME/UBA_ROOT and both documented XML config copies.
  Generated unity source includes the UI/props files; no adaptive-only validation.
- Blender export, icon render, independent byte validator and Blender re-import
  validator ran. Observed export: 29 meshes, 21 icons, three shared textures,
  2,571,651 bytes. No reference/concept pixels or downloaded assets used.
- Import exposed and corrected a read-only navigation property write, unsupported
  BodyInstance constructor keyword, and rooted expression deletion on reimport.
  Collision settings now use the reflected BodyInstance properties; graph nodes
  are reused by stable descriptions. Changed-input reimport then exited 0.
  The import invocation also disables the engine crash-report client after a
  failed assertion's background upload was blocked by the sandbox.
- Final imported receipt: **63 packages, 1,668,533 bytes** (decimal MB budget).
  **29 bounds checks**, maximum measured error **0.000004291535 cm**.
- Two default stage runs exited **0**, each printed `PROPS_IMPORT_UNCHANGED`.
  The second ran with the entire generated export directory temporarily absent.
  This tests source-output independence; it is not an actual separate fresh clone.
- Source-only receipt test passed: exported byte/time changes, tracked generated
  bytecode, untracked scripts
  and checkout relocation preserve signatures; tracked authored changes and
  engine-version changes invalidate them. Python syntax, shell syntax and
  `git diff --check` passed.

| Imported Unreal class | Bytes |
|---|---:|
| StaticMesh (29) | 1,033,604 |
| Texture2D (24: 21 icons + 3 atlas) | 567,707 |
| Material (9) | 56,319 |
| LHPropsAssets (1) | 10,903 |
| **Total** | **1,668,533** |

Full automation retry exported **244 tests: 243 passed with warnings, 1 failed,
0 unrun/in-process**, exit **255**, measured test duration **732.763 seconds**.
Failure: `Lighthaven.Visual.Monsters.CreatureImportedAssets`,
`LHCreatureArtTests.cpp:90`, missing eleven generated
`artsource/creatures/output/*-validation.json` files (rat, bat, slime, goblin,
giant_spider, balork, goblin_warrior, atrocity, dungeon_bat, giant_bat, undead_bat).
Those source-validation fixtures were not delivered in this isolated checkout;
regenerating or fabricating them is outside this task's owned paths. All other
native tests, including the new props/UI tests, passed. Passing tests carry the
common null-RHI/SDL Wayland-initialization warning; this is not a warning-free run.

Focused run: **25 UI/props tests passed with warnings, 0 failed**, exit **0**,
measured test duration **1.146 seconds**. The three new test names are
`Lighthaven.Visual.Props.CatalogAssets`, `Lighthaven.UI.Props.BrushFallback`, and
`Lighthaven.UI.Props.RowAndSlotIds`. Existing UI tests passed in both runs.
The first full-suite attempt was interrupted by sandbox-blocked editor telemetry
after 47 successful tests. The retry used both farm EditorSettings privacy
flags; no editor telemetry session was logged.

Linux hub cook completed with **exit 0**, **999 packages**, **0 errors and 20 warnings**,
commandlet duration **833.38 seconds**. All **63/63 props packages** from the
import receipt were present in cooked content; props packages plus cooked
sidecars occupied **3,802,837 bytes**. Earlier bounded shader-warmup attempts
ended with exits 124 and 137; their trimmed logs are retained. This was a cook,
not a packaged launch. This cook exercised the same native/import implementation
before the final signature-only correction described below.

## Remaining limitations

The existing reward interface supplies only `FLHUIHud::CompletionNotice` text;
there are no structured reward item rows or reward IDs in the read owner. No icon
is guessed by parsing a reward notice. Adding actual reward rows requires an
integrator-owned session/read-adapter change outside this task's owned paths.
This part of the requested coverage is therefore incomplete.

In-engine visual contrast/translucency sorting and controller interaction need
host review; no screenshot, packaged launch or play result is claimed. Windows
checks remain deferred. Generated packages/receipts, exports and build products were removed at
**2026-10-12 05:26 UTC**, more than 20 minutes before the two-hour deadline.
All 452 tracked Content/arrival-review files were restored to exact baseline
bytes, including LFS pointers. The coordinator must import before asset tests/cook.

## Final receipt correction and handoff

Cleanup revealed accidentally staged generated Python bytecode had entered the
original receipt signature. No bytecode remains in the candidate. Signatures now
filter tracked authored file types and exclude relative output/cache directories.
The unit check covers tracked bytecode and relocation beneath a directory named
`output`. Editor rebuilt successfully in **226.44 seconds**; a fresh import exited
**0** and two default-stage replays exited **0**, both reporting
`PROPS_IMPORT_UNCHANGED`, including the replay with exports absent. Final receipt
signature: `a4f4c67601aa3024c24b3cdf0b83f9b4e640411f5bf396bbe69ac342451bcffc`.
The final fresh generation is the byte table above; the earlier generation used
for native suite/cook occupied 1,697,178 imported bytes. Only the signature helper
and its source-only test changed after those runtime checks; the final generation
was not recooked or rerun through native automation.

Next task: coordinator wires the stage, generates/commits packages and receipt,
exposes structured reward IDs and completes reward rows, reviews rendered UI, and
binds world/equipment/effects in the follow-up. This is submitted evidence, not a
verified-success or gate declaration.

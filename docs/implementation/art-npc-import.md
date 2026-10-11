# W5-18 — receipt fix and NPC scope checkpoint

Evidence candidate; **partial and blocked**, not an import, build or gameplay acceptance result.

Task ID: W5-18.
Base revision: `7728272561214927936f12add7ed21a813ae0ba0` (the canonical attempt baseline, overriding the older ffc2ba7 brief label).
Result revision: submission candidate containing this report; exact hash is in the submission receipt.
Contract revision: 1.
Owned paths changed: `build/build-art.sh`, `docs/implementation/art-npc-import.md`. No binary assets.

Behavior changed: the build script stages the existing player Python importer in ignored `Saved/ArtExport/import_player.py`, replacing its single compiled-module signature input with `LHPlayerVisual.h`. The importer already hashes `LHPlayerVisual.cpp`, appearance JSON, all eight source exports, its script bytes and the engine version. Import behavior otherwise remains the source importer's behavior. The adapter checks that the expected input occurs exactly once and fails if the upstream layout changes. Generated adapter bytes include the complete original importer, so changes to that importer still invalidate the receipt. Direct invocation of the original player importer continues to have the old behavior; invoke through `build/build-art.sh`.

Source-backed mechanics: none changed.
Provisional tuning: none introduced.

## Scope checkpoint

The retained worker instruction says: “If the right change needs a file outside them, stop and say so in your report.” The NPC portion is stopped at this boundary:

- `Source/LighthavenEditor/Commandlets/LHGenerateBasementAMapsCommandlet.cpp:520–522` creates Nevanis and Shovanis markers, then independently creates their fallback boxes.
- Its `Box` helper at line 248 creates unrelated `AStaticMeshActor`s named `Geometry_%04d`, carrying generic generation/provenance tags. They have no NPC identity or marker ownership. Attaching a character to the marker would leave the old box visible inside it.
- The source NPC README explicitly calls out these unattached proxies. Hiding arbitrary geometry by location/shape would couple presentation to incidental generated geometry and could hide other dressing. This report does not implement that heuristic.

Smallest requested integrator seam: identify/attach the two fallback actors to their corresponding markers (or add an explicit, persistent NPC fallback identity) in the generator-owned code. Existing generated maps additionally need a deliberately reviewed legacy adoption rule if the no-regeneration requirement remains absolute. Changing the generator alone would not fix an already saved map. Authorize that migration and its fixtures before continuing replacement/fallback work. No generator, marker, interaction, service, save or map was changed here.

## Checks actually run

UID: 1000. Installed engine and Blender executable paths both exist; lack of installation is **not** claimed.

- `bash -n build/build-art.sh`: exit 0.
- Executed the signature-adapter heredoc, then `python3 -m py_compile Saved/ArtExport/import_player.py`: exit 0.
- Executed the generated importer's actual signature prefix in a temporary filesystem with a stub Unreal module: exit 0. Changing an unrelated C++ fixture and compiled editor module preserved the signature; changing the helper header invalidated it. Evidence: attempt `library/receipt-check.txt`. This proves dependency selection, **not** unchanged recognition of the committed 49 packages.
- `UE_ROOT=/home/brewerm/Downloads/unreal BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64 LH_ART_PLAYER_ONLY=1 LH_ART_SKIP_EXPORT=1 bash build/build-art.sh`: exit 1 in approximately 0.7 seconds. Missing `artsource/player/output/player_a.glb`; validation stopped before the Unreal importer. This is a deliberately export-skipping prerequisite probe, not a full art pipeline run.
- Both pinned `Build.sh` invocations were started with Linux Development, `-WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`, local XDG config/UBA paths and the prescribed XML configuration. Editor reached “Using Unreal Build Accelerator local executor to run 18 action(s)” without any completed action being logged; game waited at “Starting shared memory files at 1”. Both were interrupted at the scope checkpoint with exit 130. These are incomplete checks, not compiler failures or successes. No 10-minute stall/retry diagnosis was performed. HOME was not reassigned.
- `git diff --check`: exit 0. No Content or map pointers were edited.

Commands and trimmed logs are retained in this attempt's output library. No rendered review, Windows package or play observation is claimed.

## Remaining acceptance work

NPC export/import step, shared skeleton/module assembly, material/texture sharing, four idle/talk clips, hard cook references, marker binding, dialogue/service state observation and fallback tests are **not implemented** at this scope checkpoint. Imported byte totals by asset type and per NPC, ≤8 MB budget, source/import bounds within 0.5 cm, upright imported idle and interaction equivalence are **not measured**. Full headless suite: not run; expected/observed counts unknown, no failure names available. Map generation, arrivals 9/9 and hub cook: not run; no results inferred.

The player signature change has no legacy receipt migration. Existing receipts hash the old compiled module and do not store independently comparable source-input evidence. The new signature therefore differs on first use and may trigger one reimport (or the existing read-only asset guard). **Do not integrate this as satisfying unchanged recognition of already committed packages.** The coordinator needs hydrated packages plus the exact legacy inputs/module, or separately reviewed historical input evidence, to migrate the receipt without falsely endorsing stale imports. This checkout contains LFS pointer packages; no package hashes were fabricated or silently accepted. Subsequent receipts created through the staged importer exclude compiled code, but an actual same-input Unreal skip remains untested.

Next task/integration notes: resolve the legacy B1 fallback adoption seam and player receipt migration evidence, then finish W5-18 against hydrated/exported inputs and run both targets, full suite, arrivals and cook. Keep the NPC importer signature limited to source exports, importer/native helper source and engine version. No push, merge, receipt or package commit is part of this candidate.

# W5-18b — fallback identity and UI scope checkpoint

Evidence candidate: **partial, stopped at an ownership boundary**. This does not satisfy NPC import/binding acceptance.

Task ID: W5-18b. Base revision: `17ae743bbf4a70806c491a30760718a3276381c5`. Result revision: the candidate in the submission receipt. Contract revision: 1.

Owned paths changed: `Source/Lighthaven/World/LHWorldMarkers.cpp/.h`, hub and Basement A generator NPC placement sections, `Source/LighthavenTests/Visual/Npc/LHNpcFallbackTests.cpp`, this report. No binary assets, receipts, maps, shared schema, gameplay rules or build-script changes.

## Completed source work

Every hub fallback piece is registered with its marker. Nevanis and Shovanis now create their identical fallback cubes in the NPC placement helper and register them there instead of creating two anonymous cubes after the markers. Registration stores explicit actor references on the marker, sets the figure's owner, and adds `LH.NpcFallback` plus the exact `NPC.*` identity tag. It refuses a figure already owned by another actor. Hub's existing attachment call is retained; B1's boxes remain unattached. Ownership/tags avoid requiring a static mesh to attach to a movable marker root.

B1 dimensions, pivot, rotation, cube, NoCollision profile, navigation flag and mobility are the original Box values. Hub geometry/placement recipes are unchanged. Generated geometry counts and cube names retain the same sequence. All initial fallbacks remain visible. Basement B was searched: it has no placed NPCs and needs no NPC placement edit. Existing maps must be regenerated to obtain these explicit references; no proximity-based legacy adoption is provided.

The marker exposes `SetNpcFallbackVisible` for the subsequent imported visual hook. It hides/restores only explicitly registered actors whose current owner and both tags still match. It changes no collision or interaction state. **No production caller hides the figures yet.** The new automation fixture covers ownership, refusal to steal another NPC's figure, preserving transforms/collision/GUID, hiding exactly the owned figure, restoring fallback visibility, and ignoring a stale wrong-owner serialized reference. This is a source fixture, not a passed test or asset-missing integration test.

Source-backed mechanics: none changed. Provisional tuning: none introduced; original presentation dimensions are preserved.

## Required seam outside this contract

The retained worker instruction says: “If the right change needs a file outside them, stop and say so in your report.” Talk presentation needs the actual NPC associated with the open dialogue/service UI and reliable closure/screen-transition state. The current API does not expose it:

- `Source/Lighthaven/UI/LHUIPresenter.h`: `Screen()` is public, but `InteractionTarget` is private and has no getter or notification.
- `Source/Lighthaven/Framework/LHPlayerController.h`: `JournalPresenter` and `JournalWidget` are private with no public access to either active UI state or target.
- `LHPlayerController.cpp` pauses the world when opening the journal, and closes/resumes it privately. `OnInteractRequested` reports an input request, not accepted UI lifetime; it is also bypassed by the live-session NPC path. Selected target is not authoritative UI ownership.

Smallest integrator seam: add a public read-only controller query returning the actual dialogue/service target (or a presentation-only open/close notification), driven by the presenter's private target and `Screen()`; return no target on every other screen and journal close. The visual component must tick while paused to animate talk in the paused UI. This requires controller/presenter files outside the owned paths. Do not substitute name matching, global pause, selected target, or an input notification for actual UI state. Implementation stopped here; no UI file was modified and no unconnected importer/runtime binding is presented as completed.

## Checks actually run

UID 1000; pinned engine and Blender executables exist. Canonical attempt brief/input/receipts were read; receipts returned an empty list (no update to acknowledge).

- `git diff --check`: exit 0.
- `bash -n build/build-art.sh`: exit 0; unchanged script.
- Source audit: 13 hub NPC placements, two B1 placements, zero Basement B placements. All `Content/` `.uasset` and `.umap` files inspected remain LFS pointers. No binary or receipt was generated/committed.
- Both targets started using the farm XML (non-adaptive unity, UBA executors disabled, shared-memory temp file) and `XDG_CONFIG_HOME=$PWD/Saved/BuildEnvironment/config UBA_ROOT=$PWD/Saved/UBA bash /home/brewerm/Downloads/unreal/Engine/Build/BatchFiles/Linux/Build.sh TARGET Linux Development -Project=$PWD/Lighthaven.uproject -WaitMutex -DisableAdaptiveUnity -UBASharedMemoryTempFile=true -NoUBA`, with TARGET `LighthavenEditor` and `Lighthaven`. Editor reached “Using Unreal Build Accelerator local executor to run 18 action(s)” with no completed compile action; game reached “Starting shared memory files at 1”. Both were interrupted at the scope checkpoint, exit 130. Neither target reported success or compiler failure. No 10-minute stall diagnosis/retry was performed. Exact total build timing was not captured; these were interrupted prerequisite checks, not a timed completed build. HOME was not reassigned.
- `UE_ROOT=/home/brewerm/Downloads/unreal BLENDER_ROOT=/home/brewerm/Downloads/blender-5.2.2-linux-x64 LH_ART_PLAYER_ONLY=1 LH_ART_SKIP_EXPORT=1 bash build/build-art.sh`: exit 1, missing `artsource/player/output/player_a.glb` in the actual validator. This is a skip-export prerequisite probe, not the full art pipeline. Exact elapsed time was not captured.

Trimmed build/probe logs are in this attempt's `library/`. No Unreal test execution, rendered review or gameplay result is inferred from source checks.

## Remaining acceptance work

NPC export/import, shared player skeleton and identical texture reuse, per-module material tinting, hard cook references, idle/talk runtime binding and source/import bounds are **not implemented**. NPC imported bytes: **unmeasured**; no ≤8 MB claim. Full suite: **not run**, expected/observed counts unknown, failure names unavailable. Newly added test: expected one test, observed zero executions. Both-target builds: incomplete. All-map generation, arrivals 9/9, LHValidateWorld zero errors and hub NPC cook: **not run**; current candidate modules must first build and real maps/assets must be generated. Windows checks remain deferred. No Content/map pointer restoration was needed because none changed.

Known limitations: the new test covers the fallback ownership helper only, not every generated NPC or actual interaction equivalence. The import pipeline/player receipt behavior remains exactly the W5-18 baseline; this task does not migrate historical receipts. Existing player input exports and NPC output manifest are absent locally. No guessed receipts or packages were written.

Next task: integrator supplies/authorizes the exact UI state seam, then resume W5-18b importing the two deduplicated NPC kits with body-specific player skeletons, asset-only receipt evidence, ≤0.5 cm bounds checks and ≤8 MB measured package budget. Finish the production hook and acceptance suite after real source exports and successful builds. Coordinator generates/commits LFS packages and regenerated maps. Do not integrate this checkpoint as a completed NPC replacement.

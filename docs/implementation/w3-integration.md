# W3-05 integration candidate

Base: `fcd13ce22d7e169bf5589fda8203dbb036721605`; contract revision 1.
Core revision 1 remains unchanged. No map or asset bytes are authored or committed.

The native five-area registry is the Wave 3 catalog: hub, B1, B2, B3, B4;
nine entrances and eight directed reciprocal portal edges. The catalog hash now
includes ordered area/map identities, entrance transforms, portal endpoints and
spawn IDs/enemy/anchor definitions. Old Wave 2 catalog saves are incompatible and
are rejected by the save store, rather than silently reinterpreting hub `Entry`.
The mechanical profile remains the existing explicitly Prototype profile.

Creation checkpoints at hub `Temple.SafeSpawn`; Continue opens the saved area's
map and places the capsule at the saved registry entrance. Frontend remains the
default game entry; every playable map is explicitly listed in MapsToCook.
Portal interaction selects the nearest portal within 250 cm with a visibility
check and sends a run/epoch-qualified command to W3-04. This distance is Prototype
interaction presentation tuning, not an authentic T4C mechanic.

Session hooks supply completed-action capture, persistent command/movement blocking,
checkpoint sequence publication, canonical authority import and collision-checked
avatar placement. Destination installs remain subject to the W3-04 unique reviewed
marker and world validation checks. Travel does not award anything. Failed travel
uses W3-04 source restoration; persistence Retry invokes its retry state machine.
Startup with invalid/unreviewed arrival returns to the frontend, retains the durable
save and reports the refusal; it does not create an arrival checkpoint.
W4 still owns combat, lifecycle settlement and enemy population; no AI is spawned
by these adapters. Live enemy blocking overlap also rejects arrival.

## Automated arrival approval

`Lighthaven.Integration.Wave3.ArrivalSafety` loads all five real maps and logs each
of the nine IDs with transform hash and separate count/transform/capsule/floor/nav
results. Both marker actor transform and SafeArrivalTransform must match registry.
The largest currently defined player capsule is read from ALHCharacter's CDO
(35 cm radius, 90 cm half-height). A 2 cm lift avoids treating floor contact as
penetration. Ground tracing allows 5 cm height error and requires upward normal
Z >= 0.7. These tolerances are Prototype validation policy, not historical values.

Each isolated loaded world finishes static-mesh compilation before creating
physics/navigation/AI, registers actors, requests
`RebuildNavigation`, and ticks for a bounded 10 seconds of wall time before querying.
The test invokes the engine's reflected `K2_ProjectPointToNavigation` function with
25 cm horizontal and 50 cm vertical extent, enforcing horizontal displacement <=25 cm.
This is actual Unreal nav projection, not a floor-trace substitute. Missing navigation
API, navmesh, bounds, build completion or projection fails the ID. Host validation
must establish that navigation builds on all generated maps; the timeout is not proof.
No BeginPlay simulation, enemy fights or geometry edits are part of the review.

`build/review-arrivals.sh` refuses root and LFS pointers, removes stale walk evidence,
runs only the headless walk-test with the documented W3-04b startup flags, and reads
its completed automation report plus `Saved/ArrivalSafety.tsv`. It atomically refreshes
`Config/Lighthaven/ReviewedArrivals.tsv` using only PASS rows. A partial completed run
writes only passing IDs and exits 1; startup/incomplete evidence leaves the list unchanged.
The list is absent in this candidate: no geometry walk passes are claimed.
The generators read the list and set reviewed only when ID and canonical-transform
SHA-1 hash match; moving a transform invalidates the approval automatically. SHA-1
here is a deterministic change fingerprint, not a security signature. Never edit the
list or markers by hand. Geometry changes without transform changes require rerunning
the walk-test; the fingerprint covers transforms, not arbitrary map geometry.

## Packaged content check

All cooked runtime session startups check package existence for the frontend and five
registry maps, blocking character commands if content is incomplete. Launch the Linux
Development package with `-LHCheckPackagedContent -nullrhi -unattended -log`: it logs
per-map package lookup and exits with status 0 only if all required packages exist.
This runs in the game runtime, avoiding reliance on the editor-only test module.
`Lighthaven.Integration.Wave3.PackagedContent` supplies an editor preflight only;
a successful editor lookup does not establish that the package contains the maps.

## G3 packaged checklist — coordinator host work, pending

- [ ] Hydrate LFS maps. Build editor and game with `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
- [ ] Run `UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh`; require nine PASS IDs and commit the generated reviewed list.
- [ ] Regenerate hub, basement A, basement B on the host with their existing scripts (W3-04b memory-DDC/nullrhi/home-screen flags if startup needs them). Only coordinator commits maps through LFS.
- [ ] Run LHValidateWorld on all five regenerated maps with zero errors. This includes the cross-map duplicate-ID scan, all required markers, reciprocal portals and reviewed entrance checks.
- [ ] Run full Lighthaven Automation, including Wave3 traversal/reload, review hash and actual arrival walk on regenerated maps.
- [ ] Package Linux with the explicit six-map command below (the existing wrapper's argument-free default is still the G1 map set, outside this task's owned paths); run packaged `-LHCheckPackagedContent`, retain exit status and per-map log.
- [ ] Create new character at hub, interact to descend hub → B1 → B2 → B3 → B4, and return B4 → B3 → B2 → B1 → hub. No direct B4-to-hub portal is invented.
- [ ] Quit/reload at hub and on each floor, including each descent/return entrance; confirm saved identity, valid capsule placement and unchanged rewards/timers.
- [ ] Exercise missing package, invalid/blocked spawn and corrupt area data: refusal or durable source restoration; no silent hub reset, no arrival save on failure.
- [ ] Check door/stair collision and navigation using the largest enemy placeholder on all connecting apertures; walk-test player capsule coverage does not certify enemy doorway clearance.
- [ ] Retain packaged gameplay evidence. Windows packaging/launch is deferred per project memory, not passed.

Native traversal/reload tests exercise the production save store/codec/travel adapter
with in-memory A/B storage, eight registry edges and independent reloads at source
and arrival. They assert unchanged gold/XP and safe missing-destination/blocked-arrival/
corrupt-world outcomes. They do not exercise OpenLevel or physical disk/process crashes.

## Validation observations

Final build/test observations are appended after execution; G3 remains pending real
map review, regeneration, validator and packaged traversal regardless of compilation.

Exact Wave 3 package command (required because the wrapper supplies `-map`):

```sh
UE_ROOT=/home/brewerm/Downloads/unreal bash build/package-linux.sh \
  /Game/Lighthaven/Maps/L_Frontend \
  /Game/Lighthaven/Maps/L_LighthavenTempleDistrict \
  /Game/Lighthaven/Maps/L_TempleB1 /Game/Lighthaven/Maps/L_TempleB2 \
  /Game/Lighthaven/Maps/L_TempleB3 /Game/Lighthaven/Maps/L_TempleB4
```

Final-source checks observed in this worker (UID 1000, UE 5.8.3 Linux):

- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`:
  exit 0, 12 seconds combined shell wall time; editor `Result: Succeeded`
  (9.33 s UBT), game `Result: Succeeded` (2.05 s UBT). The preceding full
  compile also built both targets successfully (54.02 / 105.26 s UBT).
- Direct headless `UnrealEditor-Cmd` with `Automation RunTests Lighthaven; Quit`,
  `-DDC-ForceMemoryCache -nullrhi -unattended -nosound`, HomeScreen disabled,
  worktree XDG_CONFIG_HOME, report and absolute log paths: exit 255 in 60 s.
  Exported final report: **79 Success, 1 Fail, 0 notRun/inProcess**.
  Only ArrivalSafety failed: all nine rows have count/transform/capsule/floor/nav
  zero because real map worlds could not load from the LFS pointers. No arrival
  approval is inferred. TraversalAndReload, SessionCheckpoints, ReviewTransformHash
  and editor PackagedContent preflight succeeded. The latter finds package filenames
  even for LFS pointers; it does not establish loadable or packaged map content.
- `bash -n build/review-arrivals.sh` and `git diff --check`: exit 0.
- `UE_ROOT=/home/brewerm/Downloads/unreal bash build/review-arrivals.sh`: exit 1
  at the LFS-pointer preflight, before editor startup or list creation.
- `git lfs pull --include='Content/Lighthaven/Maps/L_*.umap'` returned 0 but
  explicitly skipped checkout because LFS is not installed for this repository;
  an explicit filter-process retry behaved identically. `git lfs checkout` also
  refused checkout. All tracked maps remain pointer files, unchanged.

Evidence is retained in this attempt's worker-output `library/`: final build log/
timing, final automation index/log/timing, nine-ID arrival TSV and script rejection.
No map generation, successful geometry walk, LHValidateWorld, cook, packaged content
invocation or gameplay check was performed. Missing real LFS maps is the concrete
prerequisite; coordinator host actions above remain required. Startup refusal and
checkpoint tests use native seams and do not establish real map/OpenLevel behavior.

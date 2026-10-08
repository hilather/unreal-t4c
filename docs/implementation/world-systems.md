# W3-04 world systems candidate

Contract revision 1; base `61536edb4f91f31b67b0fcdb71d7c6789c7ba15b`.
This implements native world authoring, state, travel and validation APIs. It is a
review candidate, not a G3 gameplay or safety approval. Core revision 1 is unchanged.
No binary maps/assets, reference art, mechanical rewards or historical tuning are added.

## Registry and authoring

`LHWorld::Registry()` returns five `FLHAreaDefinition` value records built from
frozen Core `FLHEntranceDefinition` and `FLHPortalDefinition` DTOs. Map soft paths
are `/Game/Lighthaven/Maps/L_<area suffix>.L_<area suffix>`. The integrator can
materialize `ULHAreaDefinition` assets from these records; no new Core type is
introduced. Every portal template has zero RunId; materialize its authored
InstanceId and origin area under the campaign RunId before issuing a request.

| Area | Entrance | Candidate ground pivot (cm), yaw |
|---|---|---|
| Area.LighthavenTempleDistrict | Temple.SafeSpawn | (800,500,0), 90 |
| Area.LighthavenTempleDistrict | Temple.Descent | (-400,500,0), 0 |
| Area.TempleB1 | Entry / Descent | (450,-1650,0), 90 / (4500,1400,0), -90 |
| Area.TempleB2 | Entry / Descent | (500,-5450,0), 90 / (5300,5750,0), -90 |
| Area.TempleB3 | Entry / Descent | (3200,500,0), -90 / (4700,6100,0), -90 |
| Area.TempleB4 | Entry | (650,600,0), 0 |

Fallback is Temple.SafeSpawn in the hub and Entry on each floor. Fallbacks are
explicit candidates; travel failures restore their actual source entrance rather
than silently selecting fallback/origin. Four separately authored pairs give eight
directed edges: hub Temple.Descent ↔ B1 Entry; B1 Descent ↔ B2 Entry; B2 Descent ↔
B3 Entry; B3 Descent ↔ B4 Entry. B4 has no onward link.

All coordinates/counts are inherited Prototype proposals from V-01 layout documents,
`LH_Prototype_v1`, authored 2026-10-07 and transcribed 2026-10-08, source URL null.
Transform resolution means the candidate has numeric coordinates; it does **not**
mean collision, attacks, camera, loading or gameplay safety was verified. Final
transforms and catalog freezing remain the integrator's review decision. Marker
`bSafetyReviewed` defaults false and both editor validation and runtime installation
reject it until host review. Add actual pawn capsule half-height to ground pivots;
never use the ground coordinate as a capsule center.

The registry also carries alias → independently minted literal SpawnId GUID mappings
for all 77 specified anchors: B1 17, B2 25, B3 22, B4 13. IDs were minted once during
this task and stored as constants; runtime does not derive them from names, indices,
coordinates or pointers. The registry source is the authoritative mapping artifact
for map generators. Moving an anchor preserves its GUID. Do not rerun the worker's
one-off authoring script or regenerate GUIDs. B2 Dungeon Bat remains provisional;
Undead Bat uses the selected B2 profile without resolving documentary disagreements.
No enemy statistics, loot, respawn intervals or new Bible-derived mechanics are added.

Placeable classes are in `World/LHWorldMarkers.h`:

- `ALHPortal`: PortalId, Source, Destination, Descent/Return direction; native
  `Materialize(Run)` makes a runtime entity. Portals use explicit interaction;
  there is no overlap travel or arrival bounce loop.
- `ALHEntranceMarker`: qualified EntranceId and absolute SafeArrivalTransform,
  distinct from marker/departure trigger placement; safety review defaults false.
- `ALHSpawnMarker`: Area, SpawnId, EnemyDefinitionId. `ResolveLife` returns persisted
  generation or initial zero; duplicate/mismatched/invalid saved entries fail.
  It does not spawn actors, reset health or increment a generation on reload.
- `ALHInteractableMarker`: Area, InstanceId, DefinitionId; materialize under RunId.

These are scene-root marker actors, without automatic meshes, interaction traces,
rewards, AI or ticking. Map authors may attach the existing primitive presentation
assemblies described in `art/placeholders/README.md`. GUIDs default invalid; assign
registry GUIDs to canonical spawn/portal markers and newly authored GUIDs to objects.
Duplication deliberately preserves identity so validation catches accidental copies;
map authors explicitly mint a new identity for a genuinely new persistent object.

## Coherent travel

`FLHTravelCoordinator` performs source save → destination load/validation → arrival
save, with each asynchronous step receiving a distinct monotonically increasing
completion ticket. Stale or repeated notifications cannot consume the next step.
`ILHTravelHost` isolates authority/avatar/map operations from the state machine;
`FLHTravelSaveAdapter` bridges actual `FLHSaveStore` events, writes and retries.
`ULHWorldTravelSubsystem` binds real OpenLevel, PostLoadMapWithWorld and engine travel
failure delegates. It checks the current map, destination package, matching loaded
map, a unique reviewed entrance marker and registry transform before installation.

Begin accepts only the registered portal's exact permitted destination, current
origin area, campaign RunId and session request epoch. Freeze precedes action
settlement/capture; the host must finish/cancel the active action at a coherent
boundary and suppress input, AI, transactions and unrelated autosaves across map
loads. Source Character.ActiveEntrance becomes that portal's registered source
entrance. A first-visit destination gets an empty area metadata record in this source
checkpoint, without inventing encounters or rewards; W4 owns explicit population. Await a durable save result before OpenLevel. No arrival write occurs on
missing map, wrong entrance, unsafe marker, corrupt world or failed avatar install.
Restore canonical source and source entrance on destination failure; restoration
failure remains visibly frozen with retry available. No automatic hub/origin reset.

Arrival retains character, inventory, RNG, cooldowns, all world records and reward
claims verbatim, changing only ActiveEntrance and save sequence. SafeRespawn remains
the separately authored death checkpoint, rather than being overwritten by travel.
Install/rebuild and validate the destination before arrival save, then wait for
readback-validated save success before unfreezing. Store-generated high-water
sequence becomes the basis of the arrival sequence. Both successful checkpoint
notifications publish the actual durable sequence back to the session through
CheckpointDurable; an arrival retry must not leave its authority sequence stale. Travel grants no XP, gold, loot,
quest progress or claims and does not advance timers. A new request may travel back
normally after completion; repeated pending requests are rejected without a write.

Failed source save offers retry or cancel and never loads destination. Failed arrival
save stays frozen with retry; it cannot cancel by pretending source is still newest
when the failed write may already contain valid bytes. Source restoration has a
separate retry phase. Error detail is retained for presentation. The storage adapter
refuses travel when an unrelated write/dirty checkpoint is pending. A source cancel
retains the store's dirty source snapshot; the session must continue its existing
persistence error/retry gating rather than silently discard it.

On a process crash before a durable arrival generation exists, normal A/B loading
selects the source ActiveEntrance. No separate pending-travel schema or rewards are
serialized. A fully written/readable arrival generation may survive even if the
process dies before receiving its completion callback: then normal store load
selects arrival. This is a durable commit, not proof a UI callback happened. No claim
of atomic physical writes or recovery from every filesystem failure is made.

## Area state and corruption

`ULHAreaStateSubsystem` copies validated campaign area records into the game-instance
lifetime and copies them back without overwriting global quests, bosses, claims or
portal unlocks. Hydration failure leaves the old state intact and reports an error;
missing records remain absent, permitting explicit first-visit initialization by
W4 rather than inventing an alive enemy/default reward. Unknown area/spawn, wrong
run/origin, duplicate identities, corrupt lifecycle/reward and D05 bounds fail closed.
`StoreArea` prevents dropping an encounter's latest high-water record, generation
rollback, forgetting settled rewards or undoing permanent defeat. W4 remains the
trusted owner of whole kill/loot/respawn transactions and area snapshots; this API
does not award kills, tick respawns or use wall-clock catch-up. Alive actors may
return to authored anchors while retaining saved health/lifecycle. Full snapshot
codec validation still runs before saving and loading, including inventory and
cross-container ownership constraints outside the world-only view.

## Validation and native test candidates

`LHWorld::ValidateRegistry` checks all five areas, fallbacks, numeric entrances,
spawn/portal identity uniqueness and exactly one inverse edge per portal.
`LHWorld::ValidatePlacements` consumes synthetic records or an editor scan and
requires all five maps, all nine entrances, all eight portals and all 77 spawn
slots. It rejects duplicate IDs even across kinds/areas, missing/unauthorized or
nonreciprocal portals, direction mismatches, invalid spawns, duplicate entrances,
unreviewed/mismatched/nonfinite transforms and absent required markers.

Editor commandlet `LHValidateWorld` loads the five registry maps read-only, scans
marker actors without BeginPlay/simulation, and returns 1 on missing maps/invalid
placements, 0 only after complete placement validation. Missing LFS hydration is
an error, never a skipped map. It writes no packages. Run as the non-root host:

```sh
UE_ROOT=/home/brewerm/Downloads/unreal
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Lighthaven.uproject" \
  -run=LHValidateWorld -unattended -nop4 -NullRHI
bash build/run-tests.sh Lighthaven
```

`Lighthaven.World.*` native Automation candidates cover registry counts/identity,
portal pairing, duplicates/transforms, travel success/checkpoint resume/no rewards,
missing destination/load failure/wrong entrance/source restoration failure, source
and arrival save retry/cancel, corrupt arrival rejection, hydration preservation and
high-water persistence. The eighth test, StoreDurabilityAndSourceResume, uses the actual production
codec/store and synthetic in-memory generation storage: source readback before load,
independent source reload before arrival, failed arrival write preserving source,
arrival retry, independent destination reload, actual sequence publication and
retained gold/XP/death checkpoint. It does not simulate an OS/process crash.
Synthetic values are test sentinels, not tuning or historical mechanics. Callback mocks prove native sequencing only, not packaged OpenLevel,
filesystem crash survival, pawn collision or actual enemy safety.

## Integrator requests and remaining host work

No Framework/Persistence/Core/configuration paths were changed. The scope checker
permits only the four declared paths. Required session changes exceed a small seam:

1. Replace the Dev_Movement-only W2 mechanical/catalog closure and reference validator
   with the five-area registry, canonical entrances/portals/encounters and Prototype
   provenance. Review ContentRevision/rules compatibility; old W2 `Entry` at hub
   cannot silently become Temple.SafeSpawn. Creation/recovery uses reviewed hub
   Temple.SafeSpawn. Explicitly reject/migrate incompatible old saves.
2. Configure `ULHWorldTravelSubsystem` once with the same authoritative SaveStore and
   compatibility as the session. Supply action-boundary capture, persistent freeze
   of interaction/AI/autosaves, CheckpointDurable publication and checkpoint avatar
   install/safety validation.
   Route `FLHRequestTravelRequest` from authority and UI errors/retry/cancel into
   the coordinator. Maintain save error gating after a cancelled failed source write.
   Disable early input/AI/session bind during transitional map startup.
3. W3 map owners place the registry markers and reviewed safe geometry; W4 populates
   area records and captures trusted lifecycle/interactable changes before travel.
   Add all five maps to cook configuration, replace startup travel with the reviewed
   hub checkpoint, hydrate/generate actual LFS binaries on the host and run validators.
4. Host-run full Automation, both-way hub→B4→hub travel, quit/reload at each stage,
   missing-map/wrong-entrance recovery, disk write/readback failures, crash before
   arrival durability, source restoration failure/retry, safe arrival with live
   enemies and duplicate IDs across maps. Windows packaging remains deferred.

Worker `id -u` returned **0**. Unreal Automation/editor/commandlets/cook/package/play
were not run because Unreal refuses root execution. Build observations and exact
commands/timing are appended below after final source validation and recorded in the
attempt report/library. Compilation cannot establish gameplay, checkpoint durability,
geometry safety or G3 completion.

## Final build observation

Final-source command actually run:
`UE_ROOT=/home/brewerm/Downloads/unreal bash build/build-linux.sh --game`.
Exit **0**, measured combined wall time **43.699 seconds**. Editor reported
`Result: Succeeded`, UBT execution **12.33 seconds**; game reported
`Result: Succeeded`, UBT execution **30.71 seconds**. The native test module linked
in the successful preceding editor builds; final runtime source changes also built.
Evidence: attempt output `library/build-final.log` and `library/build-final-time.txt`.
UBA warned that some action-result store tasks did not succeed without preventing
build success. Static source agreement checks found all 77 aliases/enemies/anchors
matching the layouts and all 85 authored spawn/portal GUIDs distinct. Whitespace
and owned-path checks are recorded in `library/checks.txt`. No Automation/editor/
commandlet/package/play or G3 pass is inferred from these observations.

# Schema revision 2: UseItem

Status: **Proposed** — W4-08 decision candidate for coordinator acceptance. Base `92b8431eb6692c2360cb56fd04dc69699709ee31`; contract revision 1. Implements Matt's 2026-10-09 Q5 decision. This document does not approve a gate or integration.

## Contract changes

- `FLHUseItemRequest { FLHRequestId Request; FLHEntityId Item; FLHEntityId Target; }`. Item identifies an owned inventory instance. An entirely unset Target means self; a partial identity rejects. An explicit Target needs a valid run, playable origin area and instance. Owner comes from handler context.
- `ILHCommandHandler` has twelve typed Execute overloads, including UseItem. The session returns UnresolvedRules until W4-06 wires the domain effect. The UI test double gains the matching overload.
- Append `NotUsable` (no use effect) and `NoEffect` (nothing would change, such as full mana). Rejection consumes nothing. Existing enum values remain unchanged; new names round-trip through UEnum tokens.
- `LHSave::CurrentSchemaVersion=2` is published in Core/LHSaveSnapshot.h, visible through LHSaveCodec.h. Save header defaults, snapshot validation and Character Import use it. Current writes require schema 2; version 1 is accepted only through migration. Versions greater than 2 are FutureSchema; 0/negative/undispatched versions are UnsupportedSchema.

UseItem is **persisted**, because consumption changes inventory. W4-06 must validate the selected epoch, ownership, targeting and effect on a snapshot copy, commit a receipt and save via its command wrapper. UseAbility remains runtime-only until settlement (D16). This task adds no gameplay effect, consumption, potion tuning or new saved fields.

## Frozen bytes and digests

D04 bytes and D05 limits remain unchanged. PayloadCodec stays **LHCanonicalBinary1**: there is no saved payload layout change. LHSaveWireV1.inl remains unchanged as the bounded field allowlist; LHSaveWireV2.inl explicitly delegates the identical v2 save shape to that frozen layout and defines the new request fields. Future shape changes must preserve the v1 decoder and split the v2 layout before editing shared encoders.

UseItem is allowlisted alongside the three existing persisted request digests. Its envelope is `{Command="UseItem", Domain="LHRequest1", Request={Item, Request, Target}}`, ASCII field order, existing entity/request encodings. Structural validation requires nonzero request and epoch GUIDs, a valid item and unset-or-valid target. Registry/ownership and matching the selected epoch require authority context; a structurally valid different epoch changes the digest and is not authenticated by this stateless codec. Save validation rejects receipts with an epoch differing from Session.RequestEpoch.

| Vector | Bytes | SHA-256 |
|---|---:|---|
| Frozen production rev-1 PersistenceFixture | 17,373 | `9d12cfabc98249b6ecbff1451e51a3b0148e7fa9adb6c511bf4712fb0ff7a636` |
| UseItem / LHRequest1, self target | 359 | `454d96b1305633a0d79293e4b503d2a3b8c3fb1552541f8a84708e86e8449f37` |

The rev-1 fixture was encoded by the pre-bump production codec and frozen in commit `a79f571`, in LHSchemaRev1Fixture.inl, with its full-file SHA-256. Its independent synthetic Ruleset.PersistenceFixture/content hashes avoid G3 catalog compatibility assumptions. The migration test compares the complete re-encoded file to this vector with only SchemaVersion changed, preserving every character/world/session field. The request golden was assembled independently from D04 little-endian field encodings and hashed once; request Value=(1,2,3,4), Epoch=(5,6,7,8), item Run=(1,2,3,4), Area.TempleB1, Instance=(9,10,11,12), Target entirely unset. Existing request/reward goldens in LHIdentityTests.cpp are unchanged.

## Migration and compatibility

Dispatch is explicit: 1 → DecodeV1AndMigrate; 2 → DecodeIdentityV2. Both perform bounded preflight/read and exact canonical payload comparison. Migration changes only the version and validates the resulting current snapshot. Growth history, claims, finalized loot, receipts/epoch, sequence and lifecycle high-water facts remain byte-identical. The canonical envelope comparison uses a copy of the **original pre-migration header**. Output is assigned only after full validation, including the authoritative reference callback.

No automatic disk rewrite occurs on load. FLHSaveStore already writes opposite the newest valid generation; no store edit is needed. A failed/truncated v2 write leaves the original v1 slot intact, and retry targets the other slot. Successful writes still retain the original generation. Schema migration does not override ruleset/content compatibility: G3-era catalog saves may reject once the W4 catalog changes, as accepted by Matt Q9. Rev-1 builds reject rev-2 files as FutureSchema before payload allocation. Rev-2 receipts may now represent UseItem digests.

## Coordinator corrections and downstream handoffs

Apply accepted corrections to contracts-v1.md: twelve Execute overloads; add the UseItem row and reasons; describe schema 2 current writes and schema 1 migration; separate inventory consumption from runtime UseAbility. Update persistence.md's current-version/digest dispatch descriptions. Those documents remain coordinator-owned.

D09 amendment (Matt Q4): Balork may use `RespawnPolicy=OrdinaryRepeat` with the Bible's **15:00** respawn from [the Bible monster chart](https://www.t4cbible.com/monster1), retrieval 2026-10-09 in the R-03 lookup (live chart, ruleset/server version unspecified). Its boss-completion reward remains single-claim via **BossUnique**, across respawn, reload and travel. Defeat/life facts must not implicitly grant completion. No respawn or reward policy code changes are made here.

- W4-03 implements the atomic UseItem domain effect and consumes NotUsable/NoEffect without adding Core reasons.
- W4-05 sends this typed inventory/hotbar intent, with Item instance, current Request epoch and unset Target for self.
- W4-06 replaces the unsupported handler stub, enforces context/epoch/ownership, wraps receipt/save and publishes only completed transactions. W4-04 starts from the accepted merge for persistence work.

## Validation evidence

Native cases: Core.Schema.UseItemRequest, Persistence.UseItemDigest, SchemaRev1Migrates, SchemaRev2RoundTrip, MigrationKeepsOriginalSlot; existing FutureVersionAndAlgorithms now uses 3 for FutureSchema. Runtime counts, build logs and remaining limitations are recorded in the attempt report; this proposal is not an observation of downstream gameplay.

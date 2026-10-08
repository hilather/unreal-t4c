# Balork — final-encounter presentation companion

This is staging guidance for **`Enemy.Balork` → `Presentation.Enemy.Balork`**, using the [Balork creature spec](balork.md). It is not another gameplay definition, a new arena layout or an implemented encounter. A-01 / LH_Prototype_v1 / 2026-10-08: all presentation decisions and any new numerical proposals are Prototype, source URL null. Existing V-01 geometry below remains inherited Prototype.

## Scene identity and visual focus

Use the existing B4 labelled boss complex and [B4 layout](../../layout/b4.md). The [world ledger](../../world-ledger.md#balork-and-completion-semantics) supplies the defeat → return to church → save objective and authored permanent-defeat campaign policy. No source image establishes locked doors, summons, additional combat phases, a victory cutscene or an extra completion reward. Keep the player’s retreat route visible.

The local [map reference notes](../../map-reference-notes.md#m4--lighthaven-dungeon-level-4-classic) describe darker gray-brown masonry, warm wall torches, a raised altar-like fixture and the upper-right Balork label. A-01 read those notes; **A-01 did not open the map PNG**. The actual images opened for this companion are `balork-demon-family.jpg` and `deep-dungeon-creatures-concept.png` under the read-only package root in the [image register](README.md#images-actually-opened-and-rights).

Observed in those creature images: patterned red/dark wing mass, horned dark-red demon and long pale-edged polearm; the source crops extremities, while the concept adds a complete frontal body and paired wings. Retain that identity, but do not copy the studio sheet’s lighting or relative composition scale into the arena. Use cooler fill to separate the dark torso from the floor, selective amber torches on masonry, and broad dull-red wing panels. Review grayscale separation and target-ring visibility before adding detailed markings.

## Required movement and visible envelope

Reference dimensions from [Balork](balork.md#scale-and-collision): 260 cm head, 310 cm horn/animated top, 440 cm wing width × 200 cm fore–aft transit box, approximately 400 cm polearm; capsule R=100 cm / HH=155 cm. The conservative root-centered transit turning circle is 484 cm. All are proposals; full geometry still needs a swept test. The camera is the [controls view](README.md#camera-and-silhouette-rules), including 1200 cm default boom and 45° **horizontal** FOV. Keep normal zoom/orbit control; do not introduce a cinematic camera that hides escape lanes.

| B4 space/link | Existing extent / minimum | Presentation acceptance |
|---|---|---|
| Home/patrol | BalorkArena ∪ Reliquary ∪ Ossuary, excluding walls and altar | Body, full wings and carried weapon clear fixtures at every turn |
| C04 / complex thresholds D05–D07 | 500 × 360 cm doors; C04 550 cm corridor; 600 × 600 cm turn pads | Transit box and compact jab clear; wing tips cannot vanish into jambs |
| Chase extension | C04 to Hall apron `[3,19] × [32,42]` U; proposed turnaround `(11,37)` U | Sweep the entire envelope inside reserved floor; apron edges are not automatically valid actor centers |
| Spawn alias | `B4.Balork.01`, proposed `(12,66)` U, facing −Y | Alias is an inherited design handle; W3/W4 assign persistent identity independently |
| Ordinary C01–C03 / return route | Ordinary 240 cm doors; 320 cm corridors, 300 cm stair | **Not boss routes:** wings/height do not fit; do not shrink or fold him to pursue here |
| Dev_Movement boss ruler | 500 cm width / 360 cm headroom | Checks the aperture only, not B4 turns, corridors, stairs or chase behavior |

Here U=100 cm from V-01, not a source-tile conversion. Balork is not required to use the B4 return stair. An optional wide swing with 520 cm swept diameter would fail the 500 cm doors; the required compact jab avoids that dependency. No invisible player wall or boss-only damage immunity is added. W3/W4 must still investigate ranged safe spots at the altar, Hall boundary and side doors; presentation cannot repair that gameplay risk.

## Rig, animation and event staging

Use the creature spec’s floor root, demon skeleton, two wing chains, both polearm grip sockets, both cosmetic tip sockets, hit/UI anchors and bounded death pose. The minimum set is idle, ground locomotion/start/stop/turn, basic attack windup/impact/recoil, hit, death and inert dead hold. An alert pose is optional. There is no flight, root-motion charge or phase-transition set.

| Authority/lifecycle state | Presentation response | Boundary |
|---|---|---|
| Alive/acquiring | Idle/turn blends; display the normal target identity through existing UI integration | No forced camera, dialogue lock or alert invulnerability |
| Native attack commit | Begin in-place anticipation from the approved commit signal | Hook is not exposed yet; raw input/proximity is insufficient |
| Commit + `ImpactSeconds` | Compact polearm contact key: **preview f30 at 30 fps = 1.000 s** | Native timer resolves the one impact; no damage notify |
| Resolved hit/miss | Cosmetic feedback consumes `OnImpact(FLHHitIdentity, Result)` once | A validation failure may emit no event; old cues must still end/cancel |
| Recoil | Proposed preview f31–f42 returns to stance | Current 3 s cooldown starts at commit; recoil does not extend it or lock movement |
| Authoritative death | Lower body/wings into the reviewed corpse envelope, then dead hold | `OnDeath` is not a second loot, XP, mark or completion transaction |
| Already defeated after reload | Hydrate the lifecycle owner’s corpse/absence state | Never replay a live acquisition, death reward or unearned victory cue |
| Return objective / departure | Keep stair route and available target feedback readable | UI/quest/save owners implement the objective and persistence |

Runtime impact delay remains configurable; the 1 s preview is inherited dev-fixture Prototype tuning, not Balork’s historical attack time. An approved replacement delay retimes the contact key. Cancellation/death/activation replacement/travel clears any queued cosmetic event for the old identity. Keep death pose and polearm visually clear of the exit without moving the authoritative corpse or altering loot reach behind the player’s back.

## Placeholder, replacement and evidence to capture

Use the Balork spec’s engine-shape demon assembly: full-width red/dark wings, 310 cm horn tips, 400 cm centered shaft and separate capsule. All visual pieces are nonblocking. The current dev dummy is a combat actor with a separate cylinder ruler, not a boss model or encounter. W4-02/integrator must implement the proposed presentation binding before W5-03 replaces the assembly with original/licensed art. Keep the same `Enemy.Balork`, presentation mapping, spawn/life identity, reviewed capsule and authority parameters across replacement.

After assets and the B4 map exist, capture actual Linux gameplay from the entry to the Hall, through C04 and all boss complex thresholds, around the altar and back to the exit. Exercise both zoom/orbit extremes, full-spread turns, compact attack contact, cancellation, death at a threshold, already-defeated reload and no repeated reward. Check player passing and pale target UI against the wing pattern. These are future checks, not results from A-01; Windows packaging/launch checks remain deferred.

## Provenance and rights

Original: stable ID `balork-demon-family`, `P/assets/references/monsters/balork-demon-family.jpg`; [Fantasy Classic page](https://t4cfantasy.com/Bible/Classic/MonsterImages.php), [exact image](https://t4cfantasy.com/images/skins/app20013.jpg), retrieved 2026-10-07, original artist/client patch unspecified. Concept: `P/assets/concepts/deep-dungeon-creatures-concept.png`, generated 2026-10-07 from the family references per `P/assets/concepts/prompts.md`. Both opened by A-01 on 2026-10-08. **No commercial licence or distribution permission is established.** Concept reconstruction is not permission to redistribute the underlying art; package only separately reviewed original/licensed production assets.

All work here is written guidance and local visual/source inspection. No map, rig, animation asset, material, screenshot or gameplay validation was created by this companion.

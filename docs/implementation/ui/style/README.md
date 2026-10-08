# V-03 — UI visual style guide

**First-pass proposal, open to revision — 2026-10-08.** For W2-03's C++-constructed widgets and the later W4 UI. Matt's first-pass approval of A-01/A-02/V-02 is inherited from the task brief; these additional tokens and specimens are review candidates, not a new art approval. Base: `a807706c30100455d085f41396eb4eef5f68a60c`. Nothing here implements widgets, changes rules, or passes an engine gate.

Start with [colour and type](palette-type.svg), [widget states](widget-states.svg), and [geometric icons](icon-specimen.svg). These are original small text SVG specifications, not runtime screenshots or assets to import as flattened UI. This README is the normative style proposal; the [V-02 screen specifications](../README.md) retain layout, focus order, input behavior and authority ownership.

## Direction and provenance

Keep the existing charcoal surfaces, off-white linen-like text, restrained amber emphasis and cool blue-grey information. Broad flat rectangles and quiet borders echo dark timber, dull metal and cloth without adding textures under text. Use the existing 6 px corner radius, no ornate frames, bevel gradients, glow or decorative serif lettering. Keep red landmarks and creature colours in the world; success and failure use distinct shapes and words in the interface. The visual hierarchy is title → section → labelled controls → supporting metadata.

Source direction: [art register, palette/materials](../../art-register.md#palette-and-materials), [A-01 creature conventions](../../art/creatures/README.md), [A-02 human presentation](../../art/player/README.md), [game design](../../../plan/docs/01-game-design.md), and [architecture §8](../../../plan/docs/02-unreal-architecture.md#8-ui-graphics-and-validation-targets). The brief's `art/player/README.md` resolves to `docs/implementation/art/player/README.md`; no top-level `art/player/` exists at this base.

All dimensions, colour choices and presentation IDs below are **Prototype UI design proposals**, provenance author V-03, date 2026-10-08, profile `LH_Prototype_v1`, source URL not applicable. Values marked inherited come from V-02, not historical measurements. This guide introduces no HP/XP/loot/rank/price values or rarity mechanics. Runtime numbers and their `Prototype`/`Disputed`/`Unknown` provenance remain owner-supplied. Specimen bar lengths illustrate geometry only and represent no game state.

## Colour tokens

Hex values are **opaque sRGB**, alpha 1.0. Mirror semantic names even where values alias: that permits later tuning without conflating focus, warning and XP. Retain the V-02 palette; aliases below add purpose, not new hues.

| Name | Value | Use / V-02 origin |
|---|---|---|
| `Color.Background` | `#171717` | Root, modal surround, bar track; inherited `ink` |
| `Color.Surface` | `#242424` | Opaque reading surface; inherited `panel` |
| `Color.Panel` | `#242424` | Pane/card/modal/tooltip fill; alias Surface |
| `Color.Raised` | `#333333` | Buttons, footer, selected or hovered rows; inherited `raised` |
| `Color.TextPrimary` | `#F2F2F2` | All actionable copy, errors, requirements, numbers; inherited `text` |
| `Color.TextSecondary` | `#C4C4C4` | Metadata, secondary explanation, placeholder text; inherited `muted` |
| `Color.TextDisabled` | `#C4C4C4` | Unavailable action labels at full opacity; alias TextSecondary |
| `Color.Edge` | `#909090` | Boundaries, separators, empty slot/bar outlines; inherited `edge`; **not text** |
| `Color.HoverEdge` | `#C4C4C4` | Pointer hover boundary; alias TextSecondary |
| `Color.Accent` | `#E9BF79` | Small title rule, primary-action inner rule, provenance; inherited `amber` |
| `Color.FocusRing` | `#E9BF79` | Outer focus ring and leading chevron; alias Accent |
| `Color.Warning` | `#E9BF79` | Triangle/exclamation + Warning/Unavailable; alias Accent |
| `Color.Error` | `#E9BF79` | Octagon/exclamation + Error and specific message; alias Accent, intentionally same hue as warning |
| `Color.Success` | `#A9C4CC` | Check + Saved/Equipped/Met; inherited `cool`, no green/red pair |
| `Color.Info` | `#A9C4CC` | Information circle + explanatory text; alias Success |
| `Color.SelectedMark` | `#F2F2F2` | Check + Selected, tab underline; alias TextPrimary |
| `Color.BarTrack` | `#171717` | HP/MP/XP unfilled region; alias Background |
| `Color.BarHealth` | `#C4C4C4` | Neutral Health/HP fill; alias TextSecondary, per HUD neutral-bar direction |
| `Color.BarMana` | `#A9C4CC` | Mana/MP fill; alias Info |
| `Color.BarXP` | `#E9BF79` | XP fill on Character only, if owner provides unambiguous progress; alias Accent |
| `Color.UnknownMark` | `#C4C4C4` | Question mark/diagonal hatch plus Unknown; alias TextSecondary |

There are **no item rarity or quality tiers** in V-02. Use item names, slot labels, supplied requirement text and Equipped/Quest badges. Do not add a common/rare/epic rainbow, loot border ranks, item stars or a graphics-quality selector. A future approved tier would need a textual tier name and shape as well as a token; this task defines none.

### Allowed text pairs and measured contrast

Use only the following foreground/background combinations for text, including captions inside the specimens. All are at least **4.5:1 at every specified size**, including the 16 px metadata floor at 720p and disabled labels. The stricter small-text target is used even for titles. The WCAG AA text criterion is 4.5:1 for ordinary text and 3:1 for large text; this table does not rely on the large-text or inactive-control exceptions. [W3C SC 1.4.3](https://www.w3.org/WAI/WCAG22/Understanding/contrast-minimum.html), retrieved 2026-10-08.

| Foreground / semantic aliases | Background `#171717` | Surface/Panel `#242424` | Raised `#333333` |
|---|---:|---:|---:|
| TextPrimary / SelectedMark `#F2F2F2` | 16.01:1 | 13.87:1 | 11.29:1 |
| TextSecondary / TextDisabled / UnknownMark `#C4C4C4` | 10.28:1 | 8.90:1 | 7.24:1 |
| Accent / FocusRing / Warning / Error `#E9BF79` | 10.41:1 | 9.01:1 | 7.34:1 |
| Info / Success `#A9C4CC` | 9.78:1 | 8.47:1 | 6.89:1 |

Calculated from sRGB relative luminance: normalize channels to 0–1, linearize with `c/12.92` for `c ≤ 0.04045`, otherwise `((c+0.055)/1.055)^2.4`; luminance weights 0.2126/0.7152/0.0722; ratio `(lighter+0.05)/(darker+0.05)`. Ratios displayed to two decimals; pass/fail uses unrounded values. These are token arithmetic, not measured Unreal output or a whole-product WCAG conformance claim.

The Edge boundary has **5.62 / 4.86 / 3.96:1** against Background / Panel / Raised. FocusRing has **10.41 / 9.01 / 7.34:1** respectively. These exceed the 3:1 non-text target. Raised versus Panel alone is insufficient for a control boundary: always draw Edge. [W3C SC 1.4.11](https://www.w3.org/WAI/WCAG22/Understanding/non-text-contrast.html), retrieved 2026-10-08.

Do not lower whole-widget opacity for disabled, hover, fade or pending states. Do not place text over a coloured fill, portrait, world render, texture or translucent panel; this proposal has **no inverse text-on-accent pairing**. Bars carry their label/value in a separate opaque Panel band, never across the moving fill. A coloured swatch has its name outside the swatch. The focus ring sits on the containing opaque Panel/Background, with the parent reserving clearance; it must not cross a bright preview image. Future texture, colour or opacity changes require recalculating the affected pairs.

### Cues that survive loss of colour

| Meaning | Shape and required words | Colour |
|---|---|---|
| Navigation focus | Outer ring **and leading chevron**; one logical menu focus | FocusRing |
| Selected choice/tab | Check + **Selected**; tabs also retain underline | SelectedMark |
| Equipped / saved / met | Check + **Equipped**, **Saved**, or **Met**, from the corresponding owner | Success |
| Warning / unavailable | Triangle containing `!` + **Warning** or **Unavailable**, with reason | Warning |
| Error / rejected validation | Octagon containing `!` + **Error: [message]**; errors still use Body text | Error |
| Unknown / disputed | Question in circle + **Unknown**; divided diamond + **Disputed** | UnknownMark / Warning |
| Prototype | Outlined rectangular **Prototype** badge beside field/group | Accent |
| Pending | Hourglass + **Working…**, **Saving…**, or owner's exact status | Info |
| Health / mana / XP | HP + shield outline; MP + diamond; XP + ascending bars; full resource name and value | Respective Bar token |

The check shape is shared but the word disambiguates Selected, Equipped and Saved. A successful gameplay command cannot display Saved until storage reports durability. Keep field/group provenance visible; Details carries source metadata. Unknown resources show an unfilled, optionally diagonally hatched track and “— Unknown”; never a guessed fraction. Low resources add a static warning and words without flashing, pulse or screen tint. No automatic dismissal of blocking errors. No motion is required for comprehension; the first pass uses static hourglasses, and any later spinner keeps its words and static reduced-motion fallback.

## Typography

Recommend **Source Sans 3**: Medium 500 body, Semibold 600 controls/numeric emphasis, Bold 700 headings, Regular 400 metadata. Its plain, open forms preserve V-02's functional sans-serif direction. Source Sans 3 is distributed under **SIL Open Font License 1.1**, confirmed from [the font's OFL file](https://raw.githubusercontent.com/google/fonts/main/ofl/sourcesans3/OFL.txt), retrieved 2026-10-08. When a later asset owner vendors it, pin the version and include its copyright/OFL notice; no font is downloaded or copied by V-03.

The SVG family list is `Source Sans 3, Liberation Sans, sans-serif`. On this host Source Sans 3 is absent and the specimens render with **Liberation Sans**; this is a layout fallback, not proof of the recommended font's metrics or final Unreal font coverage. Keep a runtime font reference supplied by W2-03's font/asset owner; do not depend on a Linux system font existing in a cooked build. Draw icons as geometry instead of relying on Unicode dingbat coverage. Unicode names/IME and localization need the integrator's composite-font coverage policy; do not invent an ASCII restriction.

| Token / role | 1080p size / line height | 720p size / line height | Weight / use |
|---|---:|---:|---|
| `Type.Title` | 48 / 60 px | 32 / 40 px | 700; screen title, inherited size |
| `Type.Section` | 36 / 48 px | 24 / 32 px | 700; pane/section, inherited size |
| `Type.Body` | 30 / 42 px | 20 / 28 px | 500; descriptions, errors, requirements, field values |
| `Type.Control` | 30 / 42 px | 20 / 28 px | 600; selectable labels/action verbs |
| `Type.Numeric` | 30 / 42 px | 20 / 28 px | 600; tabular lining figures, right-aligned by column |
| `Type.Metadata` | 24 / 36 px | 16 / 24 px | 400; timestamps, provenance, footer, supporting headings only |

Title/section line heights are V-03 proposals; Body/Metadata metrics and all sizes are inherited. **Minimum functional text 30/20; absolute metadata minimum 24/16.** No new small badge font. Sentence case for labels; the product wordmark may retain LIGHTHAVEN. Do not make requirements or errors metadata to fit a card. Use full attribute/slot names. Full resource names accompany HP/MP/XP. Align names left, signed numbers right, units explicitly; keep Base / Gear / Effects / Current / Pending distinct.

All sizes describe target screen pixels at default UI scale, **not values to paste blindly into a Slate point-size field**. W2-03 must calibrate font metrics/DPI and measure the rendered line boxes. Scale the 1920×1080 canvas once; do not combine an extra 2/3 font multiplier with a second viewport/DPI multiplier. Wrap prose; use two-line 108/72-high controls when needed. Grow/scroll content with text enlargement and preserve footer/actions, never shrink-to-fit. A list title may clamp after two lines only if a focusable Details view repeats the full title. Tables that cannot retain readable columns at enlarged text must offer vertically stacked, fully labelled detail rows.

## Spacing, grid and safe area

Inherited V-02 base unit `Space.Unit` = **12 px at 1080p / 8 px at 720p**. Default inset `Space.ControlInset` = 12/8; panel inset `Space.PanelInset` = 24/16; pane gutter = 24/16. Text baseline placement follows line boxes, not arbitrary literal SVG baselines.

| Geometry token | 1920×1080 px | 1280×720 px |
|---|---|---|
| `Layout.Safe` (x,y,w,h) | 72,54,1776,972 | 48,36,1184,648 |
| `Layout.Grid` | 12 columns ×126; gutter 24 | 12 columns ×84; gutter 16 |
| `Layout.Header` | 72,54,1776,96 | 48,36,1184,64 |
| `Layout.Body` | 72,174,1776,720 | 48,116,1184,480 |
| `Layout.Status` | 72,900,1776,48 | 48,600,1184,32 |
| `Layout.Footer` | 72,954,1776,72 | 48,636,1184,48 |
| `Layout.TwoPane` widths | 576 / 1176; origins x72 / x672 | 384 / 784; origins x48 / x448 |
| `Layout.ThreePane` widths | 426 / 726 / 576; x72 / x522 / x1272 | 284 / 484 / 384; x48 / x348 / x848 |
| `Layout.Modal` (x,y,w,h) | 372,282,1176,516 | 248,188,784,344 |
| `Size.TargetMin` (each axis) | 72 | 48 |
| `Size.ControlTwoLine` height | 108 | 72 |
| `Size.RowGap` minimum | 12 | 8 |
| `Size.CornerRadius` | 6 | 4 |
| `Size.EdgeStroke` | 3 | 2 |
| `Size.FocusStroke` / outside gap | 3 / 6 | 2 / 4 |
| `Size.Icon` / icon-label gap | 36 / 12 | 24 / 8 |
| `Size.HintGroupGap` | 36 | 24 |
| `Size.BarHeight` / bar-label gap | 12 / 12 | 8 / 8 |

EdgeStroke is deliberately 3/2 for crisp borders at both outputs (V-02 drawings use 2 px illustration strokes); it does not change target geometry. Inset normal borders inside the target. The focus ring has a **6/4 clear gap from target bounds to its inner edge**, then 3/2 stroke outside: total outward reserve 9/6 per side. Parent clip/scroll padding reserves that area. Neighbouring controls remain separated by at least 12/8; only the focused one has an outside ring. Round rendering to physical pixels without clipping the ring or changing the hit rectangle. Keep the chevron in a reserved leading 36/24 slot rather than shifting text when focus moves.

At 720p multiply canvas positions, sizes, line heights and padding by 2/3. For another aspect ratio, fit the 16:9 canvas uniformly with `scale = min(viewport width/1920, viewport height/1080)` and center it; fill extra surround with Background or original world art. No stretched type. For outputs smaller than 720p, the 16 px metadata minimum requires a separate reflow layout; do not claim automatic support by shrinking further. Font enlargement is additional reflow, not proportional compression of the whole screen.

Use each screen's pinned exceptions: Character's wide-left layout, Creation/Inventory's three panes and HUD's clear zone. HUD does **not** use the full-width Status strip. The 48/32-high standard status strip holds one Body line; long errors keep a readable summary there and full text in the adjacent field/detail panel. Modal title/body/error can scroll above fixed actions, which remain in the safe rectangle. Tooltips stay within the safe area and never cover the focused target, ring or footer; otherwise show the content in Details.

## Widget states

### Shared recipes and composition

These names are presentation recipes, not gameplay enums. `Normal`: Panel for rows/fields/slots, Raised for buttons, Edge boundary, TextPrimary. `Hover`: Raised + HoverEdge; no amber ring or chevron from hover alone. `Focused`: FocusRing outside + leading chevron, present even with no pointer; normal/selected interior retained. `Pressed`: Background interior + inset Edge line, text stays fixed; focus ring retained. `Disabled`: Panel + TextDisabled + slashed-circle/Unavailable and reason; keep a focusable wrapper, never globally dim. `Error`: inner Error boundary + octagon/exclamation + inline Body message; Focused stays outside. `Selected`: Raised + SelectedMark check and “Selected”; no amber outside ring unless also focused.

State priority: availability/pending determines whether activation can submit; pressed affects the interior only when an action is available; hover affects the interior only when available and not pressed. **Focus is an independent outer layer and always survives Disabled, Error, Selected and Pending.** Error message/icon remains visible beside a selected or pressed control. Selection and Equipped marks persist independently of hover/focus. A disabled action may activate its explanation, never its mutation. Pending retains the label/footprint with hourglass + owner status, blocks duplicate submissions and follows V-02's receipt/Back rules. No automatic success or selection changes on pointer hover.

### Per-widget application

Every cell inherits the shared recipe above. “No state” is deliberate, not a missing implementation requirement.

| Widget | Normal | Hover | Focused | Pressed | Disabled | Error | Selected |
|---|---|---|---|---|---|---|---|
| Button | Raised, verb, Edge; primary adds a short inner Accent rule | HoverEdge, same verb | Outer ring + chevron | Background + inset line; no label movement | Unavailable + reason; explain on Confirm | Error message below/in status, keep verb | No persistent state for one-shot actions; toggle buttons add check + Selected/On |
| List row / choice | Panel, name; reserve indicator gutter | Raised + HoverEdge; no selection | Ring on whole visible row + chevron | Inset on row during Confirm | Keep row browseable; slash + Unavailable | Octagon + reason in row detail; preserve stable ID | Raised + check + Selected, independent of focused row |
| Tab | Panel, full name | Raised + HoverEdge | Ring + chevron even for inactive tab | Inset until activation | Label/reason retained, cannot open unavailable view | Octagon beside name; message in active pane | Raised + 3/2 SelectedMark underline + check + Selected |
| Slider / setting | Panel row, label/value, track + visible thumb, −/+ alternatives | HoverEdge on row/hovered subcontrol | Ring entire row + chevron; Confirm enters adjustment | Active thumb gets inset; track doesn't dim | Slash + reason; retain known value | Reason below row; no guessed min/max or false value | Adjustment mode adds check + “Adjusting”; unrelated selection state absent |
| Text field | Panel, persistent external label, entered text; placeholder Secondary | HoverEdge | Ring + chevron; caret only during editing | Inset while opening in-game keyboard/clicking | Readable value + read-only/unavailable explanation | Inner Error + octagon; supplied field message below | Text selection uses Raised highlight + Primary text + underline; no widget-level Selected badge |
| Stat stepper | Full stat label, staged value, explicit −/+ targets | Hover only current arrow/row | Whole row ring + chevron; arrow changes only staged draft | Inset on pressed −/+ | Each blocked direction shows limit/reason; entire row inspectable | Preserve draft + Error reason from preview | Nonzero draft shows signed delta + Pending; entered adjustment has check + Adjusting, never Equipped |
| Equipment slot, including Quiver | Label + geometric slot icon + item/Empty; Panel | Raised + HoverEdge | Ring + chevron on full row | Inset while opening action/details; no speculative equip | Slot still inspectable, Unavailable + reason | Octagon + slot/requirement explanation | Selected badge distinct from persistent check + Equipped; Empty never looks equipped |
| Tooltip / help | Panel + Edge, Body text; opened by hover or focus/Details | Pointer may remain over help without dismissal | Anchor keeps ring; static tooltip has no independent focus | No press state; actionable content opens Details | Disabled anchor's reason stays readable | Octagon + Body explanation; duplicate essential error in field/detail | No selection state; selectable/actionable content belongs to Details |

Text-field selection colours are intentionally restrained; its underline and caret boundaries identify the range without an untested inverse pair. A static tooltip holds no essential requirement absent elsewhere, has no timer while hovered/focused, and can be dismissed without moving focus. Long help uses the established Details reading mode (enter, scroll, Back restores anchor). A live interactive popover must use the normal focusable panel/modal rules, not a pointer-only tooltip.

Sliders use a 12/8-high Background track with Edge boundary and Cool fill, a square 24/16 thumb outlined in Primary, and a 72/48 hit height; labels/numbers stay outside the track. The thumb can be decorative inside the row's full hit region, with explicit 72/48 −/+ alternatives. Settings values/ranges/steps remain from V-02/settings owner. Stat −/+ controls each get 72/48 targets; if the table cannot fit them, South opens the specified adjustment subcontrols, never shrinks hitboxes. No indefinite hold or fine analog precision is required.

Equipment slots are eight labelled rows in V-02 order, not a new paper-doll layout. **Quiver has its own receptacle/fletching icon and full name.** Bow icon belongs to a Main Hand item, not a ninth slot; backpack ownership cannot produce the Equipped badge. Only the resolved Wooden Arrows definition receives the word **Unlimited**; a constant decorative fletching count is not an ammo count. Empty slots have a hollow placeholder + Empty; unknown binding has `?` + Unknown while retaining the slot label and saved identity.

## Iconography and generic input glyphs

Author simple geometry in code: **36×36 design box at 1080p / 24×24 at 720p**, 3/2 strokes, 3/2 inset to the stroke centerline (1.5/1 clear space outside the stroke), no internal detail smaller than the stroke. Use rectangles, circles, lines and closed polygons; sample curves as short line segments if necessary. Outline form, Primary foreground on Panel/Raised/Background. Status icons use the semantic colour above. Never borrow an icon font, platform logo or third-party pack. Every icon accompanies a readable label; count/value is separate text, not tiny digits inside the icon.

Proposed case-canonical namespace: `Presentation.UI.Icon.<Group>.<Name>`. The table lists suffixes to append to that prefix. These are logical **presentation-only proposals**, consistent with A-01/A-02, not asset paths, registered IDs, new enum members or gameplay `Enemy.*`/item/skill IDs. Integrator owns adopting any resolver; for W2-03 a local semantic lookup suffices. A missing presentation resolves to an outlined square + `?` + full supplied name and preserves canonical identity.

| Suffix / visible label | Primitive construction / distinction |
|---|---|
| `Attribute.Strength` / Strength | Horizontal dumbbell: short shaft + two broad rectangular ends |
| `Attribute.Endurance` / Endurance | Five-sided shield with center upright line |
| `Attribute.Agility` / Agility | Two parallel forward chevrons, not a sixth Dexterity attribute |
| `Attribute.Intelligence` / Intelligence | Open book: two quadrilaterals joined at central spine |
| `Attribute.Wisdom` / Wisdom | Eye outline: upper/lower angular arcs + central circle |
| `Slot.MainHand` / Main Hand | Short vertical blade, crossguard and grip |
| `Slot.OffHand` / Off Hand | Round buckler with central boss; distinct from Endurance shield |
| `Slot.Quiver` / Quiver | Open tall trapezoid with two fixed arrow shafts and V-shaped fletching |
| `Slot.Head` / Head | Dome outline above short brim, no face detail |
| `Slot.Torso` / Torso | T-shaped tunic outline |
| `Slot.Legs` / Legs | Trouser outline, central split into two legs |
| `Slot.Feet` / Feet | Side-view boot polygon with broad sole |
| `Slot.Accessory` / Accessory | Diamond pendant below angular neck cord; one slot only |
| `Skill.Generic` / Skill | Square frame with three ascending rungs; rank is separate text |
| `Skill.Melee` / Melee | Crossed short blade and baton, only for supplied skill definitions |
| `Skill.Ranged` / Ranged | Segmented bow arc, string and arrow; not a separate equipment slot |
| `Spell.Generic` / Spell | Four-point spark within circle; no promise a spell is learned |
| `Spell.Light` / Light | Small circle with four rays |
| `Spell.FireDart` / Fire Dart | Angular flame/dart pointing right with two trailing strokes |
| `Spell.HealLight` / Heal Light | Open palms around short upward ray; avoid a medical cross emblem |
| `Spell.StoneShard` / Stone Shard | Unequal triangular rock facets |
| `Spell.DustDevil` / Dust Devil | Three offset horizontal tapered loops/segments |
| `Resource.Health` / Health (HP) | Endurance shield outline + external HP |
| `Resource.Mana` / Mana (MP) | Diamond outline + external MP |
| `Resource.XP` / XP | Three ascending bars + external XP; debt separately labelled |
| `Resource.Gold` / Gold | Two offset coin circles; numeric balance outside |
| `Resource.AttributePoints` / Attribute points | Plus in square; separate pool name |
| `Resource.SkillPoints` / Skill points | Plus in circle; separate pool name |
| `Action.Details`, `.Back`, `.Plus`, `.Minus` | Information circle; left arrow; plus; minus, each with action label |
| `Action.Target`, `.Interact`, `.Save` | Corner brackets; speech rectangle/tail; tray + downward arrow |
| `State.Selected`, `.Success` | Two-segment check, disambiguated by words |
| `State.Warning`, `.Error` | Triangle/!; octagon/!, with words |
| `State.Unavailable`, `.Unknown` | Slashed circle; circle/?, with words |
| `State.Pending`, `.Disputed`, `.Prototype` | Hourglass; divided diamond; outlined word badge |
| `State.New`, `.InProgress`, `.Ready`, `.Completed` | Small diamond + New; half-filled square + In progress; check in open box + Ready; double check + Completed |

Only instantiate candidate spell/skill icons when definitions exist. Their names are inherited slice candidates, not starter grants or an invented catalog. The general placeholder covers all other supplied definitions. Combat effects are not inferred from icon art.

### Input glyph policy

**Use generic labelled geometry, no platform trademarks.** This is V-03's explicit first-pass glyph policy under the task brief, replacing V-02's illustrative `[A]/[B]/[X]/[Y]` artwork without changing semantic bindings. The four-position diamond diagram uses four small circles with the relevant position filled; pair it with the full label **South — Confirm**, **East — Back**, **West — Filter/Secondary**, **North — Details**. Shapes retain an Edge outline and Primary active position; no coloured button lettering, branded silhouettes or logos.

| Input family | Generic construction and label |
|---|---|
| Face buttons | Four-circle position diagram + South/East/West/North + current action |
| D-pad | Four orthogonal arrows + D-pad — Navigate |
| Sticks / stick press | Circle with axis cross; center dot for press; Left stick / Right stick / Left-stick press / Right-stick press |
| Shoulders / triggers | Rounded rectangle + external L shoulder/R shoulder or L trigger/R trigger, followed by action |
| Journal / pause | Plain two-rectangle symbol + Journal button; three horizontal lines + Menu button; action still explicit |
| Keyboard / mouse | Neutral keycap with current key name; simple two-button mouse diagram with active side + readable button name |
| Chord / toggle | Separate labelled glyphs joined by `+`; words Hold or Toggle from current preference; never imply release casts |

Glyph box stays 36/24 square, label follows by 12/8; long labels expand the hint group, never squeeze letters into the icon. Footer text remains Metadata 24/16; selectable Controls-sheet rows use Control 30/20. At most four groups with 36/24 gaps. If full generic labels overflow, keep Confirm, Back and context Tabs visible, route other hints into the focusable Controls sheet per V-02; measure actual text width instead of assuming four will fit. Follow current remapped bindings and input context. Device changes retain logical focus; disconnect requests pause and displays words. No branded glyph assets are required on Linux.

## C++ widget handoff

Suggested table/struct shape **by names only**; these names do not claim existing declarations. Keep values in one local style source W2-03 can mirror. Shared headers, saved schemas, content enums and presentation registries remain integrator-owned and unchanged by this task.

| Container | Proposed member names |
|---|---|
| `FLHUIStyleTokens` | `StyleId`, `Revision`, `Colors`, `Typography`, `Geometry`, `WidgetRecipes`, `Icons`, `InputGlyphPolicy` |
| `Colors` | All exact `Color.*` suffixes in the colour table |
| `Typography` | `FontFamily`, `FallbackFamily`, `Title`, `Section`, `Body`, `Control`, `Numeric`, `Metadata`; each: `SizeAt1080`, `LineHeightAt1080`, `Weight`, `TabularFigures` |
| `Geometry` | `DesignResolution`, `MinimumResolution`, `Safe`, `Grid`, `Header`, `Body`, `Status`, `Footer`, `TwoPane`, `ThreePane`, `Modal`, `Unit`, `ControlInset`, `PanelInset`, `TargetMin`, `ControlTwoLine`, `RowGap`, `CornerRadius`, `EdgeStroke`, `FocusStroke`, `FocusGap`, `Icon`, `IconLabelGap`, `HintGroupGap`, `BarHeight`, `BarLabelGap` |
| `WidgetRecipes` | `Button`, `ListRow`, `Tab`, `Slider`, `TextField`, `StatStepper`, `EquipmentSlot`, `Tooltip`; each: `Normal`, `Hover`, `Focused`, `Pressed`, `Disabled`, `Error`, `Selected`, `Pending` |
| Recipe layers | `FillToken`, `TextToken`, `BorderToken`, `InsetBorderToken`, `OuterRingToken`, `LeadingMarker`, `StatusMarker`, `StatusLabel`, `PaddingToken`, `MinSizeToken` |
| `Icons` | `PresentationId`, `PrimitiveKind`, `DesignBounds`, `StrokeToken`, `ForegroundToken`, `AccessibleLabel`, `FallbackId` |
| `InputGlyphPolicy` | `SemanticAction`, `DeviceFamily`, `BindingLabel`, `PositionDiagram`, `ContextLabel`, `HoldToggleLabel` |

Treat foreground/background colours as sRGB source values and convert once into the widget renderer's expected colour space; do not feed encoded byte fractions into a linear-colour field and assume identical output. Avoid parent tint multiplication and default disabled darkening that changes contrast. Use shared brush/primitive recipes rather than hand-colouring each button. Reserve separate layers for focus, selection, errors and content. Visibility, focusability and activation availability are distinct: a rejected/unknown action remains inspectable even if its commit handler is gated.

Bind state to presenter/session snapshots, never widget guesses. The style consumes availability, preview/pending state, provenance and save durability; it does not compute requirements, XP thresholds or quiver eligibility. Full field errors require presenter data beyond `FLHCommandResult::Reason`; use V-02 generic copy if none is supplied. Keep focus anchored by stable IDs and restore it after modal close; Cancel/Stay remains default for destructive/permanent choices. Use the existing Linux in-game keyboard design for names (ten-column alphabet grid, explicit Done/Cancel), styled with these same field/button recipes.

### Screen-to-token map

Every screen uses the common set: Background/Panel/Raised, TextPrimary/Secondary/Disabled, Edge/HoverEdge, Accent/FocusRing, SelectedMark, Warning/Error/Success/Info/UnknownMark; the full type scale, safe grid and input-hint metrics. The table identifies the additional emphasis and widgets, **not permission to alter V-02 layout or commands**.

| V-02 screen | Specific tokens / recipes | Preserve while styling |
|---|---|---|
| [Frontend](../frontend.md) | TwoPane; Title; Button, Tooltip; Info pending, Warning unavailable, Error storage, Success durability | Opaque save card over future original art; five actions; Continue remains inspectable |
| [Character select](../character-select.md) | TwoPane, Modal; ListRow, Button; Warning recovery, Error invalid save, Success ready/durable status | Earlier-save acknowledgment, distinct stable selection/focus, safe Delete confirmation |
| [Character creation](../character-creation.md) | ThreePane, Modal; TextField, ListRow, Button, Tooltip; attribute icons, Error field, Accent Prototype, SelectedMark answers | Four questions/five answers, five full stat names, cosmetic preview, unavailable unresolved Roll/Confirm |
| [HUD](../hud.md) | BarTrack/Health/Mana; resource icons; EquipmentSlot-like quick cells, Button/Tooltip for context; Info/Success/Error save frame | No XP/stamina HUD bar; six slots; clear zone; no shared full-width Status strip; labels on opaque bands |
| [Character sheet](../character-sheet.md) | Wide-left layout; Numeric; StatStepper, Tab, Button, Tooltip; BarXP only with supplied progress, BarHealth/Mana | Separate attribute/skill pools, Base/Gear/Effects/Current/Pending, separate XP debt; permanent-confirm modal |
| [Inventory / equipment](../inventory-equipment.md) | ThreePane; ListRow, EquipmentSlot, Tab, Button, Tooltip; slot icons, Success Equipped, UnknownMark requirements | All eight slots including Quiver; Selected distinct from Equipped; Unlimited only from resolved definition |
| [Spell selection](../spell-selection.md) | TwoPane; ListRow, Tab, Button, Tooltip; spell icons, BarMana if used, UnknownMark, Warning unavailable | Six assignment cells; learned state/cast MP separate from training cost; focusable full details |
| [Services](../services.md) | TwoPane, Modal; Numeric; ListRow, Tab, StatStepper, Button; resource point/gold icons, Error quote, Success receipt | Separate gold and skill costs; fresh quote before confirmation; no widget price arithmetic |
| [NPC dialogue](../npc-dialogue.md) | TwoPane; Body transcript, Section speaker; ListRow, Button, Tooltip; New/InProgress/Ready/Completed markers | Readable transcript/choices, no timed advance, state words, full reward review in modal |
| [Pause / settings](../pause-settings.md) | TwoPane, Modal; Slider, StatStepper, Tab, Button, TextField-like binding capture; Info pending, Error conflict/save | Value and On/Off words, visible Cancel, fixed Apply/Revert, safe unsaved-exit review |
| [Death / respawn](../death-respawn.md) | Background, Panel consequence card, Modal; Body, Button; Info settling, Success checkpoint, Error save | Static readable loss/destination, unknown consequences remain unknown, Return waits for durability |

### Integration review still to run

W2-03 applies the style to the four active screens first, then character selection/recovery and common modal/keyboard/settings components; later owners use the remaining mappings. On Linux at 1080p and 720p, inspect actual font metrics and opaque surfaces under bright/dark scenes; traverse without a pointer; combine focus with disabled, error, selected and pending; inspect Quiver and long requirements. Exercise enlarged text, name IME/Unicode, overflow, every modal Back path and device disconnect. Inspect resource labels at empty/full/unknown and reduced-motion status. Check rendered colours after tint/gamma/disabled handling. Windows validation is deferred. Static specimens do not establish controller parity, font cook inclusion, performance or gameplay correctness.

## Images actually opened and evidence limits

On **2026-10-08**, V-03 opened the four images below individually at original detail, read-only. `P` = `/home/brewerm/Downloads/lighthaven-unreal-design-and-agent-waves/lighthaven-plan`. No reference pixels, fonts or icon packs are copied into the public repository.

| Path beneath P | Observed material useful to this guide |
|---|---|
| `assets/references/characters/character-creation-reference.png` | Purple portal, floating dice, adventurers and creature; **no interface**. Mood only, not creation layout/RNG evidence. |
| `assets/concepts/starter-characters-and-creatures-concept.png` | Modest linen/leather humans, subdued brown creatures and distinct green slime; broad material/value separation. |
| `assets/concepts/temple-and-basement-concept.png` | Rough masonry/timber, pale walls, amber torches with cool shadows; informs restrained UI accents. |
| `assets/concepts/deep-dungeon-creatures-concept.png` | Dark broad creature masses, red wing/goblin accents and distinct silhouettes; supports shape labels and avoiding colour-only identity. |

Reference/concept rights remain as recorded in the art register; first-pass direction approval does not make them distributable. V-03 also read **all eleven V-02 `.md`/`.svg` screen pairs** and visually opened a 1280×720 rasterization of each SVG: frontend, character-select, character-creation, character-sheet, inventory-equipment, hud, spell-selection, services, npc-dialogue, pause-settings, death-respawn. Those temporary renders are design inspection, not Unreal captures. Existing SVG illustrations sometimes compress state details; implement the paired Markdown's full behavior and the metrics here.

Static validation and resulting revision are recorded in the attempt's handoff report. Open decisions are font asset/version adoption, presenter/state availability, and actual Linux widget readability/focus verification. This package does not assert those checks have run.

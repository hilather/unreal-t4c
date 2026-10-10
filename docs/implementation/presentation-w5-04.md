# W5-04 presentation

Task contract revision 1; base `aa03c765c4b231a62b36eb0a3434a428992b5f5c`. This is a scoped native presentation implementation and evidence candidate, not a gate approval.

Read against V-02 `ui/README.md`, `ui/pause-settings.md`, V-03 `ui/style/README.md`, V-04 `ui/feedback/README.md`, and the Wave 5 row in `docs/plan/docs/05-agent-waves.md`.

## Behavior

- HUD bars interpolate published HP/MP fractions; exact numeric labels always reflect authority immediately. Unknown/max-zero resources hide their bars. Resource changes create labelled notices, not inferred healing/damage events.
- The target frame is an opaque nameplate with a `> Selected:` mark and authoritative target name/HP. `No target` explicitly marks an empty selection. This implements HUD target highlighting; no world outline/material, collision or particle is added.
- Combat components in the current play world are weakly subscribed from the Slate HUD. Committed actions involving the local player's stable instance ID admit resolved impact events. Damage/miss text appears only on OnImpact, never on command acceptance. Recent identities are bounded to 32 for duplicate suppression. Player death and earned-level increases produce explicit text notices. No cue awards XP, changes a resource, submits a command, draws RNG or writes a save.
- Settings are reachable through existing frontend/pause navigation: Master, Effects, UI volume (10% steps), Text scale (100–150%, 10% steps), High contrast, Apply, Revert, Back. Left/right adjusts, Confirm increments/toggles; existing keyboard and gamepad focus/navigation continue to work. Values preview immediately in the session; Apply persists, Revert reloads persisted values. Back retains the current session preview (an explicit simplified assumption compared with V-02's draft/discard modal).
- Preferences use section `[Lighthaven.Presentation]` in `LighthavenPresentation.ini` under Unreal's generated user-config directory (separate from character saves and the engine's cached GameUserSettings). No character snapshot fields, schema or save codec changes. Loaded and saved values are clamped and nonfinite values replaced with defaults.
- Audio IDs `UI.Click` and `Combat.Impact` feed an optional runtime sink with master × category gain; zero gain suppresses delivery. The default sink is silent. Engine inspection found WhiteNoise/Voice and editor audio, but no suitable runtime click/impact sound was identified. No guessed asset reference or editor-only sound is shipped. A future approved sound adapter installs this sink; category volume is already applied there.
- A non-consuming Slate input preprocessor observes key/gamepad input and mouse clicks. Stick movement crosses a dead zone before switching devices. Menu/HUD hints show only the most recent device using bracketed keyboard keycaps or neutral controller position labels (South/East/shoulder/trigger), without branded logos or downloaded glyph fonts. Existing bindings remain authoritative; this does not add remapping.
- HUD text/cues/footer have opaque backgrounds. Primary `#F2F2F2` and secondary `#C4C4C4` on `#242424` meet 4.5:1 in linear luminance calculations. High contrast uses white on black. Base body/control text is 30 at 1080p (20 at 720p), metadata 24 (16). Text enlargement never decreases those minima and menu content scrolls. Words, numbers and the selection chevron supplement colour.

## Content and provisional presentation tuning

No downloads or binary assets. Brushes and default regular font come from Unreal `FCoreStyle` (`WhiteBrush`, default font, engine-installed Slate resources). No project asset package is introduced. Native text widgets provide effects.

Authored prototype presentation values only: 0.5-second full-range bar interpolation (speed 2 fractions/s), 1.5-second hit/resource notice, 3-second death/level notice, 32 recent impact identities, volume/text steps, and a 0.25 analogue device dead zone. None are historical T4C mechanics or damage/award tuning.

## Validation and remaining limits

Validation results and exact commands are recorded in the attempt report. Automation names are `Lighthaven.UI.Presentation.SettingsPersistAndApply`, `GlyphSwitching`, `HitAndResourceChange`, `CombatAwardGuard`, `SettingsKeyboardAndPad`, `ContrastAndMinimumText`.

The guard uses one valid synthetic character snapshot copied twice, resolves identical combat input and runs real `LHRewards::SettleKill` with cues enabled/disabled. Canonical encoded settlement bytes must match, including XP, loot, RNG and growth awards. It does not replace packaged gameplay validation.

No physical controller, audible mix, rendered screenshots, world-space nameplates, profiling or packaged launch is claimed. World target highlight remains the HUD nameplate. Component discovery currently iterates live combat components during HUD ticks; this needs profiling and an owner-provided registration seam if actor counts grow. Actions committed before the first HUD subscription can have no impact cue. The bounded cosmetic duplicate cache is not persistent receipt protection. Restoration of a higher-level snapshot while retaining this HUD can produce a cosmetic level notice; it cannot replay awards. These are integration limitations, not new authority paths. Visual direction's disconnect/rebind/reduced-motion sheets and full world-marker pooling remain outside this minimal presentation pass; gamepad hands-on coverage stays open.

Observed Linux validation on 2026-10-10: corrected editor and game builds both reported `Result: Succeeded` (64.55 s / 73.25 s). Headless `Lighthaven.UI` completed 22/22 tests, including all six presentation tests; process exit 0. An earlier run was 21/22: settings persistence failed and was corrected with direct dedicated-user-config read/write, then rerun. Test command uses the Wave 4 null-RHI form plus the documented `InstalledNoZenLocalFallback` DDC override. Baseline LFS-pointer maps generated unloadable-package log errors; no map/editor/play gate is claimed. Build cache-store warnings did not prevent successful targets.

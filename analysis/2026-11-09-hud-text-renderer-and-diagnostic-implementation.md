# 2026-11-09 — HUD text renderer validation and diagnostic implementation

Target launcher source: `LiteLDev/LeviLaunchroid`, tag `v1.5.13`.

## Verified launcher renderer behavior

Inspected:
- `app/src/main/java/org/levimc/launcher/core/mods/inbuilt/overlay/HudOverlay.java`
- `app/src/main/java/org/levimc/launcher/core/mods/inbuilt/ExternalModBridge.java`
- `app/src/main/java/org/levimc/launcher/core/mods/inbuilt/overlay/ModConfigView.java`

Confirmed:
- HUD commands are fetched from Preloader by `ExternalModBridge.getDrawCommands()` and redrawn in `HudOverlay.onDraw()`.
- `DrawCommandType::Text` maps to `TYPE_TEXT = 0`.
- Text uses Android Canvas coordinates in pixels, with `cmd.y` as the text baseline.
- `w == -2.0f` selects centered text alignment.
- Coordinates `x <= -19000` resolve relative to screen center; exactly `-20000` resolves to horizontal center. Same mapping applies to Y.
- Text size is passed to Paint.setTextSize.
- The module HUD overlay repeatedly invalidates itself using Choreographer while visible.
- `ConfigType::Text` is an EditText, confirming it is not a read-only status display. `ConfigType::Button` is a clickable button.

## Implemented diagnostic

Source change in `src/main/cpp/native-lib.cpp`:
- When the 3D ESP module is toggled on, submit one centered white HUD text command:
  `3D ESP: HUD OK | entity scan inactive`
- When toggled off, clear the module's draw commands.
- If a future validated frame producer submits its own frame, preserve the same diagnostic text in that batch so it is not overwritten.

The label is deliberately explicit: this tests whether the native-to-launcher HUD text path is visible. It does not claim that entities are enumerated or that 3D boxes are working.

## Safety and correctness boundaries

- No game memory is dereferenced by this diagnostic.
- No hook or guessed offset was enabled.
- No entity enumeration, live AABB, or camera matrix access was added.
- The diagnostic depends on the module being enabled and the launcher's HUD overlay being visible.

## Build status

The source commit is `4db9487820b69dcdca2bd333adcae37eddedc5c2`. GitHub Actions run 120 was created for this commit. Its final result must be checked separately; repository commit success is not a build result.

## Device test after artifact is available

1. Install the new `.levipack` through LeviLauncher.
2. Enter a world and enable **3D ESP**.
3. Check whether the centered text `3D ESP: HUD OK | entity scan inactive` appears.

Interpretation:
- Text visible: the module toggle and HUD text submission path work; the remaining problem is live game data/render integration.
- Text absent: investigate module toggle callback, registration state, HUD visibility, and artifact identity. Do not proceed by enabling unverified game hooks.

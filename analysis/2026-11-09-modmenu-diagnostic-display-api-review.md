# 2026-11-09 — LeviLauncher Mod Menu diagnostic-display API review

Target: LeviLauncher 1.5.13 / pinned Preloader commit `8c19383f417744d3165165de9afca42b60059174`.

## Source inspected

Pinned SDK header:
`include/pl/ModMenu.hpp`

The public `ConfigType` enum contains:
- `Toggle`
- `SliderInt`
- `SliderFloat`
- `Radio`
- `Color`
- `Keybind`
- `Text`
- `Button`

The public `ModuleInfo` exposes static `description`, a configuration list, `onToggle`, `onConfigChanged`, and `onKeybind`. `ModuleBuilder` exposes `description(...)` and `config(...)`, but the inspected public header has no API to mutate a registered module's description or a config entry's current value after registration.

The same API exposes `submitDrawCommands(moduleId, span)` for HUD primitives, including Text and Line. This is the existing drawing bridge used by the project.

## Engineering decision

Do not add a fake read-only status row by registering `ConfigType::Text`: the inspected API models configuration entries and provides no supported setter for updating their displayed value. A Text config may be an editable user setting, not a status label. Likewise, do not assume `ConfigType::Button` is a status display.

A separate HUD diagnostic string could be submitted through `submitDrawCommands`, but before implementing it the exact launcher-side text placement, coordinate units, clipping, and interaction with the per-frame command replacement must be verified. The module's existing draw submission replaces the module's whole command list, so a one-off status command could be erased by a later frame submission unless deliberately composed into the same frame batch.

## Current reliable conclusion

- Menu registration and toggle are already confirmed on the user's device.
- Static description can communicate that runtime entity/camera integration is pending, but it is not a live status field.
- The public SDK header alone does not provide a supported dynamic status-text setter for the Mod Menu settings panel.
- No game hook, guessed offset, or new entity dereference is justified by this API review.

## Next work

Inspect the exact LeviLaunchroid v1.5.13 renderer implementation for `DrawCommandType::Text` and screen coordinate conversion. If the renderer semantics are confirmed, implement an explicitly labelled diagnostic HUD message and preserve it in every composed draw batch. This would test the HUD bridge independently; it would not be represented as ESP entity rendering.

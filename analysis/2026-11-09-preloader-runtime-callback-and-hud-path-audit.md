# 2026-11-09 — Preloader runtime callback and HUD path audit

Target remains Minecraft Bedrock Android 1.26.33, ARM64, LeviLauncher 1.5.13.

## Sources

- LeviLaunchroid v1.5.13 pins Preloader Android commit `8c19383f417744d3165165de9afca42b60059174`.
- Pinned headers inspected: `include/pl/Mod.hpp`, `include/pl/ModMenu.hpp`, `include/pl/memory/Hook.hpp`, `include/pl/memory/Signature.hpp`.
- Preloader runtime inspected at current public commit `cc08b40a31471d3e2dedb267f5f17b28da2b0604`: `src/pl/PreLoader.cpp`, `src/pl/runtime/GameHooks.cpp`, `src/pl/runtime/GameHooks.h`, `src/pl/runtime/GameHookRules.cpp`.
- Official LeviLaunchroid Mod Menu API documentation: `https://levilaunchroid.levimc.org/api/mod-menu`.

## Confirmed lifecycle shape

The pinned `pl::mod::ModRegistration` exposes only four lifecycle dispatches: `load`, `enable`, `disable`, and `unload`. There is no general per-frame tick callback in this registration table.

## Confirmed HUD drawing path

The Mod Menu API provides `submitDrawCommands(moduleId, commands)` for replacing a module's current HUD overlay commands. Supported primitives include lines, rectangles, text, images and other simple UI shapes.

This is sufficient as a destination for already-computed 2D screen-space line segments. It is not itself a source of camera matrices, world positions, entity lists, or AABBs, and it does not document a per-frame callback that supplies those values.

## What Preloader's own game hooks do

The inspected `GameHooks.cpp` installs version-rule-driven hooks for pause-menu and HUD-screen lifecycle state, plus an `isShowingMenu` query. These hooks track whether menus/HUD screens are open. They are not entity enumeration or world-render callbacks.

The runtime rules loader expects signatures for:
- pause menu destructor/open
- HUD screen destructor/open
- isShowingMenu (or a vtable index)

No actor iterator, entity component access, camera matrix provider, or render-frame callback is present in these interfaces.

## Engineering consequences

1. Keep the current Mod Menu registration and 2D line-command bridge; it is a valid rendering destination.
2. Do not treat `submitDrawCommands` as a frame callback or as a way to discover entities.
3. The remaining integration requires two separately verified inputs: a safe repeating update point and a version-matched source of live entity positions/bounds plus camera projection.
4. The TAA candidate remains disabled. The inspected Preloader APIs do not validate its ABI or make it a suitable entity/render callback.
5. No guessed offsets, no borrowed 1.26.60 actor signatures, and no patching of the game's original shared library.

## Status

This audit narrows the supported launcher API boundary and rules out the assumption that the generic mod lifecycle supplies a tick callback. It does not produce a working in-game ESP and does not justify publishing another diagnostic build.

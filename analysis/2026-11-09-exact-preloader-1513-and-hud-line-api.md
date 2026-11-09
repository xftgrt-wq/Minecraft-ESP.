# Exact LeviLauncher 1.5.13 Preloader pin and HUD line API

Date: 2026-11-09

## Exact version match

The official `LiteLDev/LeviLaunchroid` source tag `v1.5.13` contains a git submodule at:

`app/src/main/cpp/preloader`

Its gitlink points to the exact Preloader commit:

`8c19383f417744d3165165de9afca42b60059174`

This is stronger evidence than selecting a recent SDK tag: the project now pins the precise Preloader source revision recorded by the user's stated LeviLauncher version.

Source:
- https://github.com/LiteLDev/LeviLaunchroid/tree/v1.5.13
- https://github.com/LiteLDev/preloader-android/tree/8c19383f417744d3165165de9afca42b60059174

## Verified SDK APIs at that commit

Public headers include:
- `pl/Mod.hpp`: `PL_REGISTER_MOD`, `NativeMod::current()`, and load/enable/disable/unload lifecycle.
- `pl/ModMenu.hpp`: `ModuleBuilder`, toggle/config callbacks, and `submitDrawCommands`.
- `pl/memory/Hook.hpp`: supported Preloader hook API and RAII `HookHandle`.
- `pl/memory/Signature.hpp`: module signature resolution API.
- `pl/Input.hpp`: touch and key callbacks.

The Mod Menu API defines `DrawCommandType::Line` with geometry fields `x`, `y`, `w`, `h`, and `size`.

## Renderer coordinate confirmation

The matching LeviLaunchroid `v1.5.13` source file `HudOverlay.java` renders line commands as:

- start: `(x, y)`
- end: `(x + w, y + h)`
- stroke width: `size`

This validates the native bridge mapping from each projected segment:
- `x = x1`
- `y = y1`
- `w = x2 - x1`
- `h = y2 - y1`

The Android HUD overlay is driven by a Choreographer frame callback and redraws the current submitted commands. This is a supported HUD drawing surface; it is not itself a game-world entity/render callback.

## Project changes

- Replaced the placeholder JNI load logger with a Preloader lifecycle registration.
- Added a Mod Menu module named `3D ESP`.
- Added separate Players and Dropped items toggles.
- Added a single-frame line submission interface that combines red player segments and green item segments in one command batch.
- Added a `preload-native` manifest.
- Updated CMake to fetch the exact pinned Preloader commit and link the mod against `preloader`.
- Updated Android CI to use NDK `28.2.13676358`, matching the launcher source tag, and `c++_shared`.
- Updated CI packaging to produce a standalone `.so` and a `.levipack`.

## Explicitly unresolved

The new API bridge is an integration scaffold. No game callback currently calls `esp_overlay_submit_frame()`. The project still needs a verified render-stage callback, safe access to the TAA matrix and origin, and live player/item AABB enumeration. No guessed offsets or hooks have been added.

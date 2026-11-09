# Minecraft-ESP (prototype)

Target supplied by the project owner:
- Minecraft Bedrock Android 1.26.33
- LeviLauncher 1.5.13
- ABI: arm64-v8a
- Intended overlay: red 3D outlines for players and green outlines for dropped items; no distance labels.

## Binary analysis of the supplied game library

The supplied `libminecraftpe.so` was inspected locally on 2026-11-09.

Verified facts:
- ELF64, little-endian, AArch64 shared object.
- File size: 349,243,744 bytes.
- Android build number string found: `47475273`, matching the supplied Bedrock 26.33 Android build.
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`.
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.
- The required Bedrock entity, item, camera, and world-render APIs were not exposed by recognizable exported names; private-code analysis is required.

## Confirmed render-camera lead

RTTI and relocation analysis ties the routine at VA `0x1290c4a8` to the `dragon::framerenderer::modules::taau` TAA upscaling render-graph callback. Its second callback argument is strongly identified as `TAAContext`.

Within that callback:
- `TAAContext + 0x208` supplies the 64-byte `CurrentViewProjectionMatrixUniform` payload.
- `TAAContext + 0x248` supplies the 64-byte `PreviousViewProjectionMatrixUniform` payload.
- `TAAContext + 0x288..0x293` supplies the 12-byte `CurrentWorldOrigin` payload.
- The previous world origin is assembled partly from another live pointer, so its full owner/lifetime is not yet known.

The generic uniform-store helper independently copies 64 bytes from each matrix payload. This confirms payload size and semantic names, but not matrix element order, coordinate convention, or whether the context can be read safely outside the TAA callback.

Detailed notes:
- `analysis/2026-11-09-taa-callback-rtti-and-context-identification.md`
- `analysis/2026-11-09-taau-context-field-map.md`
- `analysis/2026-11-09-taa-uniform-store-helper-size.md`
- `analysis/2026-11-09-taa-world-origin-source-fields.md`

## LeviLauncher 1.5.13 Preloader integration

The repository pins the exact Preloader submodule commit embedded in the official LeviLaunchroid `v1.5.13` source tag:

- Launcher source tag: `LiteLDev/LeviLaunchroid v1.5.13`
- Embedded Preloader commit: `8c19383f417744d3165165de9afca42b60059174`
- Native SDK: `LiteLDev/preloader-android`
- Public API used: `PL_REGISTER_MOD`, native mod lifecycle, and `pl::modmenu::ModuleBuilder`
- HUD API: `pl::modmenu::submitDrawCommands` with `DrawCommandType::Line`
- Verified build toolchain: Android NDK r28b (28.2.13676358), ABI `arm64-v8a`, Android API 28, `c++_shared`.

The native module has a lifecycle registration and a Mod Menu entry named **3D ESP**, with separate Players and Dropped items switches. A frame submission interface combines red player segments and green dropped-item segments into one HUD draw-command batch. LeviLauncher's Android HUD overlay redraws submitted commands each frame.

The line command mapping was checked against the matching `v1.5.13` launcher source: the renderer draws from `(x, y)` to `(x + w, y + h)`, so projected segment endpoints map directly to the SDK line command.

**Important:** this is an integration scaffold, not yet an active ESP. No world/entity callback currently calls the frame submission API, and no live entity AABBs are collected. The menu switch and draw bridge compile, but in-game registration and drawing have not yet been verified on the target device.

## Geometry module

The native geometry module provides:
- projection of eight AABB corners using an explicitly assumed column-major OpenGL-style 4x4 matrix;
- the 12 canonical AABB edges;
- edge projection and clipping against the homogeneous camera plane and all six OpenGL frustum planes;
- finite-input validation and screen-space segment output.

Tests cover identity projection, camera-plane crossings, side/far frustum clipping, off-screen boxes, invalid bounds, null pointers, and non-finite inputs. The host CI job compiles these tests with AddressSanitizer and UndefinedBehaviorSanitizer; they pass.

## Android ARM64 build — verified

GitHub Actions run [42674589492](https://github.com/xftgrt-wq/Minecraft-ESP./actions/runs/42674589492) completed successfully on commit `cd8d15bdbff29effd99207cd28097bdbee27b242`.

All workflow steps passed:
- host geometry tests;
- Android SDK setup;
- pinned Android NDK r28b setup;
- CMake configuration for `arm64-v8a`;
- native library compilation;
- ELF architecture/dependency verification;
- LeviPack packaging and artifact upload.

The uploaded artifact is `minecraft-esp-arm64-prototype` (artifact ID `13700875076`). It contains:
- `build/libminecraft_esp.so` — verified ELF64 AArch64 shared library, Android API 28, NDK r28b; SHA-256 `5c20c9b5ed5e154c274138f173a5bb025ee4e9bf695ac6d24628d0329c1d0ffa`.
- `dist/minecraft_esp.levipack` — ZIP integrity verified; SHA-256 `67eccaa0ce9616e90f62dcf39384565d499d3f1aab6b54c58ca4d77ce4f5fe29`.

This verifies compilation and packaging only. It does not establish compatibility with the live game process or prove that the ESP works.

## Current status — not a working ESP

This repository is still a native build prototype, **not a working in-game ESP**. No player/item enumeration, live entity AABB retrieval, supported render hook, actual line submission, through-wall rendering, or verified in-game toggle behavior is implemented. The Preloader lifecycle and HUD bridge compile, but runtime entity and camera integration remains outstanding.

A working implementation still requires verified runtime access to:
1. the current world and entity/item collections;
2. entity positions and bounding boxes;
3. the correct world-to-camera transform, including world-origin rebasing;
4. a supported and correctly timed render/HUD submission point;
5. safe cleanup when the world or renderer changes.

Do not replace the game's original `libminecraftpe.so` with this prototype. Do not hardcode TAAContext-relative fields as global offsets. The signature resolver diagnostic is not a uniqueness proof and no hook is installed. A wrong hook or stale context pointer can crash the game.

## Next engineering steps

1. Runtime-check Preloader module loading and the signature diagnostic on the exact target game/launcher.
2. Identify a supported render callback and prove its timing and lifetime.
3. Validate matrix layout and world-origin rebasing against live game coordinates.
4. Recover entity/item iteration and AABB access without copying offsets from other Bedrock versions.
5. Connect validated AABBs to the geometry module and submit drawing only after the callback and coordinate space are proven.

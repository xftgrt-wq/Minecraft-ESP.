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

**Important:** this is an integration scaffold, not yet an active ESP. No world/entity callback currently calls the frame submission API, and no live entity AABBs are collected. The menu switch and draw bridge compile. On 2026-11-09, the project owner confirmed on the target device that the Mod Menu entry appears, the ESP toggle can be enabled and stays enabled, but no 3D boxes appear. This confirms menu/toggle operation only; it does not confirm entity collection or frame submission.

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

This repository is still a native build prototype, **not a working in-game ESP**. The target-device test confirms that the module loads, the Mod Menu entry appears, and the toggle stays enabled; the user reports that no boxes are visible. No player/item enumeration, live entity AABB retrieval, supported render callback, or actual per-frame line submission is implemented. Through-wall rendering is also unverified. Runtime entity and camera integration remains outstanding.

A working implementation still requires verified runtime access to:
1. the current world and entity/item collections;
2. entity positions and bounding boxes;
3. the correct world-to-camera transform, including world-origin rebasing;
4. a supported and correctly timed render/HUD submission point;
5. safe cleanup when the world or renderer changes.

Do not replace the game's original `libminecraftpe.so` with this prototype. Do not hardcode TAAContext-relative fields as global offsets. The device-side independent scan confirms one complete signature match in executable mappings, but this does not validate the callback ABI, argument lifetime, matrix convention, or render timing. No hook is installed. A wrong hook or stale context pointer can crash the game.

## Next engineering steps

1. Device-verified: the native module loads, the Mod Menu entry registers, and the complete TAA signature has exactly one match in executable mappings in the live process (see `analysis/2026-11-09-device-signature-validation-and-hook-safety.md`).
2. Identify a supported render callback and prove its timing and lifetime.
3. Validate matrix layout and world-origin rebasing against live game coordinates.
4. Recover entity/item iteration and AABB access without copying offsets from other Bedrock versions.
5. Connect validated AABBs to the geometry module and submit drawing only after the callback and coordinate space are proven.


## 2026-11-10 static-analysis progress — AABB filtered view

A new exact-binary trace identifies the FNV-1a component key for `AABBShapeComponent` as `0xbac1b3cf`. At `0x9e7ad60`, Bedrock resolves that storage through a strict-context-like object alongside SubBBs, synchronized actor data, actor ownership, actor-removal, and local-player filter storages. The caller at `0x9e7f0e0` builds and iterates a paged, generation-checked filtered view. A nearby lookup for `CameraAimAssistResetEntityExcludedStatusFlagComponent` ties this code path more closely to the CameraAimAssist system.

Detailed trace: [AABBShape key and CameraAimAssist view trace](analysis/2026-11-10-aabbshape-key-and-camera-aimassist-view-trace.md).

This is a static-analysis milestone only. The AABB component value layout, world/local coordinate semantics, complete player/item coverage, private context lifetime, callback ABI, and render-thread handoff remain unproven. No game hook was enabled and no live entity data is fed into the overlay. The current build is still a HUD prototype, not a working ESP.


### Follow-up: filtered-view iteration boundary

A follow-up disassembly of the CameraAimAssist-oriented view builder and its caller shows that it chooses the smallest of six participating component storages, validates packed entity keys against the remaining storages, forms component-slot addresses, and dispatches a per-entity visitor callback. The AABB storage is part of this filtered view, but the 0x20-byte slot stride does not reveal its field layout. The view is system-filtered and cannot yet be treated as a complete player/item feed.

Detailed report: [AABB filtered-view iteration and callback boundary](analysis/2026-11-10-aabb-filtered-view-iteration-callback-boundary.md).

Next target is the visitor callback implementation and its AABB field reads. No runtime hook was added; the module remains HUD-only and live ESP is still not implemented.


### Visitor callback resolved

The filtered-view visitor's concrete callable target is now identified at `0x9e4379c`. It validates multiple component references, reads a scalar from one component, processes 0x18-byte vector-like records with min/max and squared-distance calculations, and emits compact 0x34-byte candidate records. This confirms the path is a CameraAimAssist spatial candidate-selection pipeline, not a straightforward all-entity AABB enumerator. The exact component layouts and world-space semantics are still unknown.

Detailed report: [CameraAimAssist visitor callback spatial trace](analysis/2026-11-10-camera-aimassist-visitor-callback-spatial-trace.md).


### Bounds layout clue: SubBBs min/max vectors

The CameraAimAssist visitor's 0x18-byte records are read as six consecutive float32 values (two float3 vectors). SIMD comparisons clamp a reference point between these vectors, calculate squared distance to the box, and derive its midpoint. This is strong evidence that the records represent min/max bounds. Their coordinate space and exact producer are still unresolved, so they are not yet safe to pass as world-space ESP AABBs.

Detailed report: [SubBBs six-float bounds and closest-point calculation](analysis/2026-11-10-subbb-six-float-bounds-closest-point.md).


### Hitbox helper coordinate-source follow-up

The Hitbox helper callsite at `0x9e3cdd8` passes a three-float vector loaded from the current object's `+0xe8/+0xf0` fields, a pointer into the current HitboxComponent entry, and two zeroed output vectors. The helper at `0xb58ecd0` returns an entity key that the caller validates against paged storage. In its fallback branch it adds two 3D vectors from the Hitbox entry to two corresponding context-provided vectors, then writes the resulting pair to the outputs. This is consistent with bounds plus a position/transform contribution, but the virtual methods' semantics and world-space guarantee are not yet proven.

Detailed report: [Hitbox helper coordinate-source trace](analysis/2026-11-10-hitbox-helper-coordinate-source-trace.md).

No runtime hook was added; live ESP remains unimplemented.


### Hitbox helper input-vector provenance

A follow-up trace shows that the three-float input passed to Hitbox helper `0xb58ecd0` is read from an entity-keyed storage slot returned by the paged lookup helper `0x10d67694`. The caller obtains the storage owner through a virtual call at `x25` vtable byte offset `+0x368`, then uses its `+0x48` subobject. The vector is read at storage-object offsets `+0xe8/+0xf0`. This establishes per-entity provenance, but not the concrete component type or whether the values are world-space coordinates.

Detailed report: [Hitbox helper input-vector provenance](analysis/2026-11-10-hitbox-helper-input-vector-provenance.md).

No runtime hook was added; live ESP remains unimplemented.


### Hitbox helper lookup correction

A re-read of the callee at `0x10d67694` shows bucket-count calculation and bucket-chain collision handling consistent with a hash-map lookup, not a paged ECS-storage lookup. The caller supplies an entity key and uses the mapped value's `+0xe8/+0xf0` fields as the Hitbox helper's three-float input. The concrete map value type and coordinate semantics remain unknown.

Detailed correction: [Hitbox helper hash-map lookup](analysis/2026-11-10-hitbox-helper-hash-map-lookup-correction.md).


### Critical correction: Hitbox helper lookup key

A deeper disassembly of `0x10d67694` shows string-like key handling (small-string/heap representation, length comparison, byte comparison and hash-bucket collision traversal). The caller's `x22+0x50` argument must therefore not be called an entity handle. Earlier notes that described this as an entity-keyed or generation-aware lookup were incorrect. The mapped value's `+0xe8/+0xf0` fields remain unidentified and cannot yet be called world coordinates.

Detailed correction: [Hitbox helper string-key lookup](analysis/2026-11-10-hitbox-helper-string-key-correction.md).

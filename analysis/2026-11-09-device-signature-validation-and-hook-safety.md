# Device validation checkpoint — 2026-11-09

Target reported by the project owner:
- Minecraft Bedrock Android 1.26.33
- LeviLauncher 1.5.13
- ARM64 / arm64-v8a

## Observed device Logcat

The user installed the second diagnostic package and supplied screenshots from the live game process. The mod emitted:

- `Minecraft ESP native module loaded (render hooks pending)`
- `TAA signature resolver candidate: 0x77e732a4a8; independent executable-map matches: 1; first match: 0x77e732a4a8`
- `TAA signature is unique in executable mappings; hook remains disabled pending ABI/lifetime validation`
- `3D ESP toggle registered in LeviLauncher Mod Menu`

## What this proves

1. The native module is loaded in the Minecraft process.
2. The mod's lifecycle reached the enable/diagnostic path.
3. The Mod Menu module was registered.
4. Preloader resolved the signature to `0x77e732a4a8`.
5. The independent read-only scan of executable mappings named `libminecraftpe.so` found exactly one complete 40-byte match, at the same address.

The runtime result is consistent with the statically identified function VA/file offset `0x1290c4a8`, but the current code does not independently parse ELF load segments to prove the module-relative address. The runtime unique match validates the signature's uniqueness in the executable mappings at the time of the scan; it does not prove callback ABI, semantic identity at runtime, thread, timing, or argument lifetime.

## Hook API review

The exact Preloader 1.5.13-pinned `pl/memory/Hook.hpp` exposes a generic detour API:

- `hook(target, detour, originalFunc, priority)`
- `unhook(target, detour)`
- `HookHandle` RAII wrapper

This API accepts raw function pointers and does not encode or validate the target's calling convention, argument types, ownership, or callback lifetime. The RAII wrapper's destructor requests unhooking, but its interface alone does not establish that no detour invocation is in flight during unload. Therefore, it cannot make an incorrectly typed or stale callback safe.

## Remaining hard blockers

- Prove the exact callable ABI and semantic argument types from the callback registration/invocation path, not only from the callee's register saves.
- Establish whether the third argument's TAAContext remains valid for the full callback and whether any data can be copied synchronously without retaining pointers.
- Determine the actual matrix element order and clip-space convention from the matrix producer/consumer path.
- Establish the current-world-origin convention and convert entity world coordinates into the same camera-relative space.
- Recover a version-exact ECS entity iteration path and component layouts for the target binary. Existing type names and system adapter signatures are not enough to safely dereference components.
- Identify the correct viewport dimensions and render/HUD submission timing.

## Decision

Do not install the TAA detour yet. The unique signature is now device-verified, but ABI and lifetime are not. No entity data reaches `esp_overlay_submit_frame()`, so no in-game ESP boxes can appear. Continue static tracing and host-side tests; require a purpose-built diagnostic build and fresh device logs before any runtime callback is enabled.

Never replace the original game's `libminecraftpe.so`.

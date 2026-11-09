# 2026-11-09 — Preloader SDK hook/API boundary check

Target: Minecraft Bedrock Android 1.26.33 (build 47475273), LeviLaunchroid/LeviLauncher v1.5.13, ARM64.

## Sources checked

- LeviLaunchroid tag `v1.5.13` pins `LiteLDev/preloader-android` commit `8c19383f417744d3165165de9afca42b60059174`.
- The current public `preloader-android` main branch was last pushed 2026-11-05; its latest commit is `cc08b40a31471d3e2dedb267f5f17b28da2b0604` (`feat: optimize pattern scanning`).
- Compared public headers `include/pl/memory/Hook.hpp`, `include/pl/memory/Signature.hpp`, and `include/pl/ModMenu.hpp` at the pinned commit and current main.

## Confirmed API boundary

At the pinned v1.5.13 SDK commit:

- `pl::memory::hook(target, detour, originalFunc, priority)` installs a function detour; `unhook` removes it. `HookHandle` is an RAII wrapper.
- `pl::memory::resolveSignature(signature, moduleName)` returns a resolved address; `resolveSignatures` resolves a batch.
- Mod Menu supports primitive HUD draw commands including `Line`, `Rect`, and `Text`; `submitDrawCommands` replaces the module's current command list.
- The headers do not expose a Minecraft world/entity collection, actor iterator, player/item query, entity AABB accessor, or camera matrix provider.

## Comparison with current SDK main

The public current main branch retains the same declarations for hook/unhook and signature resolution. The latest visible commit is a pattern-scanning optimization; it does not add a Bedrock entity or camera API. The project should continue pinning the v1.5.13 submodule commit for reproducible launcher compatibility unless a deliberate SDK upgrade is made and rebuilt.

## Engineering decision

No hook was installed and no game binary was patched. A signature match alone is not enough to infer the function ABI, call frequency, thread, argument lifetime, or a safe forwarding path. The TAA candidate is also a fullscreen post-process callback, not yet a proven entity/render callback.

The existing ESP geometry and HUD bridge can remain as-is. The missing dependency is still a version-matched, evidence-backed source of live entity positions/AABBs plus a safe frame callback. Do not fabricate offsets or reuse offsets from another Bedrock build.

## Result

This check confirms the Preloader integration surface and rules out the assumption that Preloader itself supplies entity enumeration. It does not advance the runtime ESP to a working state and does not justify a new device build.

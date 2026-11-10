# Runtime integration audit — pinned LeviLauncher 1.5.13 SDK

Date: 2026-11-10
Pinned Preloader commit: `8c19383f417744d3165165de9afca42b60059174`

## Result

The pinned SDK exposes no typed Bedrock world/entity iteration API and no general world-render callback for a mod to subscribe to. The available public APIs do not provide the missing live player/item AABBs or a camera matrix. Therefore this audit does **not** justify installing a new hook or enabling the TAA callback candidate.

## Files inspected

- `include/pl/Mod.hpp`: mod lifecycle only (load, enable, disable, unload), JavaVM and mod filesystem/config context.
- `include/pl/Input.hpp`: Android touch, keyboard, text, mouse and document callbacks; no game-world events.
- `include/pl/ModMenu.hpp`: module/config registration and HUD primitive submission; no game render callback or entity access.
- `include/pl/memory/Hook.hpp`: generic detour API. It requires the caller to provide a valid target and exact ABI; it does not discover safe Bedrock hooks.
- `include/pl/memory/Signature.hpp`: byte-signature resolution only.
- `include/pl/memory/Vtable.hpp`: RTTI-backed vtable slot resolution only.
- `src/pl/runtime/GameHooks.cpp`: Preloader's internal hooks are limited to pause-menu/HUD-screen lifecycle and menu-state tracking. The configured signatures are loaded from runtime rules; these are not a public per-frame world-render callback.

The complete pinned public include directory contains only Config, Export, Input, Logger, Mod, ModMenu, ModMenuConfig, and the memory Hook/Patch/Signature/Vtable headers. No Actor, Entity, Level, World, ECS, Renderer, or RenderEvent public header is present.

## Consequence for this project

Already working:
- native module lifecycle and Mod Menu toggle;
- HUD line/rectangle command submission;
- AABB projection and frustum clipping;
- AABB-to-HUD bridge function compiled in build #135;
- input validation and command-cap handling compiled in build #136.

Still missing:
1. a verified source of current player and dropped-item AABBs;
2. a verified camera view-projection matrix with the correct matrix convention and world-origin rebasing;
3. a callback whose ABI, thread, frame timing, and context lifetime are proven.

The TAA callback at file offset `0x1290c4a8` remains a static-analysis lead only. The signature's uniqueness does not establish its callback ABI or context lifetime. The previously tested ClientInstance slot 31 and LivePlayerRenderer slot 17 candidates resolved to zero on-device and remain rejected.

## Next safe engineering path

Continue exact-version analysis of Bedrock's own private interfaces, using the supplied binary's verified Build ID and checking every candidate against the live process. A candidate is not eligible for hooking until its call sites establish the exact signature/ABI, render-thread timing, ownership/lifetime of all referenced objects, and a safe unhook strategy. Until then, keep all game hooks disabled and do not claim live ESP functionality.

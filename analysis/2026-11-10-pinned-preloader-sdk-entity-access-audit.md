# Pinned Preloader SDK entity-access audit — 2026-11-10

Target SDK: LiteLDev/preloader-android commit `8c19383f417744d3165165de9afca42b60059174` (the revision pinned by the LeviLauncher 1.5.13 project).

## Result

The pinned public Preloader SDK does **not** expose a supported Minecraft entity/ECS access API.

The complete public header tree contains lifecycle/mod context, Mod Menu/HUD, input, logging, and low-level memory helpers. In particular:

- `include/pl/Mod.hpp` exposes mod lifecycle, JavaVM, metadata, logger, and mod-owned directories only.
- `include/pl/ModMenu.hpp` exposes module configuration and HUD draw-command submission only.
- `include/pl/memory/Vtable.hpp` exposes RTTI-backed primary-vtable slot resolution; it does not expose object discovery, ownership, or entity iteration.
- `include/pl/memory/Hook.hpp` exposes generic detouring, but does not validate a target ABI, argument lifetime, or render-thread safety.
- The runtime source initializes the mod manager, game hooks, and Mod Menu bridge. No public entity registry, ECS view, Actor/Player accessor, component accessor, or render-frame callback is declared in this pinned SDK.

## Engineering consequence

There is no supported SDK call that can simply be added to enumerate players, mobs, or dropped items. Guessing internal pointers, vtable slots, component offsets, or callback signatures would risk invalid reads and game crashes.

The existing project geometry module and HUD bridge remain useful isolated components, but no live game AABBs are currently passed into `esp_overlay_submit_frame()`. Therefore the visible HUD diagnostic is not an ESP box implementation.

## Next viable paths without another user upload

1. Continue source-level inspection of the pinned SDK and launcher bridge; do not claim it provides entity access when it does not.
2. Keep the unvalidated TAA callback disabled. A unique byte-pattern match is not proof of a callable ABI or safe context lifetime.
3. If a game-side entity API is to be used, it must be identified from exact-build evidence and validated independently. The original game library must never be replaced by a loader stub.
4. Until exact-build evidence is available, work on host-side geometry tests and HUD rendering only, and label those as isolated components rather than a functioning ESP.

## Confirmed current state

- Build #125 HUD diagnostic visibly rendered on the user's device.
- Mod Menu toggle and native library loading were previously observed.
- Entity enumeration and live AABB retrieval are not implemented.
- Camera matrix source and callback ABI/lifetime are not fully validated.
- No live 3D boxes are implemented or verified.

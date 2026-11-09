# BedrockTools compatibility check for the uploaded game library

Date: 2026-11-09

## Why this check was made

The previous static analysis did not identify verified Bedrock engine hooks. I checked a public LeviLauncher Android native mod project, BedrockTools, because it publishes version-specific ARM64 signatures, offsets, actor wrappers, and a UI render callback.

## What was found

- BedrockTools documents a supported Android native-mod path using LeviLaunchroid's Preloader SDK.
- Its v1.5.12 manifest declares Minecraft compatibility as `1.26.6X.X`.
- Earlier inspected releases declare `1.26.60.X`, `1.26.5X.X`, and `1.26.50.4`; none of the inspected manifests declares the user's uploaded build as `1.26.33`.
- The project contains useful reference structures and concepts: actor state/bounds wrappers, LevelRendererPlayer camera-position storage, actor-manager/signature definitions, and a UI render-pass callback. These values are version-specific and cannot be assumed valid on another build.
- I tested nine representative ARM64 signatures from BedrockTools v1.5.12 against the uploaded library's `.text` section: ActorManagerList, ActorFetchNearbyActorsSorted, ActorRenderDispatcherRender, RenderLevel, ActorIsPlayer, SetupFogPlayer, TessellatorColor, TessellatorVertex, and MeshHelpersRenderMeshImmediately. All nine produced zero matches.
- This is consistent with a build mismatch. It is not proof that the functions do not exist; it means these particular signatures are not validated for this file.

## Consequence

Do not copy the v1.5.12 offsets or install its hooks into the uploaded `libminecraftpe.so`. A false match or stale object offset could crash the game or corrupt memory.

The official Preloader route is preferable to replacing the game's original shared library. However, a loadable Preloader mod shell alone would not provide the requested 3D ESP. A working implementation still needs signatures and data layouts verified against this exact build.

## Current status

- Exact uploaded library: ELF64 AArch64, Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.
- No verified player/item iteration hook.
- No verified world-render callback for this build.
- No verified camera view-projection matrix source.
- No functional ESP library produced.

## Next required evidence

A matching signature/offset set for this exact 1.26.33 build (or a supported Minecraft build whose signatures match) is required before implementing player/item outlines. Until then, keep the current prototype separate from the game library and do not install it as an ESP.

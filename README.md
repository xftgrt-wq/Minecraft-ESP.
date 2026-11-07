# Minecraft-ESP (prototype)

Target supplied by the project owner:
- Minecraft Bedrock Android 1.26.33
- LeviLauncher 1.5.13
- ABI: arm64-v8a
- Intended overlay: red 3D outlines for players and green outlines for dropped items; no distance labels.

## Current status

This repository contains a native Android library bootstrap and an ARM64 GitHub Actions build workflow. It is **not a working ESP mod** and must not be installed as one.

The supplied `libminecraftpe.so` was inspected:
- ELF64, little-endian, AArch64 shared object
- stripped, with a dynamic symbol table
- the initial symbol scan did not establish verified Minecraft entity, item, camera, or render interfaces

A working ESP requires verified runtime access to entity/item lists, world positions, camera matrices, and a supported render/UI integration point for this exact game build. Those interfaces and offsets have not been established. Guessing offsets or injecting unverified hooks could crash the game.

## Build

The GitHub Actions workflow builds an ARM64 prototype library. Open **Actions**, inspect the **Android ARM64 build** workflow, and download its artifact only if the run succeeds.

## Next engineering step

Identify a supported LeviLauncher mod/plugin API or obtain matching public headers/debug symbols for this exact Bedrock build. Only after the entity, item, camera, and render interfaces are verified should ESP rendering be implemented.

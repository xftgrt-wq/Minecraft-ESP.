# Minecraft-ESP (prototype)

Target supplied by the project owner:
- Minecraft Bedrock Android 1.26.33
- LeviLauncher 1.5.13
- ABI: arm64-v8a
- Intended overlay: red 3D outlines for players and green outlines for dropped items; no distance labels.

## Binary analysis of the supplied game library

The supplied `libminecraftpe.so` was inspected locally on 2026-11-07.

Verified facts:
- ELF64, little-endian, AArch64 shared object.
- File size: 349,243,744 bytes.
- Android build number string found: `47475273`, matching the published Android arm64-v8a build for Bedrock 26.33.
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`.
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.
- The ELF contains 89,746 dynamic symbol entries, but the defined exported C++ symbols inspected so far are primarily third-party libraries (V8, WebRTC, LevelDB, cohtml, Xbox services). The required Bedrock entity, item, camera, and world-render APIs were not exposed by recognizable exported names.
- The library is stripped of full debug information; a matching signature database or disassembly-based analysis is required before any game hook can be considered safe.

## Current status

This repository is a native Android build prototype only. It is **not a working ESP mod**. No player/item enumeration, camera projection, through-wall outline rendering, or distance-independent ESP is implemented.

A working ESP requires verified runtime access to:
1. the current world and entity/item collections;
2. entity positions and bounding boxes;
3. the active camera view/projection matrices;
4. a supported render or HUD submission point.

Do not guess offsets or install unverified hooks: an incorrect ARM64 hook can crash the game or corrupt memory.

## Build

The GitHub Actions workflow builds an ARM64 native prototype. Open **Actions**, inspect the **Android ARM64 build** workflow, and download its artifact only if the run succeeds.

## Next engineering step

Port a known public BedrockTools signature set to this exact build and validate each signature against the supplied binary. Any candidate must be unique and its function behavior/calling convention must be confirmed before it is used. Then implement the ESP using the documented LeviLauncher Preloader SDK and its supported HUD drawing API.

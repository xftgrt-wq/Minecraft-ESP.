# 2026-11-09 — Exact-version RTTI leads and safe runtime diagnostics

Target: Minecraft Bedrock Android 1.26.33.1 / ARM64, LeviLauncher 1.5.13.

## New exact-version public lead

A public project, ChimeraAnt-DEV/GlowberryClient, contains a static-analysis report for an ARM64 libminecraftpe.so labelled Minecraft 1.26.33.1:

- docs/binary-analysis-1.26.33.1.md
- Reported binary SHA-256: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1
- Reported APK package: com.mojang.mine26331, described as a third-party clone APK from mcpedl.org.

The report claims the following static candidates:
- ClientInstance primary-vtable slot 31: local-player accessor.
- LivePlayerRenderer primary-vtable slot 17: render entry.
- Actor + 0x230 position and Actor + 0x238 rotation are explicitly marked unverified on 1.26.33.1.
- The C++ argument ABI for the render slot is explicitly unverified.
- The report does not establish runtime hook behavior, camera matrix availability, or a complete entity list.

## Provenance / confidence caveat

This is a useful lead, not a trusted offset pack. The analysed APK is identified as a third-party clone rather than the user's known official game installation. The .so is not committed, and its SHA-256 has not been compared with the user's exact libminecraftpe.so. Therefore none of its slot or field claims is promoted to a production hook or a live entity feed.

## SDK capability confirmed

The exact Preloader SDK commit pinned by LeviLauncher 1.5.13 exports:

pl::memory::resolveVtableFunction(typeInfoName, slot, moduleName)

The resolver searches the loaded module's .rodata and .data.rel.ro sections for the RTTI name and primary-vtable slot. This is present in the pinned SDK, so RTTI-based diagnostics do not require upgrading the launcher SDK.

## Code change

The ESP module now performs read-only diagnostics for:
- resolveVtableFunction("14ClientInstance", 31, "libminecraftpe.so")
- resolveVtableFunction("18LivePlayerRenderer", 17, "libminecraftpe.so")

It checks whether each returned address lies in an executable mapping belonging to libminecraftpe.so, then logs the results. It does not dereference any returned pointer, install either hook, or read any Actor fields.

## What a successful diagnostic would and would not prove

A nonzero address in an executable game mapping would confirm that the RTTI name and slot resolve in the user's running binary. It would not independently prove:
- the slot's semantic role on the user's exact binary;
- the callback ABI and argument layout;
- pointer lifetime or safe hook forwarding;
- the local Actor field offsets;
- a world/entity iterator or dropped-item source;
- a suitable camera projection matrix.

The TAA candidate remains diagnostic-only and disabled as a hook.

## Next evidence gate

Before enabling any hook, compare the exact user's game-library SHA-256 with a trusted official 1.26.33.1 binary, inspect the resolved slot's target and call sites, and validate the ABI and lifetime on-device. Entity enumeration and item/player AABBs remain a separate missing source.

No production offsets were copied from 1.26.50/1.26.60, and the original game library is never patched.
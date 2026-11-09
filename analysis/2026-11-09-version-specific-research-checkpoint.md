# 2026-11-09 — Version-specific research checkpoint

## Target identity

The target described for this project is Minecraft Bedrock 1.26.33 on Android ARM64 with LeviLauncher 1.5.13. Public version information identifies Bedrock 26.33 Android ARM64 build 47475273 (version code 972603301) and internal Android version 1.26.33.1.

## Compatibility finding

The public LeviLauncher Android documentation describes ARM64 support and Minecraft 1.21.80 or later, but this does not establish that a particular native-mod SDK/signature set supports every later game build. The previously inspected BedrockTools releases targeted 1.26.50/1.26.60 families, not 1.26.33. Their offsets and signatures must not be copied into this target.

A separate public LeviLauncher mod project reports a target of 1.26.52.3 and explicitly pins its Preloader SDK revision to its launcher version. That is useful evidence that the Preloader ABI itself is version-sensitive; it is not a compatible 1.26.33 implementation.

## Current blocker

The original uploaded `libminecraftpe.so` is not present in the active analysis workspace at this checkpoint, so a fresh disassembly or signature scan cannot be performed now. Prior notes record that strings for ECS bounds/state components and a view-projection uniform were found, but their inspected references were metadata initializers, not verified runtime accessors.

## What is and is not established

- Confirmed from prior binary inspection: names for actor state/bounds components and a view-projection uniform occur in the target library.
- Not established: component storage offsets, actor iteration API, live camera matrix address, renderer callback, or unique 1.26.33 function signatures.
- Not safe to claim: a functioning ESP or a ready-to-install `.so`.

## Next required input

Re-upload the exact `libminecraftpe.so` extracted from the target Minecraft 1.26.33 ARM64 installation. With that binary available, continue tracing the ECS registration helpers into their callers and inspect the runtime actor/component storage path. Keep all offsets and signatures version-specific and verify matches before any native hook is attempted.

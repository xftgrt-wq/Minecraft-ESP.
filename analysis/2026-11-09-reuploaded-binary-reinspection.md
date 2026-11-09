# 2026-11-09 — Reinspection of the re-uploaded target binary

## Input verified

The re-uploaded `/mnt/data/libminecraftpe.so` was read successfully.

- Format: ELF64, little-endian, AArch64 shared object
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`

This hash matches the binary examined in the earlier investigation, so this is the same target build rather than a new variant.

## Fresh disassembly confirmation

The strings `AABBShapeComponent` and `StateVectorComponent` are referenced in the code around VA `0x0c600048–0x0c600148`. Disassembly shows ADRP/ADD instructions loading the component-name strings, building a stack-local descriptor, then passing it to a helper at `0x873d46c`. This sequence is a component/type registration initializer; it does not enumerate actors or read live component instances.

The `CurrentViewProjectionMatrixUniform` string is referenced around VA `0x1290cf28–0x1290cf44`. The surrounding code uses `__cxa_guard_acquire`, constructs a local descriptor, calls a metadata helper, and releases the guard. This is a lazy static metadata initializer, not a live GPU matrix getter.

## Result

The re-upload confirms the earlier conclusion. The target binary contains the relevant ECS and rendering type names, but the inspected references do not reveal usable runtime component offsets, actor iteration, or a live view-projection matrix. No safe, version-verified hook address has been established.

Do not treat these metadata initializer addresses as ESP hooks. A working build still requires tracing the actual runtime ECS storage/access path and the renderer's matrix upload path for this exact binary.

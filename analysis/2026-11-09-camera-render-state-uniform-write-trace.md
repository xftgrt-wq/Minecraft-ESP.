# LevelRendererCamera uniform-update trace — 2026-11-09

Target: Minecraft Bedrock Android ARM64, uploaded `libminecraftpe.so`.

## Verified binary identity

- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- ELF64, AArch64
- Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New disassembly observations

The function at VA `0xbd6f97c` is present in four relocation-backed virtual-function tables:

- `0x141d15b8`
- `0x141d1970`
- `0x141d1c18`
- `0x141d1f38`

The first table's surrounding relocation records show a contiguous run of function entries beginning at `0x141d14f8`; the candidate is at offset `+0xc0` from that address point. The same function appears in three further related tables. This supports its use as a shared virtual method across related table views, but does not alone prove the concrete C++ class name or all runtime call paths.

The function saves its four incoming arguments in callee-saved registers:
- x20 = incoming x0
- x22 = incoming x1
- x19 = incoming x2
- x26 = incoming x3

The method reads camera/object fields including:
- `[x20 + 0x424]`, `[x20 + 0x428]`, `[x20 + 0x42c]`, `[x20 + 0x430]` (four scalar float reads)
- `[x20 + 0x5a4]`, `[x20 + 0x5a8]`
- `[x20 + 0x980]` (pointer to a nested structure)
- `[x19 + 0x8a8]` (integer converted to float)
- `[x19 + 0x74]`, `[x19 + 0x810]`

It then writes scalar/vector values through nested uniform-like objects. Examples include writes to buffers reached through `[x21 + 0x20]` and `[x21 + 0x18]`, and updates to a larger state object at `x22 + 0xd30` through `x22 + 0xd70`, plus further blocks at `x22 + 0xda0` and `x22 + 0xe10`.

A helper call at `0xbd9c4d0` is made with a temporary structure and values derived from the renderer inputs; its output is then copied into the larger state object. This is consistent with camera/render-state preparation.

## What this does and does not establish

Established:
- The method performs per-frame graphics/render-state work.
- It writes several scalar and vector uniform-like values.
- It calls the previously investigated matrix/vector helper at `0xbd9c4d0`.
- The same method address is installed in four relocation-backed function tables.

Not established:
- No direct 4x4 ViewProjection matrix load from the camera object was identified in the inspected instruction ranges.
- The uniform slot names corresponding to these writes are not recovered.
- The exact runtime owner types and call sites are not yet fully resolved.
- No live ECS entity iteration, component field offsets, or usable hook point is verified by this finding.

## Next investigation

Trace the helper `0xbd9c4d0` inputs/outputs and the state blocks written at `x22 + 0xd30`, `+0xda0`, and `+0xe10`; separately trace the descriptor-registration path for `CurrentViewProjectionMatrixUniform`. Do not treat any of these offsets as a ready-made ESP interface.

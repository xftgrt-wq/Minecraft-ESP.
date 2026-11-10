# 2026-11-10 — Exact-binary follow-up: CameraAimAssist RTTI function-pointer cluster

## Binary identity

Reopened the already supplied `/mnt/data/files/libminecraftpe.so` in the current analysis environment; no re-upload was needed.

- ELF64, little-endian, AArch64
- Size: 349,243,744 bytes
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New disassembly check

Re-read the relocation-backed region around the previously identified CameraAimAssist RTTI metadata at `0x140605a0–0x14060608`.

The nearby relocation records include code pointers:
- `0x875e6f4`
- `0x9e80b18`
- `0x9e80b1c`
- `0x9e80b4c`
- `0x9e80bc8`
- `0x9e80bd8`
- `0x9e80bdc`
- `0x9e80c0c`

Direct disassembly shows:
- `0x875e6f4` is a single `ret` instruction; a distinct helper begins at `0x875e6f8`.
- `0x9e80b18` is also a single `ret`; a distinct helper begins at `0x9e80b1c`.
- `0x9e80b1c` dispatches through a process-global object at `0x1473fa48` using virtual slot `+0x18`.
- `0x9e80b4c` allocates/initializes a small object through that same global object's virtual interface (requests size 16), then installs a vtable pointer at `0x140605d8`.
- `0x9e80bc8` writes that same vtable pointer into an output object.
- `0x9e80c0c` is a trivial false-return helper.

This is evidence that the RTTI-adjacent function-pointer region includes small object lifecycle/dispatch helpers, but it does **not** identify the global object as the ECS registry or prove that these helpers construct the CameraAimAssist strict-system adapter itself. The global at `0x1473fa48` remains a generic allocator/dispatch path, not an entity accessor.

## Additional nearby code

Two other pointers in the adjacent relocation cluster, `0x8568ed8` and `0x9e7e954`, begin structurally similar routines. Both load two pointers from an object at offsets `+0x8` and `+0x10`, then reference the same global dispatch object. Their exact owning C++ types are not established by this pass. They are not promoted to runtime hook candidates.

## Decision

No constructor/factory-to-world-context chain has been proven. No new runtime offsets, entity accessor, component layout, or hook ABI is justified. Keep all game hooks disabled.

The useful correction is that the adjacent code pointers must be treated as small generated lifecycle/dispatch helpers, not assumed to be the gameplay registry or a ready-made entity iterator. Next investigation should resolve the owning RTTI/vtable record for the object whose vtable is installed at `0x140605d8`, and trace its allocation callsites back to the strict-system registration owner.

Live player/item ESP remains unimplemented; HUD and AABB projection remain separate, already-tested pieces.

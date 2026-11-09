# TAA upscaling world-origin source fields

Date: 2026-11-09

Binary examined: the uploaded ARM64 Bedrock `libminecraftpe.so` (Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`).

## Descriptor identities

The TAA upscaling uniform registration sequence in the routine at VA `0x1290c4a8` uses these descriptor addresses:

- `0x149100f0`: `CurrentWorldOrigin`
- `0x14910100`: `PreviousWorldOrigin`
- `0x149100a0`: `CurrentViewProjectionMatrixUniform`
- `0x14910090`: `PreviousViewProjectionMatrixUniform`

## Source fields observed at the registration call sites

For `CurrentWorldOrigin`, the code builds a 12-byte temporary value from:
- 8 bytes loaded from `[x21 + 0x288]`;
- 4 bytes loaded from `[x21 + 0x290]`.

The temporary is passed to the uniform-store helper with descriptor `0x149100f0`.

For `PreviousWorldOrigin`, the code builds a 12-byte temporary value from:
- 8 bytes loaded from `[x22 + 0xa8]`;
- 4 bytes loaded from `[x21 + 0x29c]`.

The temporary is passed to the uniform-store helper with descriptor `0x14910100`.

The adjacent current/previous view-projection matrix descriptors are fed from `[x21 + 0x208]` and `[x21 + 0x248]`, respectively.

## Interpretation and limits

The 12-byte payload size and the uniform names are consistent with three-component world-origin values. This is useful because camera-relative rendering commonly combines a view-projection transform with current/previous world-origin offsets.

However:
- `x21` and `x22` are live function registers, not established global object bases;
- the owning C++ type and stable field offsets have not been recovered;
- the coordinate convention, units, origin rebasing policy, and relationship to entity AABBs are not yet proven;
- the source does not establish that these fields can be safely read from an injected overlay callback;
- this is a TAA upscaling pass path, not proof that every world-render path uses the same values.

Do not transplant `+0x208`, `+0x248`, `+0x288`, `+0x290`, or `+0x29c` as process-wide offsets. They are only offsets from the unidentified live context pointer used by this one routine.

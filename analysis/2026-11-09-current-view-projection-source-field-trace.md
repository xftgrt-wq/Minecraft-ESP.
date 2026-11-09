# CurrentViewProjectionMatrixUniform source-field trace — 2026-11-09

Target binary SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## Key finding

The descriptor initializer at `0x1290cf18–0x1290cf44` registers the name string `CurrentViewProjectionMatrixUniform` (rodata VA `0x21e2297`) into the descriptor object at BSS VA `0x149100a0`.

A separate code path at `0x1290c700–0x1290c714` materializes that exact descriptor address and passes it to the generic 64-byte data-store helper at `0x1267ab74`:

- x0 = `[x21 + 0x60] + 0x8` (destination/store context)
- x1 = `0x149100a0` (the CurrentViewProjection descriptor)
- x2 = `x21 + 0x208` (source data)

The helper at `0x1267ab74`:
1. reads a 64-byte block from x2,
2. searches a descriptor-keyed entry using the descriptor pointer in x1,
3. copies the 64-byte block into the matching entry,
4. marks the entry dirty/updated.

This directly ties the named CurrentViewProjection descriptor to a 64-byte source block at offset `+0x208` from the object held in x21 at this call site.

## Adjacent descriptor

Immediately before it, another descriptor at `0x14910090` is passed to the same helper with source `x21 + 0x248`. This is a separate 64-byte matrix-like field and must not be conflated with CurrentViewProjection.

## Why this is stronger than the earlier matrix-upload lead

The earlier `glUniformMatrix4fv` cluster was not sufficiently tied to the world camera and was provisionally misclassified based on a nearest-symbol label. ELF disassembly labels can span large regions when local function symbols are absent; the nearest exported JNI symbol is not reliable evidence that a distant address belongs to that JNI function. The present result instead follows the exact named descriptor address into a data-store helper and identifies its 64-byte source.

## Confidence and limits

High confidence:
- descriptor name `CurrentViewProjectionMatrixUniform` is registered at `0x149100a0`
- code at `0x1290c704–0x1290c714` passes that exact descriptor pointer to the data-store helper
- the source block is `x21 + 0x208`
- helper copies exactly 64 bytes

Not yet proven:
- the concrete C++ type of x21 / the object containing offset `+0x208`
- whether the matrix is already world-to-clip or needs a separate transform/transpose convention
- lifetime and thread-safety of the source object
- a safe runtime hook point
- ECS registry iteration and component offsets

## Next steps

Identify the owner/type of x21 from its constructor and surrounding call chain; trace how `x21+0x208` is populated; compare with the adjacent `+0x248` matrix; then independently resolve ECS entity iteration and bounding-box component storage. Do not hardcode `+0x208` without validating the object identity and exact game build at runtime.

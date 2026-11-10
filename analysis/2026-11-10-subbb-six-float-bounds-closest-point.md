# 2026-11-10 — SubBBs six-float bounds and closest-point calculation

## Exact target

Uploaded Android ARM64 `libminecraftpe.so`:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## New finding: 0x18-byte records contain two 3D vectors

Inside the visitor callback at `0x9e4379c`, the code traverses records with a 0x18-byte stride. For each record it reads:
- 64 bits at offset `+0x00` (two floats),
- one float at `+0x08`,
- 64 bits at `+0x0c` (two floats),
- one float at `+0x14`.

Together these are six consecutive float32 values at offsets 0, 4, 8, 12, 16, and 20: two 3-component vectors.

The SIMD instructions then compare a separate 3D reference vector against the two vectors:
- `fcmgt` compares reference coordinates against the second vector;
- `fmaxnm` clamps the reference coordinates against the first vector;
- `bsl` selects the clamped values;
- squared differences are accumulated and compared against a squared radius/threshold.

This is the characteristic closest-point-to-AABB distance calculation. It establishes that each 0x18-byte record is interpreted as a pair of 3D min/max vectors for a box-like spatial region. The loop also computes the midpoint of the two vectors for a candidate point.

## Relation to components

The callback is reached from the view that resolves both AABBShapeComponent and SubBBsComponent. The 0x18-byte record stride matches the previously observed SubBBsComponent storage stride, and its six-float access pattern is consistent with SubBBs entries holding min/max bounds.

The callback also reads a float at offset `+0x08` from another resolved component slot and uses it as part of the squared-distance threshold. The exact component identity and semantic unit for that scalar are not yet proven, so it is not named here.

## Coordinate-space limitation

The reference vector is passed through the callback argument bundle. Its exact producer and coordinate space are not yet recovered. The min/max pair could be local to an actor, relative to a transform, or already transformed; the current instruction window alone does not settle that distinction.

Therefore, these six floats must not yet be fed directly into `EspAabb` as world-space coordinates. The next step is to trace the reference-vector producer and the HitboxComponent bounds helper, then establish whether the bounds are translated/rotated and how their origin is applied.

## What is established

1. The 0x18-byte records are consumed as two float3 vectors.
2. The callback uses those vectors as min/max bounds for a closest-point / squared-distance calculation.
3. It derives a midpoint candidate from the same min/max pair.
4. This is a real bounds-related data path associated with the AABBShapeComponent/SubBBs filtered view.

## What remains unproven

- The exact C++ member names and component ownership of the six floats.
- Whether the vectors are local-space or world-space.
- Whether this CameraAimAssist path includes all player and dropped-item entities.
- Whether the callback is safe to reuse outside its original strict-system lifecycle.
- The camera matrix and render-thread handoff.

## Next target

Trace the callback's reference-vector argument back to its producer and compare it with the two-vector output of HitboxComponent helper `0xb58ecd0`. Do not add a runtime hook until ABI, owner lifetime, and coordinate space are proven.

## Runtime status

No hook or game-memory dereference was added. The installed module remains HUD-only and still reports `WAITING FOR GAME DATA`; live ESP boxes are not yet implemented.

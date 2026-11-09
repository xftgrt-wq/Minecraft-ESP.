# Verification checkpoint — geometry tests and camera-xref status — 2026-11-09

## Geometry module tests

The committed AABB projection source was compiled locally with g++ using C++17, -Wall, -Wextra, -Werror, AddressSanitizer, and UndefinedBehaviorSanitizer. The test executable completed successfully.

Cases checked:
- identity matrix projects all eight AABB corners;
- viewport mapping sends NDC corners to expected pixel coordinates;
- non-positive clip-space W is rejected;
- inverted AABB bounds are rejected;
- null pointers and non-positive viewport dimensions are rejected.

These tests validate the standalone math module only. They do not validate the actual Minecraft camera matrix convention or in-game rendering.

## Camera descriptor xref

The re-uploaded target binary has the same SHA-256 as before: d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1.

The .eh_frame FDE covers 0x1290c4a8..0x1290d560. The code saves incoming x2 as x21, then reads several fields from that object, including offsets 0x60, 0x1e8, 0x1ea, 0x1f8, 0x208, 0x248, and later fields. The 0x208 and 0x248 blocks are each passed to descriptor-keyed 64-byte store helpers.

The FDE establishes a function range, not a source-level function name. The nearest-symbol annotation printed by objdump is not a reliable owner label at this address. The pattern of viewport-like halfwords, scale/size floats, and multiple uniform blocks is compatible with a structured UI/render-state object; this is a caution against treating +0x208 as the world camera matrix. It is not a definitive type identification.

Current status: the world ViewProjection source remains unverified. No target-specific matrix offset, ECS iterator, component field offset, or hook point is approved for use.

## Repository changes

- Added tests/esp_geometry_test.cpp.
- Updated .github/workflows/android-arm64.yml so host geometry tests must pass before the Android ARM64 build job runs.
- Local host test result: PASS.

The CI workflow has been updated, but this report does not claim that the remote GitHub Actions run has completed.

## Next work

1. Trace callers/callback registration for the FDE function and identify the third argument's concrete owner type.
2. Trace the producers of the 0x208 and 0x248 blocks and classify their descriptors.
3. Independently identify the world renderer's camera matrix consumer.
4. Continue ECS metadata-to-runtime access analysis for entity AABBs.
5. Validate the Android ARM64 build through GitHub Actions before producing any artifact.

No working in-game ESP is available yet.
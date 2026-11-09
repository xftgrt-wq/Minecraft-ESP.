# Near-plane edge clipping and CI correction

Date: 2026-11-09

## Geometry changes

Added `EspScreenSegment` and `esp_project_aabb_edges()` to the geometry module.

The new function:
- transforms the eight AABB corners into homogeneous clip space;
- visits the 12 canonical AABB edges;
- drops edges fully behind the `w <= epsilon` plane;
- interpolates one endpoint for edges crossing that plane;
- returns up to 12 finite screen-space segments with depth values.

The older `esp_project_aabb()` remains available and still returns the independently projected corners. Consumers drawing a complete wireframe should use the segment API for edges that cross the camera plane; the old corner-only API cannot represent a clipped edge endpoint.

## Tests added

The test suite now checks:
- all 12 canonical edge indices;
- identity-matrix projection;
- all 12 edges when the box is fully in front;
- rejection when the entire box is behind the camera;
- a synthetic `w=z` transform where four vertical edges cross the near plane;
- finite coordinates for all returned segments;
- invalid bounds and null output.

The expected result for that synthetic crossing case is 8 visible segments: four edges on the front face and four clipped crossing edges.

## CI failure found and corrected

The first GitHub Actions run for this change failed before executing tests because the compile command did not add `src/main/cpp` to the header search path. The compiler reported:

```
fatal error: esp_geometry.h: No such file or directory
```

The workflow compile command was corrected to include `-Isrc/main/cpp`. A new workflow run is triggered by that workflow-file commit; its result must be checked separately before claiming the tests or Android build passed.

## Remaining limitations

- This clips only against the homogeneous `w=epsilon` camera plane. It does not yet clip against the four side planes or the far plane, so very distant/off-screen endpoints may still be outside the viewport.
- The matrix convention remains an explicit OpenGL-style column-major assumption. It has not yet been validated against a live Bedrock frame.
- This is a geometry module only. It does not yet enumerate entities, retrieve live AABBs, acquire the render matrix at a safe callback, draw segments, or expose a toggle.
- No offsets or hooks are inferred from this geometry work.

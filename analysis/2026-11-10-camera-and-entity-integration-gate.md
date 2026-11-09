# 2026-11-10 — Camera render-path consolidation and integration gate

## Target identity
The static analysis notes consistently refer to the uploaded Bedrock Android ARM64 binary:
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`
- GNU Build ID reported by the ELF audit: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Strongest camera evidence
The current view-projection matrix is not merely a guessed camera field. Static analysis ties the TAA upscaling callable at VA `0x1290c4a8` to a generated callable under `dragon::framerenderer::modules::taau` whose callback signature contains `const TAAContext&`.

Within that callback:
- callback register `x21` is identified as the TAAContext reference;
- `TAAContext + 0x208` supplies 64 bytes to the descriptor named `CurrentViewProjectionMatrixUniform`;
- `TAAContext + 0x248` supplies 64 bytes to `PreviousViewProjectionMatrixUniform`;
- `+0x288` and `+0x290` contribute to the current-world-origin payload;
- `+0x1e8`, `+0x1ea`, and `+0x1f8` contribute to render/display dimensions.

This is a substantially stronger identification than the earlier LevelRendererCamera vtable candidate. The TAA callback has explicit RTTI/callable-type evidence and named uniform descriptors.

## Why this still is not an ESP camera hook
The identified context is private to a TAA render-graph pass. The analysis has not established:
- that the pass runs in every world-render mode or every frame;
- that the callback can be observed through a supported Preloader API;
- the callback's complete ABI and safe context lifetime for a detour;
- whether its matrix and origin share the exact coordinate convention required by entity AABBs;
- whether the HUD overlay submission is synchronized with this pass.

The Preloader 1.5.13 public SDK exposes no world-render callback or ECS entity API. The generic detour API cannot validate private callback ABI or pointer lifetime.

## Entity-side evidence
Exact component/system signatures establish a movement dataflow:
- `StateVectorComponent` is read by the actor hitbox-update system;
- `HitboxComponent` is written by that system;
- `AABBShapeComponent` participates in movement/collision;
- `ActorDataBoundingBoxComponent` is a distinct scale-by-age path.

These are useful type-level facts, not runtime component layouts, storage addresses, entity handles, or an iteration API. The two RTTI vtable candidates tested on the user's device returned zero and remain rejected.

## Integration decision
Do not enable a detour at `0x1290c4a8`, do not use LevelRendererCamera slot 24 as a hook, and do not dereference guessed ECS fields. The remaining missing proof is not a small constant or one more drawing primitive: it is a safe runtime route to both entity bounds and the camera data in a compatible frame.

## Work that is already usable
- HUD bridge: device-confirmed diagnostic rendering.
- AABB geometry: standalone projection, canonical 12-edge topology, and homogeneous frustum clipping.
- Overlay bridge: accepts screen-space line segments and assigns player/item colors.
- Unit tests: exercise identity/affine projection, behind-camera rejection, near-plane crossing, side/far-plane clipping, and invalid input.

No live entity source currently feeds the overlay bridge. Therefore there is still no functioning in-game 3D ESP.

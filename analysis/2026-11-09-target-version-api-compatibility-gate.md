# Target-version API compatibility gate — 2026-11-09

## Search scope

Searched public GitHub source and SDK material for:
- Minecraft Bedrock 1.26.33 ARM64 actor enumeration signatures;
- 1.26.33-specific `ActorFetchNearbyActorsSorted` definitions;
- 1.26.33-specific `HitboxComponent` / `AABBShapeComponent` layouts;
- LeviLauncher 1.5.13 world-render callback examples.

No public exact-version implementation or verified 1.26.33 ARM64 signature set was found in the queried repositories.

## Nearby projects are not drop-in compatible

The current public BedrockTools source contains actor-related signatures and component references, but its inspected release manifests target newer 1.26.5x/1.26.60 families, not 1.26.33. A previous direct check of nine representative signatures from its 1.26.60 ARM64 set against the supplied 1.26.33 executable text returned zero matches.

A GitHub search also surfaced community SDK examples that explicitly describe their actor-fetch ABI as verified against BedrockTools 1.26.60 ARM64. Those examples are useful for identifying concepts and candidate call graphs, but they are not evidence that the same function signature, ABI, component layout, or offsets apply to 1.26.33.

## Decision

Do not import newer offsets, component hashes, actor-vector layouts, or function typedefs into this target build. In particular, an EnTT-style `EntityContext` wrapper from a Windows/community SDK does not establish the layout of the Android 1.26.33 runtime registry.

## Safe next step

Continue target-binary analysis from the supplied 1.26.33 ELF:
1. identify the actor/entity registry owner and lifetime;
2. trace the exact system accessors that read `StateVectorComponent`, `HitboxComponent`, and `AABBShapeComponent`;
3. establish component payload layout from target-version code paths, not external offsets;
4. find a world-render callback whose ABI and lifetime can be validated independently;
5. test the resulting camera transform and AABB against controlled in-game positions before enabling any drawing.

## Geometry regression coverage

The host geometry test suite now includes a non-identity column-major affine transform with explicit expected screen coordinates, in addition to identity, frustum clipping, camera-plane crossing, invalid-input, and non-finite-input cases. This strengthens the projection module's own regression coverage; it does not prove that the live Bedrock camera matrix uses this convention.

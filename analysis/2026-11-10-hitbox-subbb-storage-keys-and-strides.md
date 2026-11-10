# Resolve HitboxComponent and SubBBsComponent storage keys and slot strides

Date: 2026-11-10

## Exact target

ARM64 `libminecraftpe.so`, Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

## HitboxComponent

A type-keyed lookup callsite at `0x9e3e270` passes key `0x5eb4e8fb` to helper `0x9e59320`. That helper allocates a wrapper and installs vtable address point `0x14061170`.

The vtable RTTI identifies:
- `std::shared_ptr_emplace<entt::basic_storage<HitboxComponent, EntityId>>`;
- underlying `entt::basic_storage<HitboxComponent, EntityId>`.

The first storage lookup method at `0x9e59614` uses the same paged entity-key scheme seen in AABB storage and returns:

`page + ((entity_key & 0x7f) * 0x18)`

Thus HitboxComponent storage uses a 0x18-byte slot stride.

## SubBBsComponent

A type-keyed lookup callsite at `0x9e7adbc` passes key `0x0b0822d1` to helper `0x9e7af04`. That helper allocates a wrapper and installs vtable address point `0x14060a28`.

The vtable RTTI identifies:
- `std::shared_ptr_emplace<entt::basic_storage<SubBBsComponent, EntityId>>`;
- underlying `entt::basic_storage<SubBBsComponent, EntityId>`.

The first storage lookup method at `0x9e7b450` returns:

`page + ((entity_key & 0x7f) * 0x18)`

SubBBsComponent storage therefore also uses a 0x18-byte slot stride.

## Comparison with AABBShapeComponent

The earlier AABB storage report established:
- RTTI: `entt::basic_storage<AABBShapeComponent, EntityId>`;
- first lookup method: `0x873f530`;
- slot stride: `0x20` bytes.

The three concrete storage layouts now identified are:

| Component | Type key | Slot stride |
|---|---:|---:|
| AABBShapeComponent | not yet recovered | 0x20 |
| HitboxComponent | `0x5eb4e8fb` | 0x18 |
| SubBBsComponent | `0x0b0822d1` | 0x18 |

The Hitbox and SubBBs keys are established by callsite immediates plus the concrete wrapper RTTI; they are not guesses from the component names.

## ESP relevance and limits

This provides concrete storage identities and per-entity slot strides for all three bounds-related components named by the CameraAimAssist strict-system metadata. It is a useful step toward understanding the game's own bounds-selection path.

It still does not reveal the internal fields of the 0x18/0x20-byte components, the exact AABB storage instance in the strict view, or a safe way to snapshot these components on the render thread. Do not treat slot strides as component field offsets, and do not invoke the storage lookup methods from the injected mod yet.

## Next target

Trace the code that reads the returned HitboxComponent/SubBBsComponent payloads and determine whether it combines them with AABBShapeComponent to produce world-space candidate bounds. Then recover the AABB storage key or direct view pointer path.

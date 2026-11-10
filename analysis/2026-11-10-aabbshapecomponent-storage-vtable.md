# AABBShapeComponent EnTT storage vtable and paged lookup

Date: 2026-11-10

## Exact target

ARM64 `libminecraftpe.so`, Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

## RTTI-to-vtable association

The RTTI name at `0x27b9863` is:

`entt::basic_storage<AABBShapeComponent, EntityId, allocator<...>>`

The typeinfo object is at `0x13f71648`. The relocation at `0x13f715e8` points to that typeinfo, and the first virtual-function entry at `0x13f715f0` resolves to `0x873f530`.

This ties the following method disassembly to the concrete AABBShapeComponent storage vtable rather than to an unrelated storage implementation.

## First virtual method at 0x873f530

The method body is:

- `lsr x8, x1, #4`
- `ldr x9, [x0, #0x50]`
- mask the shifted entity key to a page-map index;
- load a page pointer from the map;
- mask the low 7 bits of the entity key;
- return `page + ((entity_key & 0x7f) * 0x20)`.

This is a paged EnTT-style storage lookup. It establishes:
- the storage object keeps a page-map pointer at offset `+0x50`;
- the key selects a page using its upper/index bits;
- the low 7 bits select one of 128 slots in that page;
- each component slot has a 0x20-byte stride.

The method does not itself validate entity generation; generation/sparse-index validation is handled by other storage methods in the same vtable, including the method at `0x873f54c`.

## Additional vtable methods

The adjacent AABB storage virtual methods include:
- `0x873f54c`: validates the sparse entry for the entity key and copies/returns the component data path;
- `0x873f628`: updates sparse-index bookkeeping for a range of entity keys;
- `0x873f68c`: clears/reset-related storage state;
- `0x873f700`: validates or inserts an entity-key entry;
- `0x873f778` and `0x873f8bc`: cleanup/release paths.

These are storage-management routines, not an externally safe API. Their exact C++ method names are not recovered.

## ESP relevance

We now have a concrete, RTTI-backed AABB component storage implementation and its per-entity paged lookup shape. Combined with the strict-system RTTI showing AABBShapeComponent in the CameraAimAssist view, this supports the possibility of resolving an entity's AABB from the appropriate strict context.

Still missing:
- a proven route from the callback's context object to this exact AABB storage instance;
- the AABBShapeComponent field layout and coordinate convention;
- proof the candidate range includes all players/items;
- a safe snapshot/handoff to the HUD render thread.

Do not hardcode the `+0x50` storage offset as a global or context offset. It applies only to this RTTI-identified AABB storage object. Do not call this vtable method from the mod until the instance type, ABI, and lifetime are proven.

## Next static target

Trace a callsite that obtains the AABBShapeComponent storage through the same strict-context lookup path, then inspect how the returned 0x20-byte component is read to form world-space bounds. Compare that access with HitboxComponent and SubBBsComponent to determine which bounds source is appropriate for players and dropped items.

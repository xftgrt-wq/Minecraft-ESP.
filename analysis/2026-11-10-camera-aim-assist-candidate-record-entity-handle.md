# Candidate record entity-handle evidence in the 0x34-byte range

Date: 2026-11-10

## Exact target

This follow-up uses the uploaded ARM64 binary with Build ID `2e318db12824cadb2618754ab7c82fa96fb30659`.

Relevant caller loop: `0x9e440dc–0x9e444bc`.

## Record layout observations

The caller advances its current record pointer by `0x34` bytes. The following fields are directly accessed:

- `+0x18`, `+0x1c`, `+0x20`: three adjacent float-sized values copied into a 12-byte vector passed to helper `0x11879140`.
- `+0x24`: a float result is written after the record is processed.
- `+0x28`: loaded as a 32-bit value into `w20` at `0x9e44214`.
- `+0x30`: byte flag checked in the loop and set to 1 when a record is marked complete/invalid for the current pass.

## Strong entity-handle-shaped validation path

The 32-bit value from record offset `+0x28` is not merely copied or compared. At `0x9e44264–0x9e44300`, it is decoded into fields and used to validate two indexed table lookups:

- `ubfx x9, x20, #11, #7` extracts a 7-bit field from bits 11–17.
- `and x10, x20, #0x7ff` extracts the low 11 bits.
- `and w13, w20, #0xfffc0000` preserves the upper packed bits.
- The code uses the extracted index to address pointer tables, then uses the low 11-bit field to index 32-bit entries.
- It XORs table values with the packed upper bits and checks against sentinel/range values before continuing.
- A second table path repeats similar validation against another storage structure.

This resembles packed entity-ID/generation validation against sparse indexed storage. It is strong evidence that each candidate record carries an entity-like handle at `+0x28`, but the exact C++ entity type and registry owner are not yet proven.

## What this changes

The 0x34-byte records are now more plausibly **entity-associated candidate records**, not just anonymous geometry samples. The helper's three-float input and the record's entity-like handle occur in the same per-record processing loop. This is a useful lead toward associating candidate positions with game entities.

However, this still does not provide a safe ESP source:
- no proven public or stable ECS iteration API;
- no proven component offsets for player/item AABBs;
- no proof the candidate list contains all relevant entities;
- no proven lifetime or render-thread ownership for the range;
- no camera matrix/data handoff validated against this candidate loop.

## Next static-analysis target

Trace the origin and concrete type of the object loaded into `x24` at `0x9e440dc`, and the backing tables reached through `x24`, `x24+0x10`, and the candidate record's packed handle. Determine whether this is an EnTT registry/storage lookup and whether the same owner exposes the AABB/actor components required by ESP.

Do not hook this loop or publish offsets yet. Keep game hooks disabled until the registry owner, ABI, lifetime, and thread synchronization are established.

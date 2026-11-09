# TAA uniform-store helper confirms 64-byte matrix payloads

Date: 2026-11-09

In the uploaded ARM64 binary, the helper called at VA `0x1267ab74` by the TAA callback receives:
- `x0`: the uniform/resource-store object;
- `x1`: the descriptor object;
- `x2`: the address of the value payload.

The helper begins by loading four 16-byte vector registers from the source:
- `ldp q0, q1, [x2]`
- `ldp q2, q0, [x2, #0x20]`

It then writes those four 16-byte blocks into its temporary/storage path. The helper also uses a 64-byte stride for its hash/storage operation.

This independently confirms that the payloads passed from `TAAContext + 0x208` and `TAAContext + 0x248` are consumed as 64-byte values, consistent with 4x4 float matrices.

This confirms payload size, not matrix element order or coordinate convention. It does not establish that the values can safely be read outside the TAA callback.

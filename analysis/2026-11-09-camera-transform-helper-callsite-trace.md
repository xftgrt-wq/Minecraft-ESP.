# Camera transform helper call-site trace — 2026-11-09

Target: exact uploaded Bedrock Android ARM64 binary.
SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## New call-site result

A full scan of the executable segment for AArch64 `BL` instructions targeting `0xbd9c4d0` found exactly three direct call sites:

- `0xbd701ac`
- `0xbd70224`
- `0xbd702b0`

All three occur inside the previously examined camera/render-state method at `0xbd6f97c`.

## Argument and mode differences

At the first call (`0xbd701ac`):
- x0 = stack temporary at `sp + 0x750` (output)
- x1 = stack record at `sp + 0x570` (input)
- s1 = selected scalar value held in s8
- s2 = float constant assembled as `0x41e0a3d7` (approximately 28.08)
- w2 = 0

At the second call (`0xbd70224`):
- same output and input temporary addresses
- s1 = s8
- s2 = float constant assembled as `0x4197a832` (approximately 18.96)
- w2 = 1

At the third call (`0xbd702b0`):
- same temporary input/output addresses
- s1 comes from a separate stack float at `sp + 0x11c`
- s2 uses the `0x41e0a3d7` constant
- w2 = 0

The method then copies the 0x58-byte helper output into three state regions rooted at:
- camera/render object + `0xd18`
- camera/render object + `0xd88`
- camera/render object + `0xdf4`

The surrounding branches consult a virtual method reached through an object at `[x22 + 0x970]`, and compare returned integer modes against 1, 2, and 1000. Thus the three helper invocations are mode-dependent render-state variants, not three independent matrix uploads.

## ViewProjection descriptor registration clarification

The descriptor initializer for the string `CurrentViewProjectionMatrixUniform` is at `0x1290cf18–0x1290cf70`. It:
1. acquires a static initialization guard,
2. passes the name string at `0x21e2297` into a descriptor-construction helper,
3. stores the resulting descriptor at `0x149100a0`,
4. calls registration helper `0x1261014c`,
5. releases the guard.

This is static descriptor registration. The helper `0x1261014c` is a generic registration routine called by hundreds of descriptor initializers. It is not evidence of a live matrix read or GPU upload.

A call to `0x1267ab74` at `0x12679490` is a separate generic routine that copies a 64-byte block and updates descriptor-associated state. No evidence yet connects that routine specifically to the camera's current ViewProjection uniform.

## Conclusions

Confirmed:
- exactly three direct calls to `0xbd9c4d0` exist in the inspected executable segment
- all are inside the camera/render-state method
- they construct and copy 0x58-byte mode-dependent records
- `CurrentViewProjectionMatrixUniform` is registered as a named descriptor at `0x149100a0`

Still unconfirmed:
- the runtime address/value of the ViewProjection matrix
- the code path that supplies the current matrix to the GPU uniform
- any direct bridge from that uniform to the camera state blocks above
- ECS runtime iteration and component field offsets

## Next trace

Do not treat `0xd18`, `0xd88`, or `0xdf4` as ViewProjection matrices. The next productive target is the renderer's uniform binding/upload implementation and the consumers of the descriptor object at `0x149100a0`.

# Reinspection of helper 0xbd9c4d0 — 2026-11-09

Target binary SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## Instruction-level findings

The function at VA `0xbd9c4d0` takes:
- x0: output structure pointer (saved as x19)
- x1: input object pointer (saved as x20)
- s0/s1/s2: floating-point scalar inputs (s1 and s2 copied to s12 and s9)
- w2: an integer/flag saved as w21

It reads the first float from `[x1]`, and another float from `[x1+8]`. It also reads a byte flag from `[x1+0x10]`; conditional logic later reads optional nested values via `[x1+0x18]`.

The body performs trigonometric-like calculations, using constants approximately 12000, 0.0087, and 0.0173 as encoded float values, then builds a compact output record.

At the end it writes fields from `x19+0x00` through `x19+0x54`, including many floats, zero-valued padding/flags, a constant 1.0 at `+0x50`, and the input flag at `+0x54`. The output record is therefore 0x58 bytes.

## Interpretation

This is not a 4x4 ViewProjection matrix: its output size is 0x58 bytes (88 bytes), and the observed stores do not form a contiguous 64-byte matrix write. The function is more consistent with constructing a compact orientation/transform-related record from angular/scalar inputs. Its exact C++ type and semantic field names remain unconfirmed.

This corrects the earlier tentative description of the helper as a possible matrix-producing routine. Do not use its output as a camera ViewProjection matrix.

## Consequence for ESP investigation

The helper is not the direct matrix source sought. The next useful path is to inspect the consumer of the 0x58-byte record and independently trace the actual descriptor named `CurrentViewProjectionMatrixUniform` from registration to its upload/bind site. The camera method's larger state blocks remain candidates, not confirmed matrix storage.

## Confirmed vs unknown

Confirmed:
- helper inputs and output pointer role at ABI level
- output writes span 0x58 bytes
- output includes trigonometric-derived float fields and a byte flag

Unknown:
- exact source-level type and field names
- whether any consumer uses the record for camera orientation
- actual ViewProjection matrix address and GPU upload site
- ECS entity iteration and component field offsets

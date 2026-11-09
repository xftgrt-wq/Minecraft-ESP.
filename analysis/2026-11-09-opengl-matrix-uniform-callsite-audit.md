# OpenGL matrix-uniform call-site audit — 2026-11-09

Target binary SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## Dynamic imports

The ELF imports the following OpenGL ES functions:
- `glUniformMatrix4fv`
- `glUniformMatrix3fv`
- `glGetUniformLocation`
- `glGetActiveUniform`

Their PLT entries are:
- `glUniformMatrix4fv@plt`: `0x13dbdb70`
- `glUniformMatrix3fv@plt`: `0x13dbdce0`
- `glGetUniformLocation@plt`: `0x13dbde70`

## Direct call-site scan

AArch64 direct BL scan of the complete executable segment found:
- 10 direct calls to `glUniformMatrix4fv@plt`
- 1 direct call to `glUniformMatrix3fv@plt`
- 1 direct call to `glGetUniformLocation@plt`

The ten Matrix4 calls are at:
`0x129cff08`, `0x129cff60`, `0x129d0434`, `0x129d0454`, `0x129d0cc8`, `0x129d0d1c`, `0x129d11ec`, `0x129d120c`, `0x129d2bbc`, `0x129d5910`.

The Matrix3 call is at `0x129d2ad8`; the only direct GetUniformLocation call is at `0x129c964c`.

## Strong classification of the discovered call cluster

The code around `0x129c964c` queries active shader uniforms and obtains a uniform location using:
- `glGetActiveUniform` at `0x129c9640`
- `glGetUniformLocation` at `0x129c964c`

The large routine around `0x129cfe00–0x129d120c` selects 4x4 data from renderer/shader state arrays, performs SIMD matrix multiplication into stack temporaries, then calls `glUniformMatrix4fv`. Example at `0x129cff08`:
- four 16-byte vector stores create a 64-byte matrix at `sp+0x7f0` through `sp+0x820`
- x3 is set to that temporary matrix
- w2 is zero
- `glUniformMatrix4fv` is called

This cluster is a real OpenGL uniform upload path. However, the surrounding control flow and uniform metadata indicate a generic shader-uniform binding/command submission routine. It is not yet proven to be the Minecraft world-camera path or the consumer of the named `CurrentViewProjectionMatrixUniform` descriptor.

## Important negative result

No direct `glUniformMatrix4fv` call occurs inside the previously investigated camera method at `0xbd6f97c`. The camera method may write through an abstraction or use a different backend; the direct OpenGL call cluster is in a separate code region.

## Conclusions

Confirmed:
- the binary has a concrete OpenGL 4x4 uniform upload path
- 10 direct Matrix4 upload call sites exist
- the principal cluster builds 64-byte matrices using SIMD operations before upload
- the same renderer code enumerates active uniforms and resolves their locations

Not confirmed:
- which of these matrix uniforms is the world ViewProjection matrix
- whether the call cluster is UI/cohtml rendering, world rendering, or a shared renderer backend
- the runtime matrix source corresponding to `CurrentViewProjectionMatrixUniform`
- any live ECS entity/component access interface

## Next trace

Follow the data structures feeding the matrix arrays used by the upload cluster, and compare their descriptor/name metadata against the registered `CurrentViewProjectionMatrixUniform`. Do not hook the generic upload routine as if it were confirmed world-camera data.

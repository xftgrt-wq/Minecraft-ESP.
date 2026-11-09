# Critical reclassification: Matrix4 upload cluster is FilePicker UI, not world camera — 2026-11-09

Target binary SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`.

## Direct disassembly confirmation

Re-disassembled `0x129cfe00–0x129cff30` from the uploaded binary using LLVM's AArch64 disassembler.

The enclosing symbol reported by the ELF disassembler is:

`Java_com_mojang_minecraftpe_FilePickerManager_nativeDirectoryPickResult`

At `0x129cff08`, the code:
- loads two SIMD matrix operands from renderer/UI state,
- performs four-lane multiply-accumulate operations,
- stores four 16-byte vectors into a 64-byte stack temporary at `sp+0x7f0` through `sp+0x820`,
- calls `glUniformMatrix4fv@plt`.

The same enclosing JNI method contains other uniform uploads, including `glUniform4fv`.

## Corrected conclusion

The previously discovered `glUniformMatrix4fv` cluster is inside the native file-picker result handler. It is therefore UI/file-picker rendering or associated UI shader work, not evidence of the Minecraft world-camera ViewProjection upload path.

This is a decisive negative result for the earlier hypothesis. The matrix upload call sites around `0x129cfe00–0x129d120c` must not be used as a world ESP camera hook.

## What remains valid

- The ELF imports `glUniformMatrix4fv`.
- The inspected JNI routine uploads 4x4 matrices to an OpenGL uniform.
- The camera class and its render-state method at `0xbd6f97c` remain a separate investigation.
- The static descriptor name `CurrentViewProjectionMatrixUniform` is registered at `0x149100a0`, but its runtime consumer is still unknown.

## Next useful target

Search for references to the descriptor object at `0x149100a0` through descriptor tables and uniform binding abstractions, then identify consumers in world-renderer call paths rather than JNI/UI functions. In parallel, continue mapping the ECS registry/storage route for entity components. No usable ESP hook, entity iteration interface, or component offsets have been established by this scan.

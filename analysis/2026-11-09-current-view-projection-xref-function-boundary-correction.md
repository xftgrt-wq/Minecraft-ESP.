# Correction: do not classify the CurrentViewProjection xref as FilePicker code — 2026-11-09

Target binary:
- ELF64 AArch64
- SHA-256: `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1`

## Reinspection result

A fresh local disassembly of VA `0x1290c680–0x1290c780` confirms the instructions at `0x1290c700–0x1290c714`:

```asm
0x1290c700: ldr  x8, [x21, #0x60]
0x1290c704: adrp x1, 0x14910000
0x1290c708: add  x1, x1, #0xa0
0x1290c70c: add  x0, x8, #0x8
0x1290c710: add  x2, x21, #0x208
0x1290c714: bl   0x1267ab74
```

This does pass descriptor address `0x149100a0` and source pointer `x21+0x208` to the generic 64-byte store helper.

However, the earlier report claimed this instruction sequence was inside `Java_com_mojang_minecraftpe_FilePickerManager_nativeDirectoryPickResult`. That claim was based on LLVM's nearest-symbol annotation, which is misleading here: the function's actual ELF symbol entry is at VA `0x125444b8` with declared size 1232 bytes, ending at `0x12544988`. It cannot contain code at `0x1290c700`.

Therefore the FilePicker attribution is unsupported and is withdrawn. The fact that the disassembler prints the FilePicker name with a large positive offset is not proof of function membership.

## Correct status

Confirmed:
- the descriptor address is materialized at the call site
- source pointer is `x21+0x208`
- the generic helper is called
- the helper is known from separate analysis to copy a 64-byte block

Unknown:
- enclosing function boundary and source-level owner type for the code at `0x1290c700`
- whether the source object is camera/world-renderer state or another subsystem
- whether the matrix is actually the world ViewProjection matrix
- any runtime-safe hook point

## Consequence

Neither of the previous classifications is justified:
- do not label this call site as world-camera data yet
- do not label it as FilePicker/UI data either

The right next step is to resolve the actual function boundary using unwind/FDE data and trace x21's provenance from its callers/constructor. Only then can this descriptor xref be classified. The 64-byte size is consistent with a 4x4 matrix, but size alone does not establish semantics.

No offsets should be hardcoded into an ESP implementation based on this call site until owner identity and matrix convention are validated.

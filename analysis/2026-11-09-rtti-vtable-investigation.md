# RTTI and vtable investigation — Bedrock 1.26.33 ARM64

Date: 2026-11-09

## Scope
Inspected the uploaded `libminecraftpe.so` (ELF64, AArch64) for C++ RTTI/vtable metadata and class names relevant to camera, player rendering, dropped items, and actor rendering.

## Findings

- The ELF dynamic symbol table contains many real exported/weak C++ RTTI and vtable symbols (for example V8, WebRTC, PPLX, and UI/runtime classes). This confirms that RTTI is not globally absent from the binary.
- The binary also contains a large pool of mangled-name strings matching `_ZTV`, `_ZTI`, and `_ZTS`. Some of these are symbol-name strings, so their presence alone is not proof that the corresponding object or vtable is live at a given address.
- No direct RTTI/vtable symbol names containing the target class strings `LevelRendererPlayer`, `DataDrivenRenderer`, `ItemActor`, `MinecraftCamera`, `ActorRender`, or `CameraComponent` were found in the scanned mangled-name records or dynamic symbol table.
- The human-readable class-name strings remain present in the binary, but previous xref validation showed that the camera-name references inspected so far are diagnostic/type-description metadata, not live camera accessors.
- Therefore, this pass did not establish a verified camera vtable, actor/item vtable, object layout, or safe rendering hook.

## Important limitation

The lack of matching RTTI symbols does not prove that the engine has no virtual classes. The relevant engine code may be stripped, hidden, built without emitted RTTI, or represented through templates/components with different names. A string match or an arbitrary function address is not a safe hook target.

## Next investigation

Continue by tracing concrete call/data-flow around validated render-loop functions and entity iteration, and by identifying camera matrix production/use from actual math and memory access patterns. Do not install or replace the game's original `libminecraftpe.so` with the current prototype. No functional ESP library has been produced.

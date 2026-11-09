# Follow-up: validating camera-name cross-references (2026-11-09)

The earlier cross-reference list for `MinecraftCamera::CameraComponent` was re-inspected in AArch64 disassembly.

## Important correction

The references at `0x0860EE40`, `0x0873D100`, `0x08748F30`, `0x08754868`, `0x09EC2588`, `0x0C8D9974`, and several addresses in the `0x12A...` range all load the same string address (`0x023002BF)) and copy 32 bytes from it into a stack/local metadata object. The surrounding instructions are consistent with constructing diagnostic/type-description data. They do **not** demonstrate access to a live camera component, camera matrices, or a rendering callback.

The cross-reference scanner was also checked against exact string file offsets:
- `MinecraftCamera::CameraComponent`: 27 direct ADRP+ADD references.
- `ScriptModuleMinecraft::ScriptItemActorComponent`: 1.
- `GlobalActorComponent`: 2.
- `ActorEventCoordinator::sendActorDroppedItem`: 1.
- `LevelRendererPlayer::preRenderUpdate`: 0.
- `DataDrivenRenderer::render`: 0.

## Current engineering conclusion

These string references are not valid hook candidates. No hook address or structure offset is being inferred from them. The next meaningful step is to recover RTTI/vtable relationships or obtain exact-build symbols/signatures, then trace actual call sites for actor iteration, camera matrices, and overlay rendering. Until that is validated, the prototype must remain non-invasive.

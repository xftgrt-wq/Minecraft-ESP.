# Exported symbol and renderer-path audit — Bedrock 1.26.33 ARM64

Date: 2026-11-09

## Binary checked
- File: `libminecraftpe.so`
- ELF64 / AArch64
- GNU Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`

## Symbol-table results

A fresh pass over the defined dynamic symbols found:
- 88,734 defined dynamic symbols.
- 3,621 exported `_ZTV` vtable symbols.
- 1,498 exported `_ZTI` typeinfo symbols.
- 1,512 exported `_ZTS` type-name symbols.

The RTTI is therefore present in the binary, but the target Minecraft engine classes were not found among the relevant exported RTTI/vtable names. The visible RTTI entries predominantly belong to bundled libraries such as V8, WebRTC, Xbox services, and related support code.

## Renderer symbols that are present

The dynamic symbol table exposes a substantial UI rendering stack under the `renoir` and `cohtml` namespaces. Examples include:
- `renoir::CommandBufferImpl::SetMatrixWithForce`
- `renoir::CommandBufferImpl::GetMatrix`
- `renoir::CommandProcessor::DrawFillRect`
- `cohtml::ViewRendererImpl::ExecutePaint`
- `cohtml::ViewRendererImpl::SetRenderTarget`
- `cohtml::SystemRendererImpl::RegisterRenderThread`

These are UI/Cohtml rendering interfaces, not evidence of Minecraft world-entity rendering or a safe hook into the Bedrock actor renderer. Their existence may be relevant to a future HUD-overlay route, but no supported API or safe integration point has been established from this library alone.

## What this establishes — and what it does not

This pass confirms that the binary retains many exported RTTI/vtable records and a separately identifiable UI rendering subsystem. It does not reveal a verified player/item enumeration routine, camera view-projection matrix accessor, actor render callback, or a safe place to inject 3D world-space outlines.

No hook address, object offset, or calling convention is claimed. Do not replace the original game library with the prototype.

## Next practical requirement

To move from static guesses to a working implementation, the next useful evidence would be an exact-version supported Levi Launcher/Preloader interface for native overlays or a matching 1.26.33 symbol/signature map. Without one of those, hooking stripped game internals remains unverified and unsafe.

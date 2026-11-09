# TAA callback RTTI and context identification

Date: 2026-11-09

Binary Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`.

## New ownership evidence

A raw 64-bit scan found the code pointer `0x1290c4a8` in the relocation-backed table around VA `0x145e2cf8`. The table is adjacent to RTTI objects whose names are present in `.rodata`.

The relevant RTTI name at string VA `0x32b6dd3` demangles in part to:

```
std::__ndk1::__function6__base<
  void(
    const dragon::framegraph::ExecutionContext<
      dragon::framerenderer::RenderContext
    >&,
    const ...::TAAContext&
  )
>
```

A neighboring callable RTTI name at `0x32b6fe9` is a generated callable/lambda type nested under `dragon::framerenderer::modules::taau` and includes `TAAContext`.

This ties the routine at `0x1290c4a8` to the TAA upscaling render-graph callback machinery, rather than the unrelated JNI FilePicker symbol that LLVM's nearest-symbol annotation misleadingly displayed.

## Context argument interpretation

At the routine entry:
- `x0` is saved as the callable object pointer;
- `x1` is saved as the first callback argument;
- `x2` is saved as `x21`, the second callback argument.

Given the RTTI signature, `x21` is strongly identified as the `TAAContext` reference for this callback. Consequently, the observed fields are now more meaningful than arbitrary offsets from an unknown function context:

| TAAContext-relative field | Observed use |
|---|---|
| `+0x208` | 64-byte source passed to `CurrentViewProjectionMatrixUniform` |
| `+0x248` | 64-byte source passed to `PreviousViewProjectionMatrixUniform` |
| `+0x288` (8 bytes) + `+0x290` (4 bytes) | 12-byte value passed to `CurrentWorldOrigin` |
| `+0x29c` (4 bytes) | final 4 bytes of the value passed to `PreviousWorldOrigin`; first 8 bytes come from a separate live pointer at `x22+0xa8` |

The matrix payloads are each 64 bytes and are registered under explicit current/previous view-projection uniform names. The world-origin payloads are each 12 bytes.

## Confidence and remaining work

High confidence:
- the function belongs to the TAA upscaling callback machinery;
- `x21` is the TAAContext callback argument;
- the listed offsets are fields used by this specific TAAContext callback;
- current/previous matrix and world-origin uniform roles are confirmed by descriptor names.

Not yet established:
- whether this TAAContext exists or is accessible in the main world-render pass at the desired point;
- whether its matrix uses exactly the same coordinate space as entity AABBs;
- whether world-origin rebasing must be applied before projection;
- the stable source and lifetime of the previous-origin pointer in `x22`;
- a safe public Preloader/render callback for drawing an overlay;
- the exact entity iteration and AABB component access offsets.

These are TAAContext-relative observations, not universal game offsets. Do not hardcode them into an injected hook until the callback lifetime and ABI are independently verified.

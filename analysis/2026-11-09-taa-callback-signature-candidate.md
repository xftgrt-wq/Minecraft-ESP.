# Static signature candidate for the TAA callback

Date: 2026-11-09

Target binary Build ID: `2e318db12824cadb2618754ab7c82fa96fb30659`.

## Candidate

The routine previously identified as the TAA upscaling render-graph callback begins at VA/file offset `0x1290c4a8`.

Its first 40 bytes are:

```
FF 83 07 D1 FD 7B 18 A9 FC 6F 19 A9 FA 67 1A A9
F8 5F 1B A9 F6 57 1C A9 F4 4F 1D A9 FD 03 06 91
48 D0 3B D5 39 00 01 D0
```

A byte-exact scan of the uploaded file found this 40-byte sequence exactly once, at `0x1290c4a8`. The first 24 bytes alone were not unique; use the full 40-byte sequence if a static signature comparison is needed.

## Why it is only a candidate

- The sequence is unique in the supplied on-disk binary.
- It has not yet been resolved in a running process through Preloader's `resolveSignature()`.
- The function has not been hooked.
- The callback's argument ABI and TAAContext fields are supported by prior static RTTI/disassembly analysis, but runtime lifetime and safe interaction with the renderer are still untested.

Do not install a hook from this report alone. First verify that the signature resolves exactly once in the loaded `libminecraftpe.so`, then validate the callback timing and original-function forwarding in an isolated test world.

# Runtime log interpretation and Preloader signature-resolver limitation

Date: 2026-11-09  
Target: Minecraft Bedrock Android 1.26.33, ARM64; LeviLauncher 1.5.13.

## Device evidence

The user's in-game Logcat screenshot contains these messages from the mod:

- `Minecraft ESP native module loaded (render hooks pending)`
- `TAA callback signature candidate resolved at 0x77e0f694a8; hook remains disabled`
- `3D ESP toggle registered in LeviLauncher Mod Menu`

This confirms that the native mod's load and enable lifecycle reached the diagnostic code, that the menu module registered, and that the resolver returned a non-zero address for the TAA byte pattern. It does **not** confirm that the TAA function was hooked, that the callback runs, or that entities are being enumerated. No boxes are expected yet because no live AABBs are submitted to the HUD bridge.

## Important resolver behavior verified in the exact pinned SDK

The project pins Preloader commit `8c19383f417744d3165165de9afca42b60059174`, matching LeviLauncher 1.5.13.

In that revision, `src/pl/memory/Signature.cpp` implements `scanCompiledPatterns()` by recording the first match for each signature and immediately marking that pattern inactive:

```cpp
found[patternIndex] = region.start + candidateOffset;
active[patternIndex] = false;
--unresolved;
```

The scan stops looking for that signature after the first match. `resolveSignature()` delegates to `resolveSignatures()`, so a non-zero result is a first-match result, **not a uniqueness count**. Consequently, the device log line must not be interpreted as proof that the signature occurs exactly once in the loaded process image.

Pinned source:
https://github.com/LiteLDev/preloader-android/blob/8c19383f417744d3165165de9afca42b60059174/src/pl/memory/Signature.cpp

## Candidate address

The logged candidate is `0x77e0f694a8`. The statically identified callback VA/file offset is `0x1290c4a8`. Their difference is `0x77ce65d000`, which is a plausible load bias, but the current diagnostic does not independently enumerate ELF load segments or verify that this address belongs to the expected executable mapping. Treat the arithmetic as a consistency lead only.

## Safety gate before any hook

Do not install the candidate hook yet. Required evidence:

1. Identify the exact executable mapping(s) of the loaded `libminecraftpe.so` from process maps and ELF load segments, and confirm the candidate's module-relative offset.
2. Independently count all matches for the complete 40-byte pattern across executable mappings; do not use Preloader's first-match resolver as a uniqueness test.
3. Verify the callback ABI, argument lifetime, thread, and call frequency with a non-invasive diagnostic or a purpose-built test build.
4. Confirm the original function can be forwarded correctly and that the TAA context is not retained beyond the callback.
5. Only then consider a hook. Never dereference the candidate or treat its address as a callable function based only on the resolver log.

## Current implementation status

- Native library loads on the user's device.
- Mod Menu toggle registers.
- Signature resolver returns a candidate.
- Hook remains intentionally disabled.
- Entity enumeration, live AABB retrieval, camera matrix validation, and HUD frame submission are still not implemented.

The next work item is a read-only, multi-match validation of the candidate against the loaded module's executable segments. If this cannot be performed safely from the current Preloader API, keep the hook disabled and request a diagnostic build that reports mapping boundaries and all pattern matches without modifying game code.

# 2026-11-09 — Runtime RTTI diagnostics returned zero

Target: Minecraft Bedrock Android 1.26.33, ARM64, LeviLauncher 1.5.13.

## Device observation

The user searched Logcat for `RTTI` and manually reported:

- `ClientInstance` slot 31 = `0`
- `LivePlayerRenderer` slot 17 = `0`

This is a user-reported observation, not a captured log export. The two candidate resolutions therefore failed on the user's running game. No executable-address validation is possible for a null result.

## Interpretation

The public static-analysis lead used for these candidates came from a separately identified third-party clone APK, not a verified dump of the user's installed game. A zero result does not establish that the classes or slots do not exist; it may mean the RTTI spelling, module scan assumptions, stripped/changed metadata, or resolver search scope do not match the user's binary.

Do not replace zero with guessed offsets, scan arbitrary memory, dereference candidate pointers, or install hooks. The previous two slot numbers are rejected as runtime integration inputs until independently validated.

## Next safe investigation

1. Inspect the pinned Preloader implementation of `resolveVtableFunction` and `resolveSignature` to understand exact lookup assumptions and failure cases.
2. Add only bounded, read-only diagnostics that report whether the target module is found and whether relevant ELF sections / RTTI names are present; avoid broad memory scans and pointer dereferences.
3. Obtain a trustworthy identity for the exact installed `libminecraftpe.so` (version/build and SHA-256) through a user-provided file or a safe on-device file hash. Do not assume the public clone binary is equivalent.
4. Re-evaluate exact-version public evidence against that identity.
5. Keep all render hooks disabled until slot semantics, ABI, lifetime, entity source, AABBs, and camera transform are independently validated.

## Current status

The Android ARM64 diagnostic build for commit `798352b92200390b0cb9a1667834e733ddeb395e` completed successfully in GitHub Actions run `42680390526`. Build success validates compilation/package creation only; it does not validate an in-game ESP.

No new binary should be installed based solely on these null RTTI results.

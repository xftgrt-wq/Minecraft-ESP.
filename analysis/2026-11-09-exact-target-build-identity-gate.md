# 2026-11-09 — Exact target build identity gate

Target requested by user: Minecraft Bedrock Android 1.26.33 / internal 1.26.33.1, ARM64, LeviLauncher 1.5.13.

## Version identity found

Public release metadata identifies Android ARM64 build 47475273 for Bedrock 26.33; Android internal version is 1.26.33.1. This is useful to label the target, but it is not a cryptographic identity for the user's installed native library.

Source: https://ru.minecraft.wiki/w/26.33_%28Bedrock_Edition%29

## Binary identity status

No authoritative SHA-256 for the exact official Android ARM64 `libminecraftpe.so` build 47475273 was found in the sources checked. A separate public reverse-engineering report used a binary described as a third-party clone APK, so its reported hash and RTTI/vtable claims cannot be assumed to match the user's installation.

## Engineering decision

- Keep candidate slots 31 and 17 rejected as runtime integration inputs because the user's Logcat reported both as zero.
- Do not guess alternate RTTI names, slot numbers, actor offsets, or hook signatures.
- Do not install hooks or patch the game library.
- Continue using the existing geometry tests and HUD command bridge as isolated components only.
- The next meaningful exact-version validation requires the user's actual `libminecraftpe.so` or its SHA-256 plus ELF build-id. Without that, static claims cannot be matched reliably to the user's binary.

## Current implementation boundary

The project currently has geometry/projection code and a Mod Menu toggle, but no validated live entity enumeration, player/item AABB source, or safe per-frame camera/render callback. Therefore the toggle is not yet a functioning ESP.

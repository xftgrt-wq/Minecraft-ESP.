# 2026-11-09 — Device diagnostic unavailable; static-analysis gate

Target: Minecraft Bedrock Android 1.26.33 / ARM64, LeviLauncher 1.5.13.

## Latest user test

The user reports that the expected `Game ELF identity` line is absent from the available Logcat view. No screenshot or copied log is available. Treat this as **no runtime identity evidence**, not as proof that the library is absent or that its Build ID is missing.

Possible explanations include:
- Logcat is filtered to another process/tag/priority;
- the current installed package predates the identity diagnostic;
- the module lifecycle did not reach `enable()`;
- the logger output is routed elsewhere by the launcher.

These possibilities cannot be distinguished from the user's report alone. Do not claim a runtime Build ID.

## Build status

The documentation commit `f322c92098e1ad1f5e57c33755aed99c496c6572` triggered Android ARM64 workflow run #117:
- Run: https://github.com/xftgrt-wq/Minecraft-ESP./actions/runs/42687673161
- Conclusion: success
- Artifact: `minecraft-esp-arm64-prototype`
- Artifact ID: `13705951066`
- Artifact archive digest: `sha256:8819286b5e2471dce4890f9cd9b7ff5d944e7d4fe93f533caa1218f4c2869cdd`

This is a successful compile/package of the current source. It does not establish live rendering or entity access. The commit only records the diagnostic state; it does not add runtime ESP functionality.

## Verified implementation boundary

The current native module:
- registers the LeviLauncher menu entry and toggle;
- performs read-only module/signature diagnostics when the enable lifecycle runs;
- has a screen-segment submission bridge and AABB projection helpers;
- does not register a live entity iterator or render callback;
- does not submit live player/item geometry.

The user's earlier device test confirms the menu entry and toggle, but no boxes are visible. With no usable Logcat line in the latest test, do not infer whether the identity diagnostic executed.

## Next engineering gates (in order)

1. **Confirm which package is installed without requiring Logcat.** Use the artifact's manifest/version metadata and, if the launcher exposes it, its installed module version. Do not ask the user to replace the game's original `libminecraftpe.so`.
2. **Restore a minimal observable diagnostic path.** Prefer a small status field in the mod's own menu/config UI if the supported Preloader API permits it; otherwise keep the existing logger and give the user a single, simple filter instruction. Do not add an unverified game hook just to display diagnostics.
3. **Validate the exact running game ELF identity.** Compare runtime Build ID/hash only when an actual runtime value is obtained. The previously analyzed file hash `d659c56ebae1dd22a7014a70a293e3d68d26938f18dd88dfe4ee9e34eea9c8c1` and Build ID `2e318db12824cadb2618754ab7c82fa96fb30659` describe the supplied analysis binary, not yet the confirmed running process binary.
4. **Continue static tracing only against the exact binary.** The ECS component names/signatures and camera uniform names are leads, not field offsets or callable APIs.
5. **Keep TAA hook disabled** until ABI, argument lifetime, thread/timing, matrix convention, and safe unload behavior are proven.
6. **Only after entity iteration + live AABB + camera matrix + viewport + render timing are independently established**, connect them to the existing projection/submission path and test in-game.

## Explicit non-claims

- No working 3D ESP is confirmed.
- No live entity enumeration is implemented.
- No verified camera matrix is currently fed into the projection code.
- Through-wall rendering and unlimited-distance behavior are not verified.
- Successful CI is not an in-game compatibility test.

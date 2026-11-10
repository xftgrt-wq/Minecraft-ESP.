# 2026-11-10 — correction: shared global dispatch target is not an ECS accessor

## Why this follow-up was needed

The previous report identified a wrapper at `0x12b420ec` which loads a global object at `0x1473fa48` and invokes its vtable slot at `+0x18`. That observation was correct, but the earlier interpretation left open the possibility that this object might be the strict-system/ECS service.

A broader cross-reference check does not support that interpretation.

## New cross-reference evidence

The same global object access pattern appears in code well outside the entity and renderer subsystems. For example:

- `0x12b420ec`: small indirect-dispatch wrapper in a region containing JNI/file-picker related routines.
- `0x851f52c`: same global target and virtual slot are used inside an ASTC codec routine, `astc_codec::InfillWeights`.
- Numerous additional call sites in unrelated library code use the same address and dispatch slot.

The ASTC codec cross-reference is a strong negative control: the target is a broadly shared callback/dispatch facility, not evidence of a world-owned ECS registry. Its exact semantic type is still unknown, so it should not be labelled an exception handler or scheduler without further proof.

## Memory placement

The address `0x1473fa48` lies in the ELF `.bss` region, whose start is `0x1473fb00` only after the end of the `.data` region; the target is in the final bytes of `.data` immediately before `.bss`. This placement is consistent with a process-global object, but does not identify its C++ type or ownership semantics.

## Corrected conclusion

- Confirmed: the wrapper at `0x12b420ec` dispatches through a shared global object and its vtable slot `+0x18`.
- Confirmed: the same dispatch target is used by unrelated code, including ASTC codec code.
- Not confirmed: that the object is the ECS strict context, registry, world accessor, or hitbox-system owner.
- Therefore: do not use `0x1473fa48` as an entity source, do not hook it, and do not add it to runtime code.

## Next useful direction

Return to the adapter-registration metadata itself rather than following the shared dispatch target. The next pass should identify the constructor/registration code that emits the strict-system descriptor/adapter pair and follow the concrete owner object passed into that registration path. Separately, trace the EnTT storage typeinfo/vtable family to the factory/registration sites that instantiate storage objects. A usable runtime path requires an owner/context with a proven lifetime, not merely RTTI or a generic global dispatch service.

All game hooks remain disabled. No new runtime offset or hook ABI is justified by this result.

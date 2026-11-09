# Exact TAA file-offset diagnostic — 2026-11-09

## Change

The live signature diagnostic now parses the file-offset column of each matching executable mapping in `/proc/self/maps`. When the 40-byte TAA candidate is found, it logs:

- resolver candidate address;
- independent executable-map match count;
- first match address;
- computed file offset (`mapping_file_offset + match - mapping_start`);
- statically expected file offset `0x1290c4a8`.

The success message now requires all three checks to agree:
1. exactly one executable-map match;
2. independent match address equals Preloader resolver address;
3. computed file offset equals `0x1290c4a8`.

This is a diagnostic-only change. It does not install a hook, dereference TAA callback arguments, or alter rendering.

## Why

The previous device log established uniqueness and address agreement, but did not independently prove that the runtime match corresponds to the expected location in the target ELF image. The added file-offset field closes that specific verification gap, subject to normal ELF mapping assumptions and the exact target binary.

## Build status

A new Android ARM64 CI run is triggered by commit `a3fa6f43342bf083eb4ee28f02afc6a1a0789439`. Await its result before distributing this diagnostic build.

## Safety gate remains

Even a matching file offset does not validate callback ABI, call frequency/thread, argument lifetime, matrix convention, world-origin semantics, or whether the TAA pass is the right render stage. Hook remains disabled.

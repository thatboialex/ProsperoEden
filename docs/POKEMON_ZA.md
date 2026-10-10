# Pokémon Legends: Z-A on ProsperoEden (experimental)

**Status: user reports successful PS5 gameplay without the original Vulkan graphics corruption,
but intermittent stuttering remains.** The stability and frame-rate improvements
are experimental and are not a guarantee of sustained performance.

## Vulkan rendering fix

Eden's desktop AMD/RADV issue [#481](https://github.com/eden-emulator/Issue-Reports/issues/481)
documents broken/flickering graphics after leaving the first train station.
On 3 October 2026, users reported success with Vulkan, **Extended Dynamic State
EDS2**, and **Vertex Input Dynamic State disabled**. ProsperoEden now applies
this combination only to the Switch game with Title ID `0100F43008C44000`
when using Vulkan. Other titles keep the standard Vulkan state. Settings are
reset on each launch, so the workaround cannot leak into the next game.

The Vulkan extension capabilities of the PS5 Mesa/RADV port may differ from a
desktop GPU. To disable this experimental setting if the game crashes or
renders worse, create `compatibility.txt` alongside ProsperoEden's
`eboot.bin` and put this line in it:

```text
pokemon_za=off
```

Delete the line or set `pokemon_za=on` to retry the workaround. Restart
ProsperoEden after changing the file.

## Gameplay performance profile

The PS5 framebuffer and GPU shader compiler can cause pauses even when the
rendering corruption has been resolved. The per-title Vulkan performance profile
now limits **internal** rendering to 1x and ensures asynchronous shaders are on.
Your selected TV **output** resolution stays unchanged, with the selected upscale
filter handling the difference; it does not alter the EDS2/vertex-input graphics
fix. The frame-rate target is unchanged. The first playthrough can still pause
when a shader has not yet been compiled; a warmed cache may be smoother.

This profile is on by default for this title on Vulkan only. To compare with your
prior configuration while **keeping the graphics fix**, place this in the app's
`compatibility.txt` and restart:

```text
pokemon_za_perf=off
```

Set `pokemon_za_perf=on` to re-enable. For continued hitching, compare the
`EDEN_VULKAN_FRAME` worst-frame intervals and shader-cache warmup between two
play sessions before changing global settings.

## Files and cheats

Use your own legally dumped **Nintendo Switch** edition of the game and
console keys/firmware. This port does not play a Switch 2 binary. Check that
the library sees the game's Title ID `0100F43008C44000`. Install the proper
update file in `updates/` if needed; the cheat file must match the game's
**running Build ID**, not merely its Title ID. A cheat from a different update
may appear in the mod folder without being applied.

```text
<game files folder>/
  roms/Pokemon Legends Z-A.xci
  updates/<your dumped update>.nsp
  mods/0100F43008C44000/My Cheats/cheats/<matching build ID>.txt
```

The game menu is Library → Triangle → Mods. Enable the mod and then enable
the individual cheat entries in collections with more than one code.
Changes take effect on the next game launch. Turn all mods off when testing
game compatibility, then add them back one at a time.

## Logs needed for real PS5 confirmation

Enable detailed logging and immediate logs in Settings, then start the
game once and collect the files in `/data/prosperoeden/logs/`. The
`EDEN_GAME_COMPAT` line records whether the Z-A Vulkan workaround was applied.
Report exactly which failure occurs: not shown in the library, refuses to
launch, black screen, crashes, flashing geometry after leaving the station,
or poor FPS. Those are different problems requiring different fixes.

This change does not include any Nintendo game, keys, firmware, ROMs, cheats,
or copyrighted assets, and does not claim PS5 compatibility was tested.

## Verifying enabled cheats

The launcher can recognize a cheat file without knowing whether its pointer
chain points to live game memory. A game using cheats now reports a short
`cheat-vm` line on the first engine cycle and about every five seconds.
`entries` counts registered entries (including any default/master slot),
`enabled` counts those with code selected, `opcodes` counts executable code
words, and `writes` counts successful guest-memory writes. `rejected_reads`,
`rejected_writes`, or `failed_writes` indicate invalid/unwritable target
addresses or a version-specific cheat that cannot reach its pointer chain.
These are useful diagnostics, not proof that a game's own value changed as
intended. For version-dependent cheats prefer the folder form with a filename
matching the actual running 16-character Build ID.

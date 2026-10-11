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

## Internal resolution and performance

The original Z-A compatibility profile forced **1× internal rendering**
regardless of the Video > Internal resolution selection. Consequently choosing
2×, 3× or 4× did **not** actually request those internal scales; output
upscaling may simply enlarge the same 1× image and expose jagged edges or
sharpening artifacts.

The updated Z-A Vulkan profile keeps the EDS2 / static vertex-input
graphics correction **and asynchronous shader compilation** but now honors
the selected *internal* resolution. Higher scales can consume much more
PS5 shared graphics memory and may worsen frame pacing, culling, or aliasing;
this does not claim they are visually correct on your PS5. Test **1.5×,
then 2×**, using the same game scene, and restart after each change.
Prefer 1× if flickering or out-of-memory errors increase.

A conservative emergency option remains available in `compatibility.txt`:

```text
pokemon_za_scale=1x
```

Switch back with `pokemon_za_scale=selected` (the default). This option
changes only the internal-resolution cap; neither choice disables the
vertex-input graphics fix.

`pokemon_za_perf=off` is still available for a broader A/B comparison of
the shader/performance profile. The EDS2 correction has its independent
`pokemon_za=off` rollback.

The in-game **Live graphics** menu changes the *presentation filter* only.
It cannot safely change internal resolution or rebuild the active Vulkan
render targets. Make the latter changes in Game Settings > Video, then restart.

## Intermittent background building or prop popping

**Still experimental; not confirmed fixed.** User PS5 testing shows that the
EDS2/static vertex-input workaround eliminates severe scene corruption but a
building section may still disappear/reappear in Lumiose City. A single still
cannot distinguish incorrect query-based culling from stale guest memory,
geometry depth flicker, or asset streaming. Upstream Eden release notes also
warn that invisible objects/flickering can remain after vertex-input fixes.

The PS5 Vulkan backend includes two different host conditional-rendering
paths. The default is the faster query-based one; an existing exact
CPU-evaluated predicate mode avoids relying on pending GPU query results.
To compare *only* that source of geometry popping, put:

```text
pokemon_za_predication=cpu
```

in `compatibility.txt` next to `eboot.bin`, then restart the emulator. This
option affects only Pokémon Z-A on Vulkan. It does **not** disable EDS2 or
re-enable vertex-input dynamic state. CPU readbacks may introduce more frame
stutter; this is a diagnostic A/B option, not a verified fix.

If it does not resolve the disappearing building, set
`pokemon_za_predication=default` to restore the original renderer. Also
compare Library → Game settings → Performance with **Fast GPU OFF**,
**Unsafe DMA OFF**, and **Skip invalidation OFF** (GPU High, default DMA).
These options can affect freshness of guest GPU state but their impact on
this exact scene is not yet established.

Capture a 10-second clip from a stationary camera and one while panning
slowly, including the same arch/building wall. If it disappears according to
viewing angle rather than shimmering while stationary, investigate guest
occlusion/conditional rendering first; if it flickers while stationary,
investigate depth writes, shader/texture aliasing and the Vulkan driver.

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

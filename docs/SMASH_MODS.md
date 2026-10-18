# Smash Ultimate: Skyline and ARCropolis

ProsperoEden's existing ExeFS and RomFS mod loader can place Skyline and ARCropolis in the
guest game. ARCropolis reads character replacements from the emulated SD card, separate from
the game's `romfs`. **The user reports a successful full Infernape battle on PS5**
with Smash 13.0.4, emulator runtime 6cd74e1, Skyline, ARCropolis v4.0.8,
Smashline v1.6.6, Param Hook v13.0.4 and CSK Collection v5.0.4. The supplied
battle logs contain no guest faults, guest exceptions or Smashline panics and
include many presentation windows near 60 FPS, with some dips. This validates
that reported test setup; other packs and combinations still need individual tests.
The staging tool below prepares files; it does not implement missing guest kernel behavior,
verify a character against a Smash update, or establish that Skyline's hooks execute correctly.
ARCropolis's [upstream documentation](https://github.com/Raytwo/ARCropolis#work-in-progress-emulator-support)
describes Yuzu-family support as unfinished.

## Prepare files

Download `skyline.zip` from the [Skyline releases](https://github.com/skyline-dev/skyline/releases)
and `release.zip` from the [ARCropolis v4.0.8 release](https://github.com/Raytwo/ARCropolis/releases/tag/v4.0.8)
for **Smash 13.0.4**. That tag checks for game version 13.0.4; v4.1.1 checks for 13.0.5
and will refuse 13.0.4. v4.0.8 embeds menu resources and its ZIP contains only the plugin;
a separate `ultimate/arcropolis/resources` directory is not required for that release.
Use a Smash update supported by the selected framework and character/plugin versions. Read each
character pack's requirements: a cosmetic replacement and a new moveset can need different plugins.
The tool does not download characters or resolve plugin dependencies.

Extract a character pack from [GameBanana](https://gamebanana.com/games/6498). The directory
passed to `--character` must be the pack itself, with its `fighter/`, `ui/`, `sound/` or other
replacement paths directly inside. If the download contains `ultimate/mods/<pack>/`, pass that
inner `<pack>` directory. Follow the author's instructions for additional Skyline `.nro`
plugins; they are not character replacement files.

From the repository root:

```bash
python3 tools/stage-smash-mods.py \
  --skyline /path/to/skyline.zip \
  --arcropolis /path/to/release.zip \
  --character '/path/to/My Character' \
  --output results/smash-upload/data/prosperoeden
```

Add `--plugin /path/to/required-plugin.nro` for each additional Skyline plugin required by
the character pack (for example, its specified Smashline release). The tool checks the NRO
header and rejects duplicate plugin names; it does not establish version compatibility.

`--character` may be repeated or omitted to prepare just the frameworks. The output must be a
new directory. Nothing is installed on the PS5 by this command. A manifest records input archive
hashes and each staged file's SHA-256 for comparing transfers; these hashes are not upstream
signatures or a gameplay compatibility certification.

The resulting layout is:

```text
data/prosperoeden/
  mods/01006A800016E000/Skyline/
    exefs/main.npdm
    exefs/subsdk9
    romfs/skyline/plugins/libarcropolis.nro
  user/sdmc/ultimate/
    arcropolis/resources/...
    mods/My Character/fighter/...
```

Close ProsperoEden before transferring files. Back up any existing framework files and
ARCropolis configuration, then copy the staged `mods/` into the **selected game files folder**
and `user/sdmc/ultimate/` into `/data/prosperoeden/user/sdmc/ultimate/`. These roots differ when
**Settings > Game files** points at another drive. Existing saves and keys are not included in
the staging tree. Keep only one active Skyline ExeFS installation for Smash; disable any older
copy before enabling this named **Skyline** mod in the game's Mods screen.

Additional `.nro` plugins belong beside `libarcropolis.nro` in
`mods/01006A800016E000/Skyline/romfs/skyline/plugins/`, subject to the plugin author's instructions.
Do not copy `atmosphere/contents/` into that directory, and do not copy `ultimate/mods/` into
`romfs/`: ARCropolis reads it through the emulated SD filesystem.

## Validate on a console

1. Confirm the unmodified game boots with the intended update. Keep its existing memory setting
   unless a mod explicitly requires more; see [Console memory](SETTINGS.md#console-memory).
2. Start with the frameworks and no character pack. Enable detailed logging and capture the
   startup log. Verify ARCropolis actually initializes, rather than treating a visible Mods
   entry as proof. Check SD-card output under `/data/prosperoeden/user/sdmc/ultimate/arcropolis/`.
3. Add one compatible character pack and any of its required plugins. Verify that character's
   costume/model in a match, then exercise its moves and finish the match. Merely reaching the
   menu does not validate a custom fighter.
4. Record the ProsperoEden build, Smash update, framework versions, pack URL/version, and logs.
   If it fails, retain the first error and the reproduction details before changing versions.

A missing SD file, failed NRO load, and guest instruction fault require different fixes.
Do not infer that all custom characters work from a successful test of one cosmetic pack.

Host staging tests: `python3 tools/test-stage-smash-mods.py`.

## Emulator hook regression

The emulator now invalidates each core's cached instruction bytes when guest code flushes
modified executable memory. Previously, translated blocks were invalidated across cores but
a core could recompile the old bytes from its private page cache. This affects the writable
alias and cache-flush sequence used by Skyline hooks.

`tools/check-guest-hooks.py` runs two homebrew guests in the built host emulator. It checks
a patched function on all four pinned guest cores, separate writable/executable CodeMemory
views, process-code trampoline mappings, and clean process shutdown. The hook guest fails
against the previous emulator and passes with the cache fix. Results, binary/fixture hashes,
and logs are retained under `results/guest-hooks-*`. Run it after building the fixtures and
`eden-headless`; it is also part of `make test`.

These tests establish the exercised guest memory behavior. Smash **13.0.4**, Skyline,
ARCropolis, and a specific character pack still require the console validation above.

Guest cleanup also exposed a separate timing-thread lost wake during emulator shutdown.
The emulator now requests that thread to stop before signaling its wait events.
`tools/check-timing-stop.py <build-directory>` forces the old race using the generated
reset method and the pinned event implementation, then checks the corrected method.
The build scripts run this regression for both host and PS5 build derivatives.

## Diagnostics collected for Smash

Launching title `01006A800016E000` automatically enables mod diagnostics. No additional
Skyline TCP client is needed for messages Skyline emits through `svcOutputDebugString`.
The journal captures those messages directly, before Eden's asynchronous logging queue.

- `/data/prosperoeden/logs/mod-diagnostics.log`: session/source revision, backend, enabled
  mod summary, installed ExeFS/plugin file paths and SHA-256 hashes, character directory
  names, game build ID, guest debug messages, custom error details, exceptions and SVC breaks.
- Guest faults include the live stopped CPU context, general registers and FP control state,
  module base addresses and a guest backtrace. A no-execute exception records `fault_pc`
  before the JIT advances; `stopped_pc` can therefore differ by an instruction.
- `/data/prosperoeden/user/log/eden_log.txt`: engine log, including focused Loader and RO
  debug messages. Smash flushes each written engine entry; **Detailed logging** can enable
  broader tracing when reproducing a problem. Extra diagnostic writes can reduce performance.
- Native crash reports and matching logs stay under `/data/prosperoeden/logs/crash-*`.
  On the next launch, the journal and its previous segment move beside the native report;
  the existing retention policy keeps five crash sets. The active journal retains two
  approximately 8 MiB segments and caps each message at 64 KiB, with an explicit truncation marker.
- ARCropolis's own logs, when it emits them, remain under
  `/data/prosperoeden/user/sdmc/ultimate/arcropolis/logs/`. Keep these too.

After a failure, close the app and copy the logs before another reproduction. Keep the
matching emulator ZIP and character/plugin versions. Native reports must be symbolized
with the matching build's unstripped ELF, retained by the builder in `build/symbols/`.
Inventory is bounded to 256 code files; files above 64 MiB are explicitly marked as unhashed.
Installed-file inventory does not prove that a plugin loaded; correlate it with RO messages
and the fault's module addresses.

To bundle a copied `/data/prosperoeden` tree locally with Python 3.11 or newer:

```bash
python3 tools/collect-smash-diagnostics.py /path/to/copied/prosperoeden \
  --output results/smash-crash-logs.zip
```

The collector includes diagnostic logs and a file-hash manifest, excluding game files,
keys and saves. It does not upload anything. Logging cannot recover messages a plugin
never emits, or guarantee the final buffered engine entries after a kernel kill or power loss.
Actual PS5 crash collection still requires console validation; the guest-fault regression
uses a deliberately faulting homebrew guest in the host emulator.

## Interpreting the first PS5 startup logs

The supplied 702c38c PS5 logs show Smash loading, Skyline starting, and
`libarcropolis.nro` completing its main function. This establishes framework execution,
not completion of ARCropolis filesystem initialization or working gameplay. Four SD
character-pack directories are listed. The launcher's emulator patch list does not
list ARCropolis SD packs, or frameworks bundled directly into a game image.

One attempt failed before guest startup because shader catalog discovery called
`std::filesystem::relative`, which canonicalizes paths using operations denied on PS5.
Catalog keys now use lexical relative paths. This fixes that exception; the later attempt
in the supplied logs had already passed this stage. Its final filesystem-not-initialized
messages alone do not identify the cause of the subsequent startup stop.

For that remaining issue, retain `user/log/eden_log.txt` (and rotated segments),
`logs/mod-diagnostics.log` and `.prev`, and the ARCropolis logs under
`user/sdmc/ultimate/arcropolis/logs/`. Include how long the loading screen remained.
ARCropolis v4.0.8 detects emulators by checking whether game text is at `0x8004000`
or `0x8504000`; this PS5 session uses `0x80af0000` and therefore selects its Switch
behavior. Its `legacy_discovery` flag can bypass preset filtering, but that mismatch
has not been established as the cause of the startup stop. Do not change game addresses
or delete saves to work around it.

## Conflict found in the next PS5 run

The 1248936 run finishes all 3704 cached pipelines in about 25 seconds, runs Skyline,
and completes ARCropolis main. It then reports a conflict between
`(Additional Slot) All movesets` and
`(Hype Bros) Additional Slots/ui/param/database/ui_chara_db.prc`.
ARCropolis v4.0.8 discovery rejects a conflicting root, repeats discovery, and tries
to display a conflict error. The supplied log does not show discovery completing.

Close the emulator and temporarily move `(Additional Slot) All movesets` out of
`/data/prosperoeden/user/sdmc/ultimate/mods/`, keeping a backup outside the scanned
mods directory. If remaining movesets depend on that pack, move those out as well
for the isolation run. A framework-only boot with the pack directories held outside
`mods/` distinguishes this mod-set problem from another emulator startup problem.
Restore only a compatible set; two different `ui_chara_db.prc` files cannot be merged
by selecting an arbitrary overwrite order. Use the authors' supported combined setup.

The guest OpenFile panic in this capture occurs after return-to-menu and shutdown
begin; it does not establish the cause of the earlier loading stall.

## Null-call failure after removing the conflicting pack

The next supplied journal inventories three packs on both launches. Its first launch
has an actual pre-shutdown execution fault at address zero, with the return address
in `smashline_test`'s `main` at module offset `0x132C0`. Two loaded modules share
that internal name, so it does not identify a particular pack filename or establish
whether the fault is an incompatible plugin, missing dependency, or emulator bug.
The emulator automatically retries; that attempt finishes shader loading in about
159 seconds and later stalls after ARCropolis startup.

Inspect the exact pack `plugin.nro` binaries and author requirements before replacing
Smashline versions. For diagnosis, retain exact pack download links, all pack-level
`plugin.nro` files, and the companion Skyline `.nro` filenames and versions.
ARCropolis v4.0.8 emits a misleading invalid-file message for collected non-plugin
patches in `load_and_run_plugins`; those messages alone do not prove missing files.

The previous-session files in this upload repeat the earlier conflicting-pack run.
They do not capture the reported pack-free black intro. Preserve that baseline in
a separate log capture before restoring packs, rather than relying on rotated logs.

## Focused performance diagnostics

The framework-only capture still drops below 1 FPS, so character-pack content is not
required to reproduce the slowdown. All four framework plugins finish initialization.
Shader-cache loading finishes in about 25 seconds. The journal records 3916 events
(2603 engine warnings and 1270 guest debug messages) over its first 162 seconds;
frame progression continues after the journal quiets. These logs do not establish
which plugin or renderer operation dominates the later frame time.

The performance candidate batches normal journal records through the existing
one-second log flusher. Producers can append while storage writes a batch. A
256 KiB pending threshold applies backpressure by flushing; records are not silently
dropped. Faults and durable startup markers drain prior records and fsync. Explicit
Write logs at once still selects immediate writes. Buffered native-signal/power-loss
tails can lose about a second of normal journal records, or longer if storage is
stalled; this differs from a guest fault's synchronous drain.

Smash no longer forces every engine log line to flush. Every five seconds, release
Smash sessions append cumulative performance counters for JIT compilation, storage,
draw/dispatch and GPU producer waits, plus journal records/batches/bytes/I/O time.
`heap.log` includes timestamped CPU owner samples; stale or unsupported CPU samples
are explicitly marked and must not be read as zero load. Counters overlap and must
be differenced between snapshots, not summed into a CPU/GPU utilization percentage.
Focused timing adds clock reads on measured paths. This candidate needs hardware
comparison; no PS5 speedup or black-screen fix has been established.

Interpretation: `gpu_queue_wait` is the GPU worker waiting for incoming work,
not physical GPU execution time. `gpu_dispatch` and `draw` measure CPU processing
of GPU commands. Fence/presentation/queue-full timers measure producer waits;
they can overlap other work. High storage or journal I/O totals identify storage
latency, but background journal I/O is not automatically a blocked guest thread.
The same 247 file-size patch warnings persist in the zero-pack capture, so they
are not evidence specific to Infernape.

## Pin Smashline for Smash 13.0.4

Use [Smashline v1.6.6](https://github.com/HDR-Development/smashline/releases/tag/v1.6.6)
for the current 13.0.4 moveset test. v1.6.7 explicitly changes to 13.0.5 offsets;
v1.6.8 inherits those changes. Exported API names alone do not establish game-version
compatibility. The earlier recommendation of v1.6.8 was incorrect for this target.

The Infernape stage-loading capture first reports `NotRelocated` in
`src/create_agent.rs:1181`, then repeats Smashline panic dialogs and BRK exceptions.
ARCropolis completes the motion-list patch before this first panic. Replacing
`libsmashline_plugin.nro` with the official v1.6.6 binary corrects the identified
version mismatch; a PS5 match retest is still required. Keep only one active copy.
Official binary SHA-256:
`05fca8fa5c745a2dc6578e9eb3f1ace187a9b45bb9efbf03af2ef96dbbaf46d2`.

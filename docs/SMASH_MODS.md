# Smash Ultimate: Skyline and ARCropolis

ProsperoEden's existing ExeFS and RomFS mod loader can place Skyline and ARCropolis in the
guest game. ARCropolis reads character replacements from the emulated SD card, separate from
the game's `romfs`. **Gameplay compatibility has not been validated on ProsperoEden.**
The staging tool below prepares files; it does not implement missing guest kernel behavior,
verify a character against a Smash update, or establish that Skyline's hooks execute correctly.
ARCropolis's [upstream documentation](https://github.com/Raytwo/ARCropolis#work-in-progress-emulator-support)
describes Yuzu-family support as unfinished.

## Prepare files

Download `skyline.zip` from the [Skyline releases](https://github.com/skyline-dev/skyline/releases)
and `release.zip` from the [ARCropolis releases](https://github.com/Raytwo/ARCropolis/releases).
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

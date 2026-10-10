# Fast RMX: PS5 shared-memory mitigation (experimental)

**Status:** The player's reported out-of-memory stop has not been reproduced
on a console here. This change reduces the game's graphics memory pressure;
it does not claim to repair an unconfirmed allocation leak.

## Why this exists

The PS5 port's native allocator and Vulkan RADV driver consume the same
~12 GiB direct-memory pool. A rendering scale of 2x/3x/4x and a 4K output
can reserve much more graphics memory than native settings. The port also
normally keeps GPU-written textures until graphics memory is critically
short, to avoid expensive GPU downloads. This works well for speed but
risks abrupt OOM in games that stream many assets.

Fast RMX is Title ID `01009510001CA000` (Nintendo Switch edition).
For that title *only* (Vulkan or OpenGL), the low-memory profile applies
at game launch:

- Limits internal rendering to **1x** and output to **1080p** if the player
  selected a higher value. Lower selections are preserved.
- Uses the **conservative VRAM budget** instead of the port's normally
  aggressive/high budget. The Vulkan caches are encouraged to evict
  before the direct-memory pool approaches exhaustion.
- Lets the texture collector evict GPU-written images at the expected
  memory mark, instead of retaining them until the critical mark; this
  can increase stutter but reduce peak retained textures.
- Turns **frame generation off** for this game, including if globally
  enabled or enabled by an app-side `framegen.txt`.
- Writes a log line `EDEN_RMX_MEMORY` about every 5 seconds of rendered
  gameplay, including frame number, largest free direct-memory block in
  MiB, and whether the collector sees memory pressure.

The per-game policy takes effect without rewriting the user's saved
settings; switching to a different game restores the normal dirty-image
collector and VRAM budget on the next launch. Any developer `gc_dirty`
override still takes priority in development builds.

## A/B testing

Enable low-memory mode (default): no `fast_rmx` line required.
To disable **all** of this experimental profile, put the following line
in `compatibility.txt` beside `eboot.bin` in the application directory:

```text
fast_rmx=off
```

Set `fast_rmx=on` or remove the line to enable it again. Restart the
application before trying the alternative. Keep the game's mods disabled
for the initial test; cheats and other mods can change memory behavior.

Use a legally dumped Switch game with your own compatible firmware
and keys. Before trying it, record the game version and make sure
your current save data is backed up.

## How to tell whether it worked

In `/data/prosperoeden/logs/`, find the latest `stderr.log`,
`eden_log.txt`, or crash report:

- `compatibility: Fast RMX: low-memory mode...`: the profile was applied
- `EDEN_RMX_MEMORY`: free-block headroom as the game runs
- `EDEN_VULKAN_TEXTURE_BUDGET`: texture usage and budget (periodic)
- `memory: largest free block` in a crash report: headroom at failure
- `Direct allocation failed`, `bad_alloc` or Vulkan OOM: confirm the
  failing allocator; an OOM in the guest OS is a different issue

Compare free-block readings on a cold launch and after multiple game
sessions **without closing ProsperoEden**. A steady drop across sessions
even at 1x suggests retained memory needs investigation independently of
this per-game mitigation.

**Limitations:** This is not a verified fix. Conservative eviction can
cause pauses and may not avoid memory fragmentation, CPU heap exhaustion,
or leaked allocations from the driver or emulator. It also does not
automatically retry a failed game or change system memory limits.

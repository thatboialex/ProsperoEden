# In-game pause, cheats, and controller shortcuts (PS5)

The game menu now has four independent remappable two-button shortcuts in
**Settings → Controls**. Highlight an action and use Left/Right or Cross to
cycle the chord. If you choose another action's chord, they swap instead of
colliding. The choices are saved with your launcher's profile and take effect
on the next game launch.

| Action | Default |
| --- | --- |
| Leave game for library | Touchpad + L1 |
| Show/hide FPS counter | Touchpad + R1 |
| Pause and manage cheats | Touchpad + Options |
| Pause and adjust live graphics filters | Create + Options |

Other choices: Create + L1, Create + R1, Create + Options.

## Finding and editing all four shortcuts

Open **Settings → Controls** (Cross). The right-side Settings overview shows
the exact saved Return, FPS, Pause/Cheats, and Live graphics combinations.
The expanded Controls screen includes **six visible rows** (Vibration, Button
mapping and four editable actions). Highlight one of the shortcut rows and
press Left/Right or Cross to cycle it; choosing an occupied chord swaps
its assignment rather than creating conflicting shortcuts. Restart your
running game to pick up the saved changes.

You can also navigate **Library → Game settings → Controls** and select
**Pause / Cheats shortcut** or **Live graphics shortcut** to open the same
global editor, returning to your game-specific settings with Circle.

### Live graphics while a game is running

Default: **Create + Options** (hold Create, then press Options). The game
pauses and opens a native PS5 numeric dialog. Choose:

- **1** — Bilinear upscaling
- **2** — AMD FSR
- **3** — Bicubic
- **4** — Nearest neighbor
- **0** or cancel — resume gameplay

The choice changes the **Vulkan presentation filter during the current
session**, without restarting the game. It does **not** change the
internal-render target resolution or rebuild its texture caches. The
Vulkan renderer notices and recreates the presentation pass on the next
frame. Use **Settings → Video** or **Library → Game settings → Video** for
internal resolution, output resolution, backend, or other changes that need
a game restart. Live filter selection isn't persisted to your settings.

Keep **Faster GPU emulation**, **Faster CPU emulation**, and **Faster DMA**
off when checking cheat reliability: the current Mario + Rabbids screenshot
shows 99,999 coins with those options off and Reactive flushing on. This is
a correlation, not yet proof which of those settings caused prior issues.

## How to change cheats without restarting

First place your cheats in the supported game's mods folder, enable the mod
under **Library → Game Settings → Mods**, and start the game. The module's
running Build ID must match the cheat file if the file uses a Build ID name.

During gameplay press the configured **Pause / cheats** chord. The emulated
CPU and kernel pause, and the PS5 native text-entry dialog opens as the
first console-safe in-game selector. It shows one loaded cheat at a time with
its current ON/OFF state. On the number pad enter one digit and confirm:

- **1** toggles the shown cheat immediately and saves the selection.
- **2** shows the next cheat.
- **3** shows the previous cheat.
- **0**, or cancel, resumes gameplay.

No game restart is needed to turn a **loaded** cheat off and back on. A
different cheat file or a previously disabled mod that was not loaded still
requires one restart. The native PS5 dialog is a simple initial selector,
not a custom Vulkan-drawn overlay; it avoids creating a second graphics
renderer while the game owns the swapchain.

Switching a grouped cheat also respects the existing mutually exclusive
group rules (for example, incompatible frame-rate options). Single-cheat
files toggle as a unit. The program of the running guest cheat VM is
reloaded while the guest is paused instead of destroying its scheduler
event. The next timing callback executes the newly enabled instructions.

## Mario + Rabbids - what the uploaded logs actually establish

The selected `cheat_99999_coins_and_bananas.txt` was found and ran under
main Build ID `3B39E0C06B8841F1`, but the console reported **zero successful
writes** after 1,320 cheat VM cycles, including 1,320 rejected writes and
last rejected guest address `0x10`. This is a failed/unresolved pointer
chain for that running game state, not a failure to open the file or parse
its opcodes. The emulator refuses writes to invalid guest addresses to
prevent crashes.

The in-game menu makes loading and toggling reliable, but it **cannot**
reconstruct an outdated game's pointer offsets or guarantee that a cheat
written for another game update or scene will work. To fix that particular
coin value, the code itself must be checked against the memory layout of the
actual game/update and game mode, then retested on console. Do not
replace the rejected `0x10` with a guessed address.

`/data/prosperoeden/logs/stderr.log` still reports `cheat-vm`
enabled/opcode/read/write/rejection counters. After toggling, expect
`cheats: live selection updated` and the next `cheat-vm` report;
a write count above zero is proof of guest memory modification but not
proof that the intended in-game value changed.

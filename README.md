# ProsperoEden experimental Smash 13.0.4 downloads

[Download the PS5 emulator](./ProsperoEden-smash-diagnostics-702c38c.zip?raw=true)  
[Download Skyline + ARCropolis for Smash 13.0.4](./prosperoeden-smash-13.0.4-frameworks-experimental.zip?raw=true)

Close ProsperoEden and copy the emulator ZIP's `PPSA99008` folder to `/data/homebrew/PPSA99008`.
The framework ZIP contains `data/prosperoeden/mods`; copy `mods` into the selected Game files folder
(default `/data/prosperoeden`). Enable Skyline in Smash's Mods screen. ARCropolis v4.0.8 targets
Smash 13.0.4. No games, firmware, keys, saves or character packs are included.

Automatic diagnostic logs are under `/data/prosperoeden/logs/`. See [DIAGNOSTICS.txt](DIAGNOSTICS.txt)
for engine/ARCropolis logs, collection instructions, retention and limitations.

Guest-fault, journal, inventory, collector, native build and package checks passed.
Actual PS5 startup, modded gameplay and PS5 crash collection remain unverified.

[Native source at build revision](https://github.com/thatboialex/ProsperoEden/tree/702c38cc593f1529d154dc67eba83b21fcc6ac10)
and [collector fixes](https://github.com/thatboialex/ProsperoEden/commit/aea09ce).
Verify downloaded ZIPs against [SHA256SUMS](SHA256SUMS).

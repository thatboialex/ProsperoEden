// SPDX-License-Identifier: GPL-3.0-or-later
// Cheats chosen one by one. Eden runs every cheat of every mod that is on; a mod that lists
// several (mods.h) has each of them chosen in the launcher instead (Library > Game settings >
// Mods), and the ones not chosen are named here before the game starts. The derived
// patch_manager.cpp (headless/CMakeLists.txt) passes each mod's cheat list through Append(),
// which leaves those in the list but switched off: Eden's cheat machine skips them.
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>
#include "core/memory/dmnt_cheat_types.h"

namespace Eden::Cheats {
// A cheat as both sides name it: its mod, then its own name without the blanks around it. Eden
// keeps the first 63 bytes of a name, so no more than that is compared.
inline std::string Key(std::string_view mod, std::string_view cheat) {
    while (!cheat.empty() && std::isspace(static_cast<unsigned char>(cheat.front()))) cheat.remove_prefix(1);
    while (!cheat.empty() && std::isspace(static_cast<unsigned char>(cheat.back()))) cheat.remove_suffix(1);
    cheat = cheat.substr(0, 60);
    while (!cheat.empty() && std::isspace(static_cast<unsigned char>(cheat.back()))) cheat.remove_suffix(1);
    return std::string(mod) + "#" + std::string(cheat);
}

// The cheats that stay off in the game about to start (main.cpp fills it before Eden boots).
inline std::vector<std::string>& Off() {
    static std::vector<std::string> off;
    return off;
}

inline bool Runs(std::string_view mod, std::string_view cheat) {
    const auto& off = Off();
    return std::find(off.begin(), off.end(), Key(mod, cheat)) == off.end();
}

// The loader reads each module separately. Capture only the MAIN module's original
// (unfiltered) entries so the game can safely rebuild the cheat VM without reloading
// the game; the disabled flags of each cheat are re-evaluated each time.
struct LiveEntry {
    Core::Memory::CheatEntry entry{};
    std::string key;
    std::string mod;
    bool switchable = false;
};
inline std::vector<LiveEntry>& Pending() {
    static std::vector<LiveEntry> list;
    return list;
}
inline std::vector<LiveEntry>& Main() {
    static std::vector<LiveEntry> list;
    return list;
}
inline void BeginCollection() { Pending().clear(); }
inline void KeepMain() { Main() = Pending(); }
inline void ClearMain() { Main().clear(); Pending().clear(); }
inline std::vector<Core::Memory::CheatEntry> Rebuild() {
    std::vector<Core::Memory::CheatEntry> entries;
    entries.reserve(Main().size());
    for (const auto& source : Main()) {
        auto entry = source.entry;
        if (source.switchable && entry.enabled)
            entry.enabled = std::find(Off().begin(), Off().end(), source.key) == Off().end();
        entries.push_back(std::move(entry));
    }
    return entries;
}

// A mod's cheat list as Eden parsed it, added to the game's: the first entry is the file's
// master code, which always stays.
template <typename Entries>
void Append(Entries& out, const Entries& list, std::string_view mod) {
    for (auto entry : list) {
        const bool switchable = entry.cheat_id != 0;
        const std::string key = Key(mod, entry.definition.readable_name.data());
        Pending().push_back({entry, key, std::string(mod), switchable});
        if (switchable && entry.enabled) entry.enabled = Runs(mod, entry.definition.readable_name.data());
        out.push_back(entry);
    }
}
} // namespace Eden::Cheats

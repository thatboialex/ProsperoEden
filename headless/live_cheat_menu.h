// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// In-game cheat selector using the PS5's own on-screen dialog. The guest OS
// stays paused while the native dialog receives the controller. Unlike a
// launcher-only cheat switch, this reloads the running VM before resuming.
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <string>
#include <vector>
#include "cheats.h"
#include "devices.h"
#include "mods.h"
#include "settings_store.h"
#include "system_keyboard.h"
#include "diagnostics.h"
#include "core/core.h"
#include "common/scope_exit.h"

namespace Eden {
inline std::u16string CheatDialogText(const std::string& value) {
    std::u16string out;
    for (unsigned char c : value) out.push_back(c < 128 && c >= 32 ? char16_t(c) : u'?');
    return out;
}

inline void ShowPausedCheats(Core::System& system, Pad& pad, std::uint64_t title_id,
                             const std::atomic<bool>& stopped) {
    if (!system.IsPoweredOn() || stopped.load()) return;
    pad.SetCheatMenuOpen(true);
    system.Pause();
    SCOPE_EXIT {
        if (system.IsPoweredOn() && system.IsPaused()) system.Run();
        pad.SetCheatMenuOpen(false);
    };
    const auto all_mods = Mods::List(AssetsPath("mods"), title_id);
    std::vector<std::size_t> available;
    const auto& loaded = Cheats::Main();
    for (std::size_t i = 0; i < loaded.size(); ++i)
        if (loaded[i].switchable) available.push_back(i);
    std::size_t selected = 0;
    bool just_changed = false;
    while (!stopped.load()) {
        TextRequest request;
        request.max_length = 1;
        request.numbers = true;
        std::string title;
        if (available.empty()) {
            title = "PAUSED - No loaded cheats. Enable a cheat mod in Library and restart. 0 Resume";
        } else {
            const auto& item = loaded[available[selected]];
            const bool enabled = std::find(Cheats::Off().begin(), Cheats::Off().end(), item.key) == Cheats::Off().end();
            const std::string name = item.entry.definition.readable_name.data();
            title = "PAUSED " + std::to_string(selected + 1) + "/" + std::to_string(available.size()) +
                    (enabled ? " [ON] " : " [OFF] ") + name.substr(0, 56);
        }
        if (just_changed) title = "Saved. " + title;
        request.title = CheatDialogText(title.substr(0, 124));
        request.placeholder = CheatDialogText(available.empty()
            ? "0 Resume" : "1 Toggle  2 Next  3 Previous  0 Resume");
        const TextAnswer answer = AskSystemKeyboard(request, stopped);
        if (answer.outcome != TextOutcome::accepted || answer.text.empty() || answer.text[0] == u'0')
            break;
        just_changed = false;
        if (available.empty()) continue;
        const char16_t command = answer.text[0];
        if (command == u'2') { selected = (selected + 1) % available.size(); continue; }
        if (command == u'3') { selected = (selected + available.size() - 1) % available.size(); continue; }
        if (command != u'1') continue;
        const auto& target = loaded[available[selected]];
        const bool enabled = std::find(Cheats::Off().begin(), Cheats::Off().end(), target.key) == Cheats::Off().end();
        auto chosen = LoadChosenCheats(title_id);
        auto disabled = LoadDisabledMods(title_id);
        bool saved = false;
        const auto mod = std::find_if(all_mods.begin(), all_mods.end(), [&](const Mods::Mod& m) {
            return m.name == target.mod;
        });
        if (mod != all_mods.end() && !mod->cheats.empty()) {
            chosen = Mods::ChooseCheat(*mod, target.entry.definition.readable_name.data(), !enabled, chosen);
            saved = SaveChosenCheats(title_id, chosen);
        } else {
            const auto it = std::find(disabled.begin(), disabled.end(), target.mod);
            if (enabled) {
                if (it == disabled.end()) disabled.push_back(target.mod);
            } else if (it != disabled.end()) {
                disabled.erase(it);
            }
            saved = SaveDisabledMods(title_id, disabled);
        }
        if (!saved) {
            Report("cheats", "Could not save in-game selection; leaving running cheat engine unchanged");
            continue;
        }
        auto off = Mods::CheatsOff(all_mods, chosen);
        for (const auto& item : loaded) {
            if (std::find(disabled.begin(), disabled.end(), item.mod) != disabled.end() &&
                std::find(off.begin(), off.end(), item.key) == off.end())
                off.push_back(item.key);
        }
        Cheats::Off() = std::move(off);
        system.RegisterCheatList(Cheats::Rebuild(), system.GetApplicationProcessBuildID(), 0, 0);
        just_changed = true;
        std::fprintf(stderr, "[ProsperoEden] cheats: live selection updated for %s (requested=%s)\n",
                     target.key.c_str(), enabled ? "off" : "on");
    }
}
} // namespace Eden

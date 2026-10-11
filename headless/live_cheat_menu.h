// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// True Eden-style in-game cheat manager: D-pad navigates names; Cross directly
// switches the focused code; Circle resumes. No numbered keyboard dialog.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include "cheats.h"
#include "devices.h"
#include "mods.h"
#include "settings_store.h"
#include "diagnostics.h"
#include "in_game_overlay.h"
#include "core/core.h"
#include "common/scope_exit.h"

namespace Eden {
inline void ShowPausedCheats(Core::System& system, Pad& pad, GraphicsBackend backend, std::uint64_t title_id,
                             const std::atomic<bool>& stopped) {
    if (!system.IsPoweredOn() || stopped.load()) return;
    if (backend != GraphicsBackend::Vulkan) {
        Report("cheats", "EDEN_CHEAT_MENU: native styled overlay requires Vulkan");
        return;
    }
    const auto all_mods = Mods::List(AssetsPath("mods"), title_id);
    const auto& loaded = Cheats::Main();
    std::vector<std::size_t> available;
    std::vector<InGameOverlay::Row> rows;
    for (std::size_t i = 0; i < loaded.size(); ++i) {
        if (!loaded[i].switchable) continue;
        available.push_back(i);
        const auto& entry = loaded[i];
        const bool enabled = std::find(Cheats::Off().begin(), Cheats::Off().end(), entry.key)
                             == Cheats::Off().end();
        rows.push_back({entry.entry.definition.readable_name.data(), enabled});
    }
    if (rows.empty()) rows.push_back({"NO CHEATS LOADED FOR THIS GAME", false});
    pad.SetCheatMenuOpen(true);
    system.Pause();
    InGameOverlay::Open(InGameOverlay::Mode::cheats, std::move(rows));
    Report("cheats", "EDEN_CHEAT_MENU_OPEN controller UI active; guest paused");
    SCOPE_EXIT {
        InGameOverlay::Close();
        pad.SetCheatMenuOpen(false);
        if (system.IsPoweredOn() && system.IsPaused()) system.Run();
        Report("cheats", "EDEN_CHEAT_MENU_CLOSE");
    };

    while (!stopped.load() && system.IsPoweredOn()) {
        // Poll here because the normal gameplay loop waits for this function.
        // The controller adapter intercepts commands before forwarding anything
        // to the emulated Switch controller.
        pad.Poll();
        const Pad::MenuKey key = pad.TakeMenuKey();
        if (key == Pad::MenuKey::back) break;
        if (key == Pad::MenuKey::up) InGameOverlay::Move(-1);
        if (key == Pad::MenuKey::down) InGameOverlay::Move(1);
        if (key == Pad::MenuKey::page_up) InGameOverlay::Move(-InGameOverlay::kVisibleRows);
        if (key == Pad::MenuKey::page_down) InGameOverlay::Move(InGameOverlay::kVisibleRows);
        if (key == Pad::MenuKey::accept && !available.empty()) {
            const int selected = InGameOverlay::Selected();
            if (selected < 0 || selected >= int(available.size())) continue;
            const auto& target = loaded[available[std::size_t(selected)]];
            const bool currently_enabled =
                std::find(Cheats::Off().begin(), Cheats::Off().end(), target.key) == Cheats::Off().end();
            auto chosen = LoadChosenCheats(title_id);
            auto disabled = LoadDisabledMods(title_id);
            bool saved = false;
            const auto mod = std::find_if(all_mods.begin(), all_mods.end(), [&](const Mods::Mod& m) {
                return m.name == target.mod;
            });
            if (mod != all_mods.end() && !mod->cheats.empty()) {
                chosen = Mods::ChooseCheat(*mod, target.entry.definition.readable_name.data(),
                                           !currently_enabled, chosen);
                saved = SaveChosenCheats(title_id, chosen);
            } else {
                const auto it = std::find(disabled.begin(), disabled.end(), target.mod);
                if (currently_enabled) {
                    if (it == disabled.end()) disabled.push_back(target.mod);
                } else if (it != disabled.end()) {
                    disabled.erase(it);
                }
                saved = SaveModEnabled(title_id, target.mod, !currently_enabled);
            }
            if (!saved) {
                Report("cheats", "EDEN_CHEAT_MENU_SAVE_FAILED; live VM unchanged");
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
            // May have mutually exclusive groups; refresh *all* rows, not just
            // the focused one, after loading the new VM instructions.
            for (std::size_t i = 0; i < available.size(); ++i) {
                const auto& item = loaded[available[i]];
                InGameOverlay::SetEnabled(int(i),
                    std::find(Cheats::Off().begin(), Cheats::Off().end(), item.key) ==
                        Cheats::Off().end());
            }
            std::fprintf(stderr, "[ProsperoEden] EDEN_CHEAT_MENU_TOGGLE name=%s enabled=%d\n",
                         target.key.c_str(), !currently_enabled);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
}
} // namespace Eden

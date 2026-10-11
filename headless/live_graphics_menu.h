// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Eden-themed game graphics menu. The running Vulkan renderer can safely
// reconfigure its presentation filter without rebuilding texture render targets.
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include "common/scope_exit.h"
#include "common/settings.h"
#include "core/core.h"
#include "devices.h"
#include "diagnostics.h"
#include "in_game_overlay.h"
#include "settings_store.h"

namespace Eden {
inline ::Settings::ScalingFilter LiveFilterValue(int index) {
    switch (index) {
    case 0: return ::Settings::ScalingFilter::Bilinear;
    case 1: return ::Settings::ScalingFilter::Fsr;
    case 2: return ::Settings::ScalingFilter::Bicubic;
    default: return ::Settings::ScalingFilter::NearestNeighbor;
    }
}
inline int LiveFilterIndex(::Settings::ScalingFilter value) {
    for (int i = 0; i < 4; ++i)
        if (LiveFilterValue(i) == value) return i;
    return 0;
}
inline void ShowPausedGraphics(Core::System& system, Pad& pad, GraphicsBackend backend,
                               const std::atomic<bool>& stopped) {
    if (!system.IsPoweredOn() || stopped.load()) return;
    if (backend != GraphicsBackend::Vulkan) {
        Report("graphics", "EDEN_GRAPHICS_MENU: live Vulkan filters are unavailable on OpenGL");
        return;
    }
    const int current = LiveFilterIndex(::Settings::values.scaling_filter.GetValue());
    pad.SetCheatMenuOpen(true);
    system.Pause();
    InGameOverlay::Open(InGameOverlay::Mode::graphics, {
        {"Bilinear", current == 0}, {"AMD FidelityFX FSR", current == 1},
        {"Bicubic", current == 2}, {"Nearest neighbor", current == 3}}, current);
    Report("graphics", "EDEN_GRAPHICS_MENU_OPEN guest paused; controller choices active");
    SCOPE_EXIT {
        InGameOverlay::Close();
        pad.SetCheatMenuOpen(false);
        if (system.IsPoweredOn() && system.IsPaused()) system.Run();
        Report("graphics", "EDEN_GRAPHICS_MENU_CLOSE");
    };
    while (!stopped.load() && system.IsPoweredOn()) {
        pad.Poll();
        const auto key = pad.TakeMenuKey();
        if (key == Pad::MenuKey::back) break;
        if (key == Pad::MenuKey::up) InGameOverlay::Move(-1);
        if (key == Pad::MenuKey::down) InGameOverlay::Move(1);
        if (key == Pad::MenuKey::page_up) InGameOverlay::Move(-1);
        if (key == Pad::MenuKey::page_down) InGameOverlay::Move(1);
        if (key == Pad::MenuKey::accept) {
            const int selection = InGameOverlay::Selected();
            ::Settings::values.scaling_filter.SetValue(LiveFilterValue(selection));
            InGameOverlay::SelectOnly(selection);
            Report("graphics", (std::string("EDEN_GRAPHICS_MENU_FILTER ") +
                kUpscalingFilterLabels[selection]).c_str());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
}
} // namespace Eden

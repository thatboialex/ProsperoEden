// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Small PS5 system dialog for LIVE, Vulkan-safe presentation-filter changes.
// This does not rebuild the GPU texture cache or claim to change internal resolution.
#include <atomic>
#include <string>
#include "common/scope_exit.h"
#include "common/settings.h"
#include "core/core.h"
#include "devices.h"
#include "diagnostics.h"
#include "settings_store.h"
#include "system_keyboard.h"
#ifdef EDEN_PS5_OPENGL
#include "hud.h"
#endif

namespace Eden {
inline Settings::ScalingFilter LiveFilterValue(int index) {
    switch (index) {
    case 0: return Settings::ScalingFilter::Bilinear;
    case 1: return Settings::ScalingFilter::Fsr;
    case 2: return Settings::ScalingFilter::Bicubic;
    default: return Settings::ScalingFilter::NearestNeighbor;
    }
}
inline int LiveFilterIndex(Settings::ScalingFilter value) {
    for (int i = 0; i < 4; ++i)
        if (LiveFilterValue(i) == value) return i;
    return 0;
}
inline std::u16string GraphicsDialogText(const std::string& ascii) {
    std::u16string s;
    for (unsigned char ch : ascii) s.push_back(ch >= 32 && ch < 127 ? char16_t(ch) : u'?');
    return s;
}
inline void ShowPausedGraphics(Core::System& system, Pad& pad, GraphicsBackend backend,
                               const std::atomic<bool>& stopped) {
    if (!system.IsPoweredOn() || stopped.load()) return;
    pad.SetCheatMenuOpen(true); // all gameplay input goes to the PS5 system keyboard
    system.Pause();
    Report("graphics", "LIVE_GRAPHICS_V1: guest paused, Vulkan filter selector opened");
    SCOPE_EXIT {
        if (system.IsPoweredOn() && system.IsPaused()) system.Run();
        pad.SetCheatMenuOpen(false);
        Report("graphics", "LIVE_GRAPHICS_V1: resume");
    };
    bool changed = false;
    while (!stopped.load()) {
        TextRequest request;
        request.numbers = true;
        request.max_length = 1;
        const bool supported = backend == GraphicsBackend::Vulkan;
        const int current = LiveFilterIndex(Settings::values.scaling_filter.GetValue());
        std::string title = supported ?
            std::string("PAUSED - Live filter: ") + kUpscalingFilterLabels[current] :
            "PAUSED - Live filter changes require Vulkan";
        if (changed) title = "Applied. " + title;
        request.title = GraphicsDialogText(title);
        request.placeholder = GraphicsDialogText(supported ?
            "1 Bilinear 2 FSR 3 Bicubic 4 Nearest 0 Resume" :
            "0 Resume - set Vulkan under Video to use live filters");
        const TextAnswer answer = AskSystemKeyboard(request, stopped);
        if (answer.outcome == TextOutcome::unavailable) {
            Report("graphics", "LIVE_GRAPHICS_V1: PS5 keyboard unavailable");
            break;
        }
        if (answer.outcome != TextOutcome::accepted || answer.text.empty() || answer.text[0] == u'0')
            break;
        if (!supported) continue;
        const char16_t ch = answer.text[0];
        if (ch < u'1' || ch > u'4') continue;
        const int index = int(ch - u'1');
        // The Vulkan present layer reads the filter on each render and rebuilds
        // the window-adapt pipeline if it changed. Unlike resolution scaling,
        // the underlying guest render targets do not need reallocation.
        Settings::values.scaling_filter.SetValue(LiveFilterValue(index));
        Report("graphics", (std::string("LIVE_GRAPHICS_V1: present filter -> ") +
                             kUpscalingFilterLabels[index]).c_str());
        changed = true;
    }
}
} // namespace Eden

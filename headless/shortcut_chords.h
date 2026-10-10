// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cstddef>
namespace Eden {
// 4 actions: return to library, FPS HUD, pause/cheat menu, live graphics menu.
// Each chord uses two non-system DualSense buttons; duplicates swap actions.
inline constexpr const char* kShortcutLabels[] = {
    "Touchpad + L1", "Touchpad + R1", "Touchpad + Options",
    "Create + L1", "Create + R1", "Create + Options"};
inline constexpr const char* kShortcutKeys[] = {
    "touch_l1", "touch_r1", "touch_options", "create_l1", "create_r1", "create_options"};
inline constexpr const char* kShortcutActionKeys[] = {"menu", "hud", "pause", "graphics"};
inline constexpr std::array<int, 4> kDefaultShortcuts{0, 1, 2, 5};
inline constexpr int kShortcutActionCount = static_cast<int>(kDefaultShortcuts.size());
inline bool ValidShortcuts(const std::array<int, 4>& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] < 0 || value[i] >= int(std::size(kShortcutLabels))) return false;
        for (std::size_t j = 0; j < i; ++j) if (value[i] == value[j]) return false;
    }
    return true;
}
inline std::array<int, 4> AssignShortcut(std::array<int, 4> value, int action, int chord) {
    if (action < 0 || action >= kShortcutActionCount || chord < 0 || chord >= int(std::size(kShortcutLabels)))
        return value;
    for (int i = 0; i < kShortcutActionCount; ++i)
        if (i != action && value[i] == chord) value[i] = value[action];
    value[action] = chord;
    return value;
}
} // namespace Eden

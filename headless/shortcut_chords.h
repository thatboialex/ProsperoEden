// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>

namespace Eden {
// Two-button chords prevent commonly used gameplay actions (L3/R3, triggers)
// from accidentally opening a menu. Original six indices and storage keys stay
// unchanged so older saved controls migrate without any conversion.
enum class ShortcutButton : unsigned char {
    touchpad, create, options, l1, r1, l2, r2, l3, r3
};
struct ShortcutChord {
    const char* label;
    const char* key;
    ShortcutButton first;
    ShortcutButton second;
};
inline constexpr std::array<ShortcutChord, 27> kShortcutChords{{
    {"Touchpad + L1", "touch_l1", ShortcutButton::touchpad, ShortcutButton::l1},
    {"Touchpad + R1", "touch_r1", ShortcutButton::touchpad, ShortcutButton::r1},
    {"Touchpad + Options", "touch_options", ShortcutButton::touchpad, ShortcutButton::options},
    {"Create + L1", "create_l1", ShortcutButton::create, ShortcutButton::l1},
    {"Create + R1", "create_r1", ShortcutButton::create, ShortcutButton::r1},
    {"Create + Options", "create_options", ShortcutButton::create, ShortcutButton::options},
    {"Touchpad + L2", "touch_l2", ShortcutButton::touchpad, ShortcutButton::l2},
    {"Touchpad + R2", "touch_r2", ShortcutButton::touchpad, ShortcutButton::r2},
    {"Touchpad + L3", "touch_l3", ShortcutButton::touchpad, ShortcutButton::l3},
    {"Touchpad + R3", "touch_r3", ShortcutButton::touchpad, ShortcutButton::r3},
    {"Create + L2", "create_l2", ShortcutButton::create, ShortcutButton::l2},
    {"Create + R2", "create_r2", ShortcutButton::create, ShortcutButton::r2},
    {"Create + L3", "create_l3", ShortcutButton::create, ShortcutButton::l3},
    {"Create + R3", "create_r3", ShortcutButton::create, ShortcutButton::r3},
    {"L1 + R1", "l1_r1", ShortcutButton::l1, ShortcutButton::r1},
    {"L2 + R2", "l2_r2", ShortcutButton::l2, ShortcutButton::r2},
    {"L3 + R3", "l3_r3", ShortcutButton::l3, ShortcutButton::r3},
    {"L1 + L3", "l1_l3", ShortcutButton::l1, ShortcutButton::l3},
    {"R1 + R3", "r1_r3", ShortcutButton::r1, ShortcutButton::r3},
    {"L2 + L3", "l2_l3", ShortcutButton::l2, ShortcutButton::l3},
    {"R2 + R3", "r2_r3", ShortcutButton::r2, ShortcutButton::r3},
    {"L1 + L2", "l1_l2", ShortcutButton::l1, ShortcutButton::l2},
    {"R1 + R2", "r1_r2", ShortcutButton::r1, ShortcutButton::r2},
    {"L1 + R2", "l1_r2", ShortcutButton::l1, ShortcutButton::r2},
    {"L2 + R1", "l2_r1", ShortcutButton::l2, ShortcutButton::r1},
    {"L3 + R1", "l3_r1", ShortcutButton::l3, ShortcutButton::r1},
    {"R3 + L1", "r3_l1", ShortcutButton::r3, ShortcutButton::l1},
}};
// Single source of truth for labels and persistent identifiers.
inline constexpr auto kShortcutLabels = [] {
    std::array<const char*, kShortcutChords.size()> a{};
    for (std::size_t i = 0; i < a.size(); ++i) a[i] = kShortcutChords[i].label;
    return a;
}();
inline constexpr auto kShortcutKeys = [] {
    std::array<const char*, kShortcutChords.size()> a{};
    for (std::size_t i = 0; i < a.size(); ++i) a[i] = kShortcutChords[i].key;
    return a;
}();
inline constexpr const char* kShortcutActionKeys[] = {"menu", "hud", "pause", "graphics"};
inline constexpr std::array<int, 4> kDefaultShortcuts{0, 1, 2, 5};
inline constexpr int kShortcutActionCount = int(kDefaultShortcuts.size());
inline constexpr bool ValidShortcuts(const std::array<int, 4>& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] < 0 || value[i] >= int(kShortcutChords.size())) return false;
        for (std::size_t j = 0; j < i; ++j)
            if (value[i] == value[j]) return false;
    }
    return true;
}
inline constexpr std::array<int, 4> AssignShortcut(std::array<int, 4> value,
                                                   int action, int chord) {
    if (action < 0 || action >= kShortcutActionCount ||
        chord < 0 || chord >= int(kShortcutChords.size())) return value;
    for (int i = 0; i < kShortcutActionCount; ++i)
        if (i != action && value[i] == chord) value[i] = value[action];
    value[action] = chord;
    return value;
}
static_assert(ValidShortcuts(kDefaultShortcuts));
static_assert(kShortcutChords[0].first == ShortcutButton::touchpad);
static_assert(kShortcutChords[16].second == ShortcutButton::r3);
static_assert(AssignShortcut(kDefaultShortcuts, 2, 16)[2] == 16);
} // namespace Eden

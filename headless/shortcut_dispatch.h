// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include "shortcut_chords.h"
namespace Eden {
template <class Mask>
struct ShortcutEvent {
    int action = -1;
    Mask consumed{};
    bool activated = false;
};
// Modal actions take priority over Return when a player holds overlapping chords.
template <class Mask, std::size_t N>
constexpr ShortcutEvent<Mask> DispatchShortcut(
    const std::array<int, 4>& choices, const std::array<Mask, N>& chords,
    Mask pressed, Mask previous) {
    for (int action : {2, 3, 0, 1}) {
        if (choices[action] < 0 || std::size_t(choices[action]) >= N) continue;
        const Mask mask = chords[choices[action]];
        if (mask != 0 && (pressed & mask) == mask)
            return {action, mask, (previous & mask) != mask};
    }
    return {};
}
} // namespace Eden

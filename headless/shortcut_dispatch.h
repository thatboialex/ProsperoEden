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
// Pause takes priority over Return when a player holds overlapping chords.
template <class Mask>
constexpr ShortcutEvent<Mask> DispatchShortcut(
    const std::array<int, 3>& choices, const std::array<Mask, 6>& chords,
    Mask pressed, Mask previous) {
    for (int action : {2, 0, 1}) {
        const Mask mask = chords[choices[action]];
        if ((pressed & mask) == mask)
            return {action, mask, (previous & mask) != mask};
    }
    return {};
}
} // namespace Eden

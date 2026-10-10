// SPDX-License-Identifier: GPL-3.0-or-later
#include "shortcut_chords.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace Eden;
    assert(ValidShortcuts(kDefaultShortcuts));
    auto bindings = AssignShortcut(kDefaultShortcuts, 2, 0);
    assert(bindings[0] == 2 && bindings[1] == 1 && bindings[2] == 0);
    assert(ValidShortcuts(bindings)); // setting Pause=Return swaps rather than duplicates.
    bindings = AssignShortcut(bindings, 1, 5);
    assert(bindings[1] == 5 && ValidShortcuts(bindings));
    const auto unchanged = AssignShortcut(bindings, 1, 100);
    assert(unchanged == bindings);
    assert(!ValidShortcuts({0, 0, 2}));
    assert(!ValidShortcuts({0, 1, 6}));
    std::puts("controller shortcut selection, collision and validation: PASS");
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "controls_menu_layout.h"
#include "shortcut_chords.h"
#include <cassert>
#include <iostream>
#include <string>
int main() {
    using namespace Eden::ControlsMenu;
    assert(Fits());
    assert(kRowCount == 6);
    assert(RowTop(0) >= kHeaderBottom);
    assert(RowTop(kRowCount-1) + kRowHeight < kFooterTop - 12.0f);
    for (int i = 1; i < kRowCount; ++i)
        assert(RowTop(i) >= RowTop(i-1) + kRowHeight);
    assert(Eden::ValidShortcuts(Eden::kDefaultShortcuts));
    assert(Eden::kDefaultShortcuts[2] == 2);
    assert(Eden::kDefaultShortcuts[3] == 5);
    assert(std::string(Eden::kShortcutLabels[2]) == "Touchpad + Options");
    const auto changed = Eden::AssignShortcut(Eden::kDefaultShortcuts, 2, 0);
    assert(changed[0] == 2 && changed[2] == 0 && Eden::ValidShortcuts(changed));
    std::cout << "PASS visible controls geometry and pause shortcut wiring\n";
}

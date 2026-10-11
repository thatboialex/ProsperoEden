// SPDX-License-Identifier: GPL-3.0-or-later
// Host-only regression test of the actual in-game menu model and rendering data.
#include "in_game_overlay.h"
#include "shortcut_chords.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace Eden::InGameOverlay;
    assert(!IsOpen());
    Open(Mode::cheats, {{"99999 coins and bananas", true}, {"All maps", false},
                       {"Infinite ammo", true}}, 0);
    assert(IsOpen());
    auto frame = Frame();
    assert(frame.background.loading == 2 && frame.background.width == 1920);
    assert(frame.background.glyphs[0] == 0); // cheat mode
    assert(frame.background.glyphs[2] == 3);
    assert(frame.background.glyphs[3] == 5); // first and third enabled
    assert(frame.background.glyphs[6] == 7); // every cheat has a switch
    assert(frame.count >= 5 && frame.count <= kMaxTextLines);
    assert(frame.text[0].loading == 3); // title
    assert(frame.text[2].loading == 4); // first focused code
    Move(1);
    assert(Selected() == 1);
    SetEnabled(1, true);
    frame = Frame();
    assert(frame.background.glyphs[1] == 1);
    assert(frame.background.glyphs[3] == 7);
    Move(-1);
    assert(Selected() == 0);
    Close();
    assert(!IsOpen() && Frame().background.width == 0);
    std::vector<Row> rows;
    for (int i = 0; i < 21; ++i)
        rows.push_back({"Cheat test " + std::to_string(i), i == 10});
    Open(Mode::cheats, rows, 10);
    frame = Frame();
    assert(frame.background.glyphs[4] == 7);  // second page
    assert(frame.background.glyphs[1] == 3);
    assert(frame.background.glyphs[3] == (1u << 3));
    assert(frame.count <= kMaxTextLines);
    assert(frame.background.glyphs[6] == 127); // seven real switches on page 2
    Close();
    Open(Mode::cheats, {{"NO CHEATS LOADED FOR THIS GAME", false, false}});
    frame = Frame();
    assert(frame.background.glyphs[2] == 1); // informational row is visible
    assert(frame.background.glyphs[6] == 0); // but no misleading switch is drawn
    Close();
    Open(Mode::graphics, {{"Bilinear", true}, {"FSR", false}}, 1);
    assert(Frame().background.glyphs[0] == 1);
    SelectOnly(1);
    assert(Frame().background.glyphs[3] == 2);
    assert(Frame().background.glyphs[6] == 3);
    Close();
    assert(Eden::ValidShortcuts({8, 9, 15, 16}));
    std::puts("native Eden in-game list, toggles, page scrolling and rendering state: PASS");
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Dependency-free check of the text passed to Eden's strict cheat parser.
#include "cheat_text.h"
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    const auto source = std::string("\xEF\xBB\xBF") +
        "// collection credits\r\n"
        "# author line\n"
        "[Direct #1] // trailing annotation\r\n"
        "04000000 00000010 0000002A ; write 42\n"
        "; a full-line annotation\n"
        "{Master} // another label\n"
        "04000000 00000020 00000001 # comment\n";
    const auto actual = Eden::Cheats::NormalizeText(source);
    const std::string expected =
        "[Direct #1]\n"
        "04000000 00000010 0000002A\n"
        "{Master}\n"
        "04000000 00000020 00000001\n";
    assert(actual == expected);

    // Never silently correct an invalid opcode or rename a labeled cheat.
    assert(Eden::Cheats::NormalizeText("[Patch // as named]\n0Xinvalid\n") ==
           "[Patch // as named]\n0Xinvalid\n");
    assert(Eden::Cheats::NormalizeText("") == "");
    std::puts("cheat text normalization: PASS");
}

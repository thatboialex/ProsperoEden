// SPDX-License-Identifier: GPL-3.0-or-later
// Host-side check that a loose cheat file parses and the write result is visible.
#include "cheat_runtime.h"
#include "patch_library.h"
#include <cassert>
#include <cstdio>

int main() {
    constexpr const char* example =
        "[17# 99999 Coins and bananas]\n"
        "580F0000 0330B038\n"
        "580F1000 00000068\n"
        "580F1000 00000050\n"
        "580F1000 00000198\n"
        "580F1000 00000348\n"
        "780F0000 00000010\n"
        "640F0000 00000000 0001869F\n";
    const auto cheats = Eden::Patches::ParseCheats(example, "cheat_99999_coins_and_bananas.txt");
    assert(cheats.size() == 1);
    assert(cheats.front().name == "17# 99999 Coins and bananas");
    assert(cheats.front().id ==
           "cheat_99999_coins_and_bananas.txt#17# 99999 Coins and bananas");
    Eden::CheatRuntime::Reset();
    Eden::CheatRuntime::Read(true, 0x80000000ULL);
    Eden::CheatRuntime::Read(false, 0ULL);
    Eden::CheatRuntime::Write(true, true, 0x80000020ULL);
    Eden::CheatRuntime::Write(true, false, 0x80000030ULL);
    Eden::CheatRuntime::Write(false, false, 0x80000040ULL);
    assert(Eden::CheatRuntime::reads.load() == 1);
    assert(Eden::CheatRuntime::rejected_reads.load() == 1);
    assert(Eden::CheatRuntime::writes.load() == 1);
    assert(Eden::CheatRuntime::failed_writes.load() == 1);
    assert(Eden::CheatRuntime::rejected_writes.load() == 1);
    assert(Eden::CheatRuntime::last_rejected_address.load() == 0x80000040ULL);
    Eden::CheatRuntime::Reset();
    assert(Eden::CheatRuntime::writes.load() == 0);
    std::puts("cheat file selection and VM write counters: PASS");
}

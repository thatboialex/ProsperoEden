// SPDX-License-Identifier: GPL-3.0-or-later
// Host smoke test: c++ -std=c++20 -Wall -Wextra game_compatibility_check.cpp -o /tmp/za_check
#include "game_compatibility.h"
#include <cassert>
#include <cstdio>

int main() {
    using namespace Eden::Compatibility;
    assert(ForGame(kPokemonLegendsZA, true).pokemon_za_workaround);
    assert(!ForGame(kPokemonLegendsZA, true).vertex_input_dynamic_state);
    assert(!ForGame(kPokemonLegendsZA, false).pokemon_za_workaround);
    assert(ForGame(kPokemonLegendsZA, false).vertex_input_dynamic_state);
    assert(!ForGame(kPokemonLegendsZA, true, false).pokemon_za_workaround);
    assert(ForGame(kPokemonLegendsZA, true, false).vertex_input_dynamic_state);
    assert(ForGame(0x0100000000001000ULL, true).vertex_input_dynamic_state);
    std::puts("per-game Vulkan compatibility: PASS");
}

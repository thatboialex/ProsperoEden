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
    // Fast RMX is the only title with conservative memory settings.
    auto rmx = ForFastRmx(kFastRmx);
    assert(rmx.enabled && rmx.resolution_ceiling == 2 && rmx.output_ceiling == 0);
    assert(rmx.early_dirty_eviction && rmx.disable_frame_generation && rmx.use_conservative_vram);
    assert(!ForFastRmx(kFastRmx, false).enabled);
    assert(!ForFastRmx(kPokemonLegendsZA).enabled);
    assert(!ForFastRmx(0x0100000000001000ULL).enabled);
    std::puts("per-game Vulkan compatibility: PASS");
}

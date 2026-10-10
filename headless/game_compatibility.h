// SPDX-License-Identifier: GPL-3.0-or-later
// Conservative Vulkan per-title compatibility overrides. No copyrighted game data.
#pragma once
#include <cstdint>

namespace Eden::Compatibility {
inline constexpr std::uint64_t kPokemonLegendsZA = 0x0100F43008C44000ULL;

struct VulkanProfile {
    bool vertex_input_dynamic_state = true;
    bool pokemon_za_workaround = false;
};

// On desktop RADV, Z-A rendering was corrected with EDS2 and dynamic vertex input
// disabled. main.cpp restores EDS2 and the default vertex setting each launch.
constexpr VulkanProfile ForGame(std::uint64_t title_id, bool use_vulkan, bool enabled = true) {
    const bool workaround = enabled && use_vulkan && title_id == kPokemonLegendsZA;
    return {!workaround, workaround};
}
static_assert(ForGame(kPokemonLegendsZA, true).pokemon_za_workaround);
static_assert(!ForGame(kPokemonLegendsZA, false).pokemon_za_workaround);
static_assert(ForGame(kPokemonLegendsZA, true, false).vertex_input_dynamic_state);
static_assert(ForGame(0x0100000000001000ULL, true).vertex_input_dynamic_state);
} // namespace Eden::Compatibility

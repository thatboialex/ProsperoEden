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

// Leave the proven EDS2/vertex-input workaround untouched. Shader compilation
// and 2x/3x internal resolution are separate causes of first-play frame spikes;
// allow the user to compare this conservative Vulkan performance profile.
struct PokemonZAPerformanceProfile {
    bool enabled = false;
    int resolution_ceiling = 2; // safety option; not forced by default.
    bool cap_resolution = false;
    bool asynchronous_shaders = false;
};
constexpr PokemonZAPerformanceProfile ForPokemonZAPerformance(std::uint64_t title_id,
                                                               bool use_vulkan,
                                                               bool enabled = true,
                                                               bool cap_resolution = false) {
    const bool active = enabled && use_vulkan && title_id == kPokemonLegendsZA;
    return {active, 2, active && cap_resolution, active};
}
static_assert(ForPokemonZAPerformance(kPokemonLegendsZA, true).enabled);
static_assert(!ForPokemonZAPerformance(kPokemonLegendsZA, false).enabled);
static_assert(!ForPokemonZAPerformance(kPokemonLegendsZA, true).cap_resolution);
static_assert(ForPokemonZAPerformance(kPokemonLegendsZA, true, true, true).cap_resolution);
static_assert(!ForPokemonZAPerformance(kPokemonLegendsZA, true, false).enabled);
static_assert(!ForPokemonZAPerformance(0x0100000000001000ULL, true).enabled);

// A residual Z-A bug can make background meshes pop in and out even after
// fixing vertex-input state. The PS5 Vulkan port already supports an exact
// CPU predicate path for host conditional rendering (hcr_mode=2). Keep it
// opt-in: readback/synchronization can be slower, and the cause is unproven.
struct PokemonZAPredicationProfile {
    bool cpu_predication = false;
    int hcr_mode = 1;
};
constexpr PokemonZAPredicationProfile ForPokemonZAPredication(std::uint64_t title_id,
                                                              bool use_vulkan,
                                                              bool cpu_requested) {
    const bool active = title_id == kPokemonLegendsZA && use_vulkan && cpu_requested;
    return {active, active ? 2 : 1};
}
static_assert(ForPokemonZAPredication(kPokemonLegendsZA, true, true).hcr_mode == 2);
static_assert(ForPokemonZAPredication(kPokemonLegendsZA, true, false).hcr_mode == 1);
static_assert(ForPokemonZAPredication(kPokemonLegendsZA, false, true).hcr_mode == 1);
static_assert(ForPokemonZAPredication(0x0100000000001000ULL, true, true).hcr_mode == 1);

// FAST RMX (original Switch release): avoid growing the shared PS5 CPU/GPU memory
// pool until allocations fail. Conservative cache budgets intentionally trade
// some performance for less resident graphics memory. This is a mitigation, not
// a demonstrated cure for a memory leak.
inline constexpr std::uint64_t kFastRmx = 0x01009510001CA000ULL;
struct FastRmxMemoryProfile {
    bool enabled = false;
    int resolution_ceiling = 2; // 1x in kResolutionKeys
    int output_ceiling = 0;     // 1080p in kOutputKeys
    bool early_dirty_eviction = false;
    bool disable_frame_generation = false;
    bool use_conservative_vram = false;
};
constexpr FastRmxMemoryProfile ForFastRmx(std::uint64_t title_id, bool enabled = true) {
    const bool active = enabled && title_id == kFastRmx;
    return {active, 2, 0, active, active, active};
}
static_assert(ForFastRmx(kFastRmx).enabled);
static_assert(ForFastRmx(kFastRmx).early_dirty_eviction);
static_assert(!ForFastRmx(kFastRmx, false).enabled);
static_assert(!ForFastRmx(kPokemonLegendsZA).enabled);
} // namespace Eden::Compatibility

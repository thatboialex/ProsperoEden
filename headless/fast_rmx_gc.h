// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>

namespace Eden::FastRmxGC {
inline constexpr std::uint64_t kPressureThreshold = 1024ULL << 20;

struct Policy {
    bool high_priority = false;
    bool aggressive = false;
    std::uint64_t age_frames = 50;
    std::size_t iterations = 10;
};

// Pure policy so its invariants are checked on Linux without PS5 dependencies.
// The normal games' limits match the upstream Eden collector. Fast RMX gets
// earlier and larger eviction passes before a new Vulkan image needs a block.
constexpr Policy Choose(bool rmx, bool allow_aggressive, std::uint64_t used,
                        std::uint64_t expected, std::uint64_t critical,
                        std::uint64_t largest_free) {
    const bool urgent = rmx && largest_free < kPressureThreshold;
    const bool high = used >= expected || urgent;
    const bool aggressive = allow_aggressive && (used >= critical || urgent);
    if (!rmx) return {high, aggressive, aggressive ? 10ULL : high ? 25ULL : 50ULL,
                      aggressive ? 40u : high ? 20u : 10u};
    return {high, aggressive, aggressive ? 10ULL : high ? 20ULL : 50ULL,
            aggressive ? 96u : high ? 48u : 10u};
}

constexpr bool NeedSecondPass(bool rmx, std::uint64_t used, std::uint64_t critical,
                              std::uint64_t largest_free) {
    return used >= critical || (rmx && largest_free < kPressureThreshold);
}
static_assert(Choose(false, true, 4, 2, 3, 0).iterations == 40);
static_assert(Choose(false, false, 4, 2, 3, 0).iterations == 20);
static_assert(Choose(true, true, 4, 2, 3, 0).iterations == 96);
static_assert(NeedSecondPass(true, 1, 3, 512ULL << 20));
static_assert(!NeedSecondPass(false, 1, 3, 512ULL << 20));
} // namespace Eden::FastRmxGC

// SPDX-License-Identifier: GPL-3.0-or-later
// Standalone host test: the emergency GC applies to Fast RMX only.
#include "fast_rmx_gc.h"
#include <cassert>
#include <cstdio>

int main() {
    using namespace Eden::FastRmxGC;
    constexpr std::uint64_t GiB = 1024ULL * 1024 * 1024;
    const auto normal = Choose(false, true, 4 * GiB, 2 * GiB, 3 * GiB, 256 * 1024 * 1024);
    assert(normal.aggressive && normal.age_frames == 10 && normal.iterations == 40);
    const auto pressure = Choose(true, false, 1 * GiB, 2 * GiB, 3 * GiB, GiB / 2);
    assert(pressure.high_priority && !pressure.aggressive && pressure.iterations == 48);
    const auto aggressive = Choose(true, true, 1 * GiB, 2 * GiB, 3 * GiB, GiB / 2);
    assert(aggressive.aggressive && aggressive.age_frames == 10 && aggressive.iterations == 96);
    const auto calm = Choose(true, true, GiB / 2, 2 * GiB, 3 * GiB, 2 * GiB);
    assert(!calm.high_priority && !calm.aggressive && calm.iterations == 10);
    assert(NeedSecondPass(true, GiB, 3 * GiB, GiB / 2));
    assert(!NeedSecondPass(false, GiB, 3 * GiB, GiB / 2));
    assert(!NeedSecondPass(true, GiB, 3 * GiB, 2 * GiB));
    std::puts("fast-rmx gc pressure policy: PASS");
}

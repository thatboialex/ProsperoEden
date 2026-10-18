// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "mod_diagnostics.h"
#include "performance.h"
#include <fmt/format.h>
namespace Eden {
// Main-thread snapshots only. All totals are cumulative, include overlapping work,
// and must be differenced over time; their sum is not a CPU or GPU percentage.
inline void SmashPerformanceSnapshot() {
    Performance::Snapshot(); // owner-written CPU samples, with timestamps and validity
    const auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::string text = fmt::format("mono_ns={} cumulative=true\n", now);
    const auto add = [&](const char* name, const Performance::Totals& totals) {
        text += fmt::format("{} calls={} ns={} bytes={}\n", name,
            totals.calls.load(std::memory_order_relaxed),
            totals.nanoseconds.load(std::memory_order_relaxed),
            totals.requested_bytes.load(std::memory_order_relaxed));
    };
    for (unsigned core = 0; core < 4; ++core)
        add(fmt::format("jit{}", core).c_str(), Performance::compilation[core]);
    add("storage", Performance::storage);
    add("draw", Performance::rasterizer_draw);
    add("gpu_dispatch", Performance::gpu_dispatch);
    add("gpu_queue_wait", Performance::gpu_queue_wait);
    add("gpu_fence_drain", Performance::gpu_fence_drain);
    add("gpu_present_wait", Performance::gpu_present_wait);
    add("gpu_queue_full", Performance::gpu_queue_full);
    text += fmt::format("journal records={} batches={} bytes={} io_ns={}\n",
        ModDiagnostics::records.load(), ModDiagnostics::batches.load(),
        ModDiagnostics::written_bytes.load(), ModDiagnostics::io_ns.load());
    ModDiagnostics::Write("performance-counters", text);
}
}

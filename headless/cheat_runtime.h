// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace Eden::CheatRuntime {
// VM execution and guest-address validation counters, with bounded reporting.
// Do not log per opcode: the cheat VM executes repeatedly during gameplay.
inline std::atomic<std::uint64_t> frames{0};
inline std::atomic<std::uint64_t> reads{0};
inline std::atomic<std::uint64_t> rejected_reads{0};
inline std::atomic<std::uint64_t> writes{0};
inline std::atomic<std::uint64_t> rejected_writes{0};
inline std::atomic<std::uint64_t> failed_writes{0};
inline std::atomic<std::uint64_t> last_rejected_address{0};

inline void Reset() {
    frames.store(0, std::memory_order_relaxed);
    reads.store(0, std::memory_order_relaxed);
    rejected_reads.store(0, std::memory_order_relaxed);
    writes.store(0, std::memory_order_relaxed);
    rejected_writes.store(0, std::memory_order_relaxed);
    failed_writes.store(0, std::memory_order_relaxed);
    last_rejected_address.store(0, std::memory_order_relaxed);
}

inline void Read(bool valid, std::uint64_t address) {
    if (valid) reads.fetch_add(1, std::memory_order_relaxed);
    else {
        rejected_reads.fetch_add(1, std::memory_order_relaxed);
        last_rejected_address.store(address, std::memory_order_relaxed);
    }
}
inline void Write(bool valid, bool write_succeeded, std::uint64_t address) {
    if (!valid) {
        rejected_writes.fetch_add(1, std::memory_order_relaxed);
        last_rejected_address.store(address, std::memory_order_relaxed);
    } else if (!write_succeeded) {
        failed_writes.fetch_add(1, std::memory_order_relaxed);
        last_rejected_address.store(address, std::memory_order_relaxed);
    } else {
        writes.fetch_add(1, std::memory_order_relaxed);
    }
}

inline void Report(std::size_t entries, std::size_t active, std::size_t words) {
    std::fprintf(stderr,
        "[ProsperoEden] cheat-vm: frames=%llu entries=%zu enabled=%zu opcodes=%zu "
        "reads=%llu rejected_reads=%llu writes=%llu rejected_writes=%llu failed_writes=%llu "
        "last_rejected=%#llx\n",
        static_cast<unsigned long long>(frames.load(std::memory_order_relaxed)),
        entries, active, words,
        static_cast<unsigned long long>(reads.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(rejected_reads.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(writes.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(rejected_writes.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(failed_writes.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(last_rejected_address.load(std::memory_order_relaxed)));
}
} // namespace Eden::CheatRuntime

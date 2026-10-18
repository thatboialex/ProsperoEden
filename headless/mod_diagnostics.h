// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Normal-thread diagnostics only. Native signal handlers use crash_report.cpp instead.
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <fcntl.h>
#include <unistd.h>
namespace Eden::ModDiagnostics {
inline std::mutex mutex, io_mutex;
inline std::string pending;
inline bool immediate = false;
inline std::atomic<unsigned long long> records{0}, batches{0}, written_bytes{0}, io_ns{0};
inline constexpr std::size_t pending_limit = 256 * 1024;
inline void Flush(bool durable = false);
inline std::string path;
inline bool enabled = false;
inline constexpr std::size_t limit = 8 * 1024 * 1024;
inline bool Start(const std::string& folder) {
    Flush();
    std::lock_guard io_lock(io_mutex);
    std::lock_guard lock(mutex);
    path = folder + "/mod-diagnostics.log";
    std::error_code error;
    std::filesystem::create_directories(folder, error);
    if (error) return false;
    std::rename(path.c_str(), (path + ".prev").c_str());
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) return false;
    ::close(fd);
    return true;
}
inline bool IsEnabled() { std::lock_guard lock(mutex); return enabled && !path.empty(); }
inline void Enable(bool value) { std::lock_guard lock(mutex); enabled = value; }
// Storage is serialized separately: producers can queue while a batch is written.
inline void Flush(bool durable) {
    std::lock_guard io_lock(io_mutex);
    std::string record, destination;
    {
        std::lock_guard lock(mutex);
        if (pending.empty() && !durable) return;
        if (path.empty()) return;
        destination = path;
        record.swap(pending);
    }
    const auto start = std::chrono::steady_clock::now();
    std::error_code error;
    if (std::filesystem::file_size(destination, error) >= limit && !error)
        std::rename(destination.c_str(), (destination + ".prev").c_str());
    const int fd = ::open(destination.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) { std::fputs("EDEN_MOD_DIAGNOSTICS write failed\n", stderr); return; }
    std::size_t offset = 0;
    while (offset < record.size()) {
        const auto n = ::write(fd, record.data() + offset, record.size() - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { std::fputs("EDEN_MOD_DIAGNOSTICS incomplete write\n", stderr); break; }
        offset += static_cast<std::size_t>(n);
    }
    if (durable && ::fsync(fd) != 0) std::fputs("EDEN_MOD_DIAGNOSTICS sync failed\n", stderr);
    ::close(fd);
    batches.fetch_add(1, std::memory_order_relaxed);
    written_bytes.fetch_add(offset, std::memory_order_relaxed);
    io_ns.fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - start).count(), std::memory_order_relaxed);
}
inline void SetImmediate(bool value) { std::lock_guard lock(mutex); immediate = value; }
inline void Write(std::string_view event, std::string_view text, bool durable = false) {
    bool flush = durable;
    {
        std::lock_guard lock(mutex);
        if (!enabled || path.empty()) return;
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        pending += "\n[" + std::to_string(now) + "] " + std::string(event) + "\n";
        pending.append(text.substr(0, 65536));
        if (text.size() > 65536) pending += "\n[truncated at 64 KiB]";
        pending += "\n";
        records.fetch_add(1, std::memory_order_relaxed);
        flush = flush || immediate || pending.size() >= pending_limit;
    }
    // Faults and critical startup markers drain all preceding records and fsync.
    // Normal records are flushed by LogFlusher once a second, or at the size bound.
    if (flush) Flush(durable);
}
}

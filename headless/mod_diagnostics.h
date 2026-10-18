// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Normal-thread diagnostics only. Native signal handlers use crash_report.cpp instead.
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
inline std::mutex mutex;
inline std::string path;
inline bool enabled = false;
inline constexpr std::size_t limit = 8 * 1024 * 1024;
inline bool Start(const std::string& folder) {
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
inline void Write(std::string_view event, std::string_view text, bool durable = false) {
    std::lock_guard lock(mutex);
    if (!enabled || path.empty()) return;
    std::error_code error;
    if (std::filesystem::file_size(path, error) >= limit && !error)
        std::rename(path.c_str(), (path + ".prev").c_str());
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) { std::fputs("EDEN_MOD_DIAGNOSTICS write failed\n", stderr); return; }
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string record = "\n[" + std::to_string(now) + "] " + std::string(event) + "\n";
    record.append(text.substr(0, 65536));
    if (text.size() > 65536) record += "\n[truncated at 64 KiB]";
    record += "\n";
    std::size_t offset = 0;
    while (offset < record.size()) {
        const auto n = ::write(fd, record.data() + offset, record.size() - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { std::fputs("EDEN_MOD_DIAGNOSTICS incomplete write\n", stderr); break; }
        offset += static_cast<std::size_t>(n);
    }
    if (durable && ::fsync(fd) != 0) std::fputs("EDEN_MOD_DIAGNOSTICS sync failed\n", stderr);
    ::close(fd);
}
}

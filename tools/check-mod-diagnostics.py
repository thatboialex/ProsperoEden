#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Verify journal survival without destructors, concurrent writers and bounded retention."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
program = r'''
#include "mod_inventory.h"
#include "log_flusher.h"
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <fstream>
#include <iterator>
#include <sys/wait.h>
#include <thread>
#include <vector>
std::atomic<bool> stall_write{false};
std::mutex storage_mutex;
std::condition_variable storage_wake;
bool storage_blocked = false, storage_released = false;
extern "C" ssize_t __real_write(int, const void*, size_t);
extern "C" ssize_t __wrap_write(int fd, const void* bytes, size_t count) {
    if (stall_write.load()) {
        std::unique_lock lock(storage_mutex);
        storage_blocked = true; storage_wake.notify_all();
        storage_wake.wait(lock, [] { return storage_released; });
    }
    return __real_write(fd, bytes, count);
}
std::string read(const std::string& path) {
    std::ifstream file(path); return {std::istreambuf_iterator<char>(file), {}};
}
int main(int argc, char** argv) {
    assert(argc == 2);
    const std::string folder = argv[1], log = folder + "/mod-diagnostics.log";
    using namespace Eden::ModDiagnostics;
    assert(Start(folder));
    Write("disabled", "must not appear"); assert(read(log).empty());
    const auto child = fork(); assert(child >= 0);
    if (!child) { Enable(true); Write("guest-debug", "plugin-before-fault", true); _exit(0); }
    int status; assert(waitpid(child, &status, 0) == child && status == 0);
    assert(read(log).find("plugin-before-fault") != std::string::npos);
    assert(Start(folder)); assert(read(log + ".prev").find("plugin-before-fault") != std::string::npos);
    Enable(true);
    std::vector<std::thread> writers;
    for (unsigned t = 0; t < 4; ++t) writers.emplace_back([t] {
        for (unsigned i = 0; i < 50; ++i) Write("parallel", std::to_string(t) + ":" + std::to_string(i));
    });
    for (auto& thread : writers) thread.join();
    assert(batches.load() == 0); // 200 producer messages caused no storage calls in this process.
    Flush();
    assert(batches.load() == 1);
    const auto text = read(log);
    for (unsigned t = 0; t < 4; ++t) for (unsigned i = 0; i < 50; ++i)
        assert(text.find("\n" + std::to_string(t) + ":" + std::to_string(i) + "\n") != std::string::npos);
    Write("slow-storage", "first");
    stall_write = true;
    std::thread flush_thread([] { Flush(); });
    {
        std::unique_lock lock(storage_mutex);
        storage_wake.wait(lock, [] { return storage_blocked; });
    }
    // Must finish while the actual storage write is still blocked.
    for (unsigned i = 0; i < 100; ++i) Write("during-stall", std::to_string(i));
    {
        std::lock_guard lock(storage_mutex); storage_released = true;
    }
    storage_wake.notify_all(); flush_thread.join(); stall_write = false;
    Flush(); assert(read(log).find("\n99\n") != std::string::npos);
    std::filesystem::resize_file(log, limit + 1);
    Write("fault", "final-fault", true);
    assert(std::filesystem::file_size(log + ".prev") == limit + 1);
    assert(read(log).find("final-fault") != std::string::npos);
    const std::string mods = folder + "/mods", user = folder + "/user";
    const std::string plugins = mods + "/01006A800016E000/Skyline/romfs/skyline/plugins";
    std::filesystem::create_directories(plugins);
    std::ofstream(plugins + "/libarcropolis.nro") << "abc";
    std::filesystem::create_directories(user + "/sdmc/ultimate/mods/Character Test");
    Inventory(mods, "", user, 0x01006A800016E000ULL);
    assert(read(log).find("sha256=ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") != std::string::npos);
    assert(read(log).find("Character Test") != std::string::npos);
    Write("error-detail", std::string(70000, 'X'));
    Flush();
    assert(read(log).find("[truncated at 64 KiB]") != std::string::npos);
    SetImmediate(true); Write("immediate", "visible-now");
    assert(read(log).find("visible-now") != std::string::npos);
    SetImmediate(false);
    // Exercise the actual periodic integration, not just explicit Flush calls.
    Eden::LogFlusher flusher; assert(flusher.Start());
    Write("periodic", "visible-after-background-flush");
    for (unsigned i = 0; i < 30 && read(log).find("visible-after-background-flush") == std::string::npos; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(read(log).find("visible-after-background-flush") != std::string::npos);
    Write("stop", "drained-on-stop"); flusher.Stop();
    assert(read(log).find("drained-on-stop") != std::string::npos);
}
'''
with tempfile.TemporaryDirectory(prefix='eden-mod-diagnostics-') as temporary:
    work = Path(temporary)
    (work / 'check.cpp').write_text(program)
    subprocess.run(['c++', '-std=c++20', '-O2', '-pthread', '-Wall', '-Wextra', '-Werror',
                    '-I' + str(root / 'headless'), str(work / 'check.cpp'), '-o', str(work / 'check'), '-lcrypto', '-Wl,--wrap=write'], check=True)
    subprocess.run([str(work / 'check'), str(work / 'logs')], check=True, timeout=10)
print('Mod diagnostics PASS: batched writes, producers survive blocked storage, durable abrupt exit, periodic/stop flush, immediate mode, concurrency, rotation and bounds')

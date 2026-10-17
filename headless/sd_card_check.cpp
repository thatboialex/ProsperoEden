// SPDX-License-Identifier: GPL-3.0-or-later
// Host check of the SD card folder's state (sd_card.h): a missing folder stays missing, a folder
// is writable or read-only as its permissions say, and looking never creates anything.
#include "sd_card.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

int main() {
    using Eden::SdCard::State;
    // The folder is Eden's own SDMC_DIR in its user folder.
    assert(Eden::SdCard::Dir() == Eden::UserDir() + "/sdmc");

    char pattern[] = "/tmp/eden-sd-card-check-XXXXXX";
    const char* made = mkdtemp(pattern);
    assert(made != nullptr);
    const fs::path root{made};
    const std::string sd = (root / "user/sdmc").string();

    // Missing: reported, not created. The parent need not exist either.
    assert(Eden::SdCard::StateOf(sd) == State::Missing);
    assert(Eden::SdCard::Describe(sd).find("does not exist") != std::string::npos);
    assert(!fs::exists(root / "user"));

    // A file where the folder should be is not a folder.
    fs::create_directories(root / "user");
    std::ofstream(root / "user/sdmc") << "x";
    assert(Eden::SdCard::StateOf(sd) == State::Missing);
    fs::remove(root / "user/sdmc");

    // Present and writable; a card of mixed-case folders is read as it is (no folding yet).
    fs::create_directories(root / "user/sdmc/Ultimate/Mods");
    assert(Eden::SdCard::StateOf(sd) == State::Writable);
    assert(Eden::SdCard::Describe(sd) == "sd:/ is " + sd);
    assert(!fs::exists(root / "user/sdmc/ultimate"));

    // Read-only (not testable as root, which writes anywhere).
    if (geteuid() != 0) {
        chmod(sd.c_str(), 0555);
        assert(Eden::SdCard::StateOf(sd) == State::ReadOnly);
        assert(Eden::SdCard::Describe(sd).find("read-only") != std::string::npos);
        chmod(sd.c_str(), 0755);
    } else {
        std::puts("SD card: read-only case skipped (running as root)");
    }

    fs::remove_all(root);
    std::puts("SD card check passed");
    return 0;
}

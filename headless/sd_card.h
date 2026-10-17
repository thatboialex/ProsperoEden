// SPDX-License-Identifier: GPL-3.0-or-later
// The SD card folder: what a game sees as sd:/ (docs/INSTALL.md). Eden mounts it from its own
// user folder, so it is /data/prosperoeden/user/sdmc. Eden makes it when a game starts, in
// CreateFactories; the path manager's start-up pass skips it (headless/CMakeLists.txt), so
// ProsperoEden alone never creates it.
#pragma once
#include <string>
#include <unistd.h>

#include "storage_paths.h"

namespace Eden::SdCard {
inline std::string Dir() { return UserDir() + "/sdmc"; }

enum class State { Missing, ReadOnly, Writable };

// Looks only: nothing is created, and a read-only drive is a state, not an error.
inline State StateOf(const std::string& folder) {
    if (!DirectoryExists(folder)) return State::Missing;
    return access(folder.c_str(), W_OK) == 0 ? State::Writable : State::ReadOnly;
}

// The log line for a game's start. The mods the folder's atmosphere/contents holds are counted
// with the others (mods.h), so this says nothing about them.
inline std::string Describe(const std::string& folder) {
    switch (StateOf(folder)) {
    case State::Missing: return "sd:/ is not available: " + folder + " does not exist";
    case State::ReadOnly: return "sd:/ is " + folder + " (read-only: a game cannot write to it)";
    case State::Writable: return "sd:/ is " + folder;
    }
    return {};
}
} // namespace Eden::SdCard

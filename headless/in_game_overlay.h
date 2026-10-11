// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Native Vulkan pause-screen view model. The renderer presents Eden's green
// panel using the already-initialized Vulkan swapchain and the HUD draw pass;
// it never opens a competing EGL window on top of the running game.
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include "hud.h"

namespace Eden::InGameOverlay {
enum class Mode : std::uint8_t { none, cheats, graphics };
inline constexpr int kVisibleRows = 7;
inline constexpr std::size_t kMaxTextLines = 21;

struct Row {
    std::string label;
    bool enabled = false;
};
struct Model {
    Mode mode = Mode::none;
    std::vector<Row> rows;
    int selected = 0;
};
struct Draw {
    HudSnapshot background;
    std::array<HudSnapshot, kMaxTextLines> text{};
    unsigned count = 0;
};
inline std::mutex state_mutex;
inline Model state;
inline std::atomic<bool> visible{false};

inline void Open(Mode mode, std::vector<Row> rows, int selected = 0) {
    std::lock_guard lock(state_mutex);
    state = {mode, std::move(rows), 0};
    if (!state.rows.empty()) state.selected = std::clamp(selected, 0, int(state.rows.size()) - 1);
    visible.store(true, std::memory_order_release);
}
inline bool IsOpen() { return visible.load(std::memory_order_acquire); }
inline void Close() {
    visible.store(false, std::memory_order_release);
    std::lock_guard lock(state_mutex);
    state = {};
}
inline void Move(int by) {
    std::lock_guard lock(state_mutex);
    if (state.rows.empty()) return;
    const int size = int(state.rows.size());
    state.selected = (state.selected + (by % size) + size) % size;
}
inline int Selected() {
    std::lock_guard lock(state_mutex);
    return state.selected;
}
inline void SetEnabled(int index, bool enabled) {
    std::lock_guard lock(state_mutex);
    if (index >= 0 && index < int(state.rows.size())) state.rows[index].enabled = enabled;
}
inline void SelectOnly(int index) {
    std::lock_guard lock(state_mutex);
    for (int i = 0; i < int(state.rows.size()); ++i)
        state.rows[i].enabled = (i == index);
}
inline std::string VisibleText(std::string_view source) {
    // The Vulkan overlay uses compact glyphs, never transfers user strings to
    // shaders or treats a cheat name as markup. UTF-8 is kept in settings.
    std::string result;
    for (unsigned char ch : source) {
        if (result.size() == 48) break;
        if (ch == '\t' || ch == '\n') ch = ' ';
        if (ch < 32) continue;
        result.push_back(ch >= 128 ? '?' : char(std::toupper(ch)));
    }
    return result;
}
inline HudSnapshot Line(std::string_view value, unsigned x, unsigned y, bool focus = false) {
    HudSnapshot line{};
    line.glyphs = HudText(value);
    line.width = 1;
    line.x = x;
    line.y = y;
    line.loading = focus ? 4 : 3; // menu text in the same Vulkan HUD shader
    return line;
}
inline Draw Frame() {
    Draw frame{};
    if (!IsOpen()) return frame;
    std::lock_guard lock(state_mutex);
    if (state.mode == Mode::none) return frame;
    const int total = int(state.rows.size());
    const int page = state.selected / kVisibleRows * kVisibleRows;
    const int visible_rows = std::min(kVisibleRows, total - page);
    frame.background.width = 1920;
    frame.background.x = 1920;
    frame.background.y = 1080;
    frame.background.loading = 2; // full Eden-theme panel, not loading animation
    frame.background.glyphs[0] = state.mode == Mode::cheats ? 0u : 1u;
    frame.background.glyphs[1] = static_cast<std::uint32_t>(state.selected - page);
    frame.background.glyphs[2] = static_cast<std::uint32_t>(visible_rows);
    frame.background.glyphs[4] = static_cast<std::uint32_t>(page);
    frame.background.glyphs[5] = static_cast<std::uint32_t>(total);
    for (int i = 0; i < visible_rows; ++i)
        if (state.rows[page + i].enabled) frame.background.glyphs[3] |= 1u << i;

    const auto append = [&](std::string_view name, unsigned x, unsigned y, bool selected) {
        if (frame.count < frame.text.size()) frame.text[frame.count++] = Line(name, x, y, selected);
    };
    append(state.mode == Mode::cheats ? "CHEAT MANAGER" : "LIVE GRAPHICS", 480, 220, false);
    append(state.mode == Mode::cheats ? "SELECT A CHEAT TO TOGGLE" : "CHANGE UPSCALING FILTER", 480, 273, false);
    for (int i = 0; i < visible_rows; ++i) {
        const auto name = VisibleText(state.rows[page + i].label);
        const unsigned y = 362u + 62u * unsigned(i);
        const bool focus = page + i == state.selected;
        append(std::string_view(name).substr(0, 24), 500, y, focus);
        if (name.size() > 24) append(std::string_view(name).substr(24, 24), 884, y, focus);
    }
    append("CROSS SELECT  UP DOWN MOVE", 480, 827, false);
    append("CIRCLE RESUME", 480, 873, false);
    return frame;
}
} // namespace Eden::InGameOverlay

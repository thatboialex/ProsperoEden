// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace Eden::ControlsMenu {
// In 1920x1080 virtual canvas units. The Controls panel is y=180..900;
// subtitles end above y=324, and the footer begins at y=848.
inline constexpr int kRowCount = 5;
inline constexpr float kRowTop = 350.0f;
inline constexpr float kRowPitch = 96.0f;
inline constexpr float kRowHeight = 94.0f;
inline constexpr float kHeaderBottom = 324.0f;
inline constexpr float kFooterTop = 848.0f;
inline constexpr float RowTop(int row) { return kRowTop + row * kRowPitch; }
inline constexpr bool Fits() {
    if (RowTop(0) < kHeaderBottom) return false;
    for (int i = 0; i < kRowCount; ++i)
        if (RowTop(i) + kRowHeight >= kFooterTop - 12.0f) return false;
    return true;
}
static_assert(Fits(), "Controls menu rows overlap the header/footer");
} // namespace Eden::ControlsMenu

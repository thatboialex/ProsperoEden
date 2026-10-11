// SPDX-License-Identifier: GPL-3.0-or-later
#version 450
#extension GL_GOOGLE_include_directive : enable
layout(push_constant) uniform Text { uint glyphs[24]; uint width; uint x; uint y; uint loading; } text;
layout(location = 0) out vec4 color;
#include "loading_text.glsl"
#include "loading_scene.glsl"
void main() {
    // Vulkan push-constant arrays require dynamically uniform indices. Load
    // each word at a constant index before the per-pixel local-array lookup.
    uint glyphs[24] = uint[24](
        text.glyphs[0], text.glyphs[1], text.glyphs[2], text.glyphs[3],
        text.glyphs[4], text.glyphs[5], text.glyphs[6], text.glyphs[7],
        text.glyphs[8], text.glyphs[9], text.glyphs[10], text.glyphs[11],
        text.glyphs[12], text.glyphs[13], text.glyphs[14], text.glyphs[15],
        text.glyphs[16], text.glyphs[17], text.glyphs[18], text.glyphs[19],
        text.glyphs[20], text.glyphs[21], text.glyphs[22], text.glyphs[23]);
    if (text.loading == 2u) {
        // Same palette and rounded rows as pe/ui/theme.hpp, using the existing
        // Vulkan swapchain (never an EGL overlay window competing for the screen).
        vec2 p = vec2(gl_FragCoord.x * 1920.0 / max(float(text.x), 1.0),
                      gl_FragCoord.y * 1080.0 / max(float(text.y), 1.0));
        vec3 base = vec3(0.0235, 0.0353, 0.0392); // theme::kBase
        vec3 panel = vec3(0.043, 0.090, 0.075);  // theme::kPanel
        vec3 edge = vec3(0.463, 0.557, 0.459);  // theme::kPanelEdge
        vec3 lime = vec3(0.663, 0.859, 0.388);  // theme::kLime
        vec3 muted = vec3(0.09, 0.141, 0.118);
        color = vec4(base, 1.0);
        vec2 pos = p - vec2(960.0, 540.0);
        vec2 delta = abs(pos) - vec2(610.0 - 23.0, 407.0 - 23.0);
        float box = length(max(delta, 0.0)) + min(max(delta.x, delta.y), 0.0) - 23.0;
        float outer = 1.0 - smoothstep(-1.0, 1.0, box);
        float rim = (1.0 - smoothstep(-1.0, 1.0, box)) -
                    (1.0 - smoothstep(-1.0, 1.0, box + 2.0));
        vec3 rgb = mix(base, panel, outer);
        rgb = mix(rgb, edge, clamp(abs(rim) * 0.34, 0.0, 1.0));
        if (outer > 0.0) {
            const float start_y = 331.0;
            int count = int(min(text.glyphs[2], 7u));
            for (int i = 0; i < 7; ++i) {
                if (i >= count) break;
                float row_y = start_y + 62.0 * float(i);
                vec2 local = p - vec2(956.0, row_y + 23.0);
                vec2 row_d = abs(local) - vec2(490.0 - 12.0, 25.0 - 12.0);
                float row_box = length(max(row_d, 0.0)) +
                                min(max(row_d.x, row_d.y), 0.0) - 12.0;
                float inside = 1.0 - smoothstep(-1.0, 1.0, row_box);
                bool focus = i == int(text.glyphs[1]);
                vec3 row_color = focus ?
                    mix(lime, vec3(0.141, 0.365, 0.290), clamp((p.x - 458.0) / 995.0, 0.0, 1.0)) :
                    muted;
                rgb = mix(rgb, row_color, inside * (focus ? 0.90 : 0.8));
                // Hide switches for non-actionable informational rows (for
                // example an empty cheat list). The GPU still draws the row.
                if ((text.glyphs[6] & (1u << uint(i))) != 0u) {
                    float sw = length(max(abs(p - vec2(1396.0, row_y + 23.0)) -
                        vec2(48.0 - 17.0, 17.0 - 17.0), 0.0)) -
                        17.0; // oval toggle
                    float pill = 1.0 - smoothstep(-1.0, 1.0, sw);
                    bool enabled = (text.glyphs[3] & (1u << uint(i))) != 0u;
                    vec3 pill_color = enabled ? lime : vec3(0.25, 0.36, 0.31);
                    rgb = mix(rgb, pill_color, pill);
                    float knob_x = enabled ? 1423.0 : 1369.0;
                    float knob = length(p - vec2(knob_x, row_y + 23.0));
                    rgb = mix(rgb, vec3(0.96, 0.98, 0.94),
                        (1.0 - smoothstep(11.0, 12.0, knob)) * pill);
                }
            }
            // Eden lime divider beneath the panel heading.
            float divider = (1.0 - smoothstep(1.0, 2.5, abs(p.y - 314.0))) *
                step(460.0, p.x) * step(p.x, 1460.0);
            rgb = mix(rgb, lime, divider * 0.65);
        }
        color = vec4(rgb, 1.0);
        return;
    }
    if (text.loading == 3u || text.loading == 4u) {
        ivec2 p = ivec2(gl_FragCoord.xy) - ivec2(text.x, text.y);
        float scale = 4.0 * float(text.width) / 1080.0;
        bool ink = false;
        if (p.x >= 0 && p.y >= 0) {
            ivec2 at = ivec2(vec2(p) / scale);
            int letter = at.x / 4;
            int x = at.x % 4;
            if (letter < 24 && x < 3 && at.y < 5) {
                // Reuse the dynamically uniform glyph array loaded above.
                ink = ((glyphs[letter] >> uint((4 - at.y) * 3 + 2 - x)) & 1u) != 0u;
            }
        }
        vec3 typeface = text.loading == 4u ?
            vec3(0.062, 0.137, 0.094) : vec3(0.94, 0.97, 0.92);
        color = vec4(typeface, ink ? 1.0 : 0.0);
        return;
    }
    if (text.loading != 0u) {
        // The loading screen: x and y carry the picture's size, loading the milliseconds since
        // it began (plus one), the glyphs' words what the start is doing (hud.h). Vulkan counts
        // rows from the top.
        vec2 size = vec2(float(text.x), float(text.y));
        color = vec4(loading_scene(vec2(gl_FragCoord.x, size.y - gl_FragCoord.y), size,
                                   float(text.loading - 1u) * 0.001, glyphs), 1.0);
        return;
    }
    // The overlay: x and y are where its text starts, width the picture's height. The text is
    // laid out for a picture 1080 rows high (a glyph's cell is 4 pixels there).
    ivec2 p = ivec2(gl_FragCoord.xy) - ivec2(text.x, text.y);
    float cell_size = 4.0 * float(text.width) / 1080.0;
    bool ink = false;
    if (p.x >= 0 && p.y >= 0) {
        ivec2 cell = ivec2(vec2(p) / cell_size);
        int i = cell.x / 4;
        int x = cell.x % 4;
        if (i < 24 && x < 3 && cell.y < 5)
            ink = ((glyphs[i] >> uint((4 - cell.y) * 3 + 2 - x)) & 1u) != 0u;
    }
    color = ink ? vec4(0.9, 0.95, 1.0, 1.0) : vec4(0.025, 0.04, 0.075, 0.5);
}

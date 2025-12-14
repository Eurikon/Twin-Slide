#pragma once
#include <rack.hpp>

using namespace rack;

// Twisted Cable Logo Widget - Shared component for all modules
// Renders the TC branding with consistent styling across the plugin

struct TCLogoWidget : Widget {
    enum Style {
        COMPACT,  // "T C ∞" vertical layout (for 2HP modules like Control1)
        FULL,     // "TWISTED CABLE ∞" two-line layout (for wider modules)
        WIDE      // "TWISTED ∞ CABLE" single row (for wide modules)
    };

    Style style;
    Module* module;  // Optional module pointer for future dynamic behavior

    TCLogoWidget(Style s = FULL, Module* m = nullptr) : style(s), module(m) {
        if (style == COMPACT) {
            box.size = Vec(20, 30);
        } else if (style == WIDE) {
            box.size = Vec(100, 12);
        } else {
            box.size = Vec(42, 20);
        }
    }

    void draw(const DrawArgs& args) override {
        std::shared_ptr<Font> sonoFont = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono-Bold.ttf"));
        if (!sonoFont) return;

        nvgFontFaceId(args.vg, sonoFont->handle);
        nvgFontSize(args.vg, 12.0f);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        if (style == COMPACT) {
            drawCompact(args);
        } else if (style == WIDE) {
            drawWide(args);
        } else {
            drawFull(args);
        }
    }

private:
    void drawCompact(const DrawArgs& args) {
        // T - dark fill with amber outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, dx * 0.3f, -4 + dy * 0.3f, "T", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 0, -4, "T", NULL);

        // C - dark fill with amber outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, dx * 0.3f, 4 + dy * 0.3f, "C", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 0, 4, "C", NULL);

        // ∞ - dark fill with white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, dx * 0.3f, 12 + dy * 0.3f, "∞", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 0, 12, "∞", NULL);
    }

    void drawFull(const DrawArgs& args) {
        // Line 1: "TWISTED"
        // T with dark fill and amber outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, -15 + dx * 0.3f, dy * 0.3f, "T", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, -15, 0, "T", NULL);

        // WISTED with dark fill and white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, 7 + dx * 0.3f, dy * 0.3f, "WISTED", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 7, 0, "WISTED", NULL);

        // Line 2: "CABLE ∞"
        // C with dark fill and amber outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, -15 + dx * 0.3f, 10 + dy * 0.3f, "C", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, -15, 10, "C", NULL);

        // ABLE with dark fill and white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, 1 + dx * 0.3f, 10 + dy * 0.3f, "ABLE", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 1, 10, "ABLE", NULL);

        // ∞ with dark fill and white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, 20 + dx * 0.3f, 10 + dy * 0.3f, "∞", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, 20, 10, "∞", NULL);
    }

    void drawWide(const DrawArgs& args) {
        // Single row: "TWISTED ∞ CABLE" - all positioned from widget center (50, 6)
        float cx = 50.f;  // Widget center X
        float cy = 6.f;   // Widget center Y

        // Layout: TWISTED (right-aligned to left of ∞) - ∞ (center) - CABLE (left-aligned to right of ∞)
        float gap = 8.f;  // Gap between words and symbol

        // TWISTED - right-aligned, ending before the ∞
        nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);

        // First draw "WISTED" with white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, cx - gap + dx * 0.3f, cy + dy * 0.3f, "WISTED", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, cx - gap, cy, "WISTED", NULL);

        // Measure where WISTED starts to place T
        float wistedWidth = nvgTextBounds(args.vg, 0, 0, "WISTED", NULL, NULL);
        float tPos = cx - gap - wistedWidth;

        // T with amber outline (right-aligned, just before WISTED)
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, tPos + dx * 0.3f, cy + dy * 0.3f, "T", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, tPos, cy, "T", NULL);

        // ∞ with white outline (center)
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, cx + dx * 0.3f, cy + dy * 0.3f, "∞", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, cx, cy, "∞", NULL);

        // CABLE - left-aligned, starting after the ∞
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        // C with amber outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, cx + gap + dx * 0.3f, cy + dy * 0.3f, "C", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, cx + gap, cy, "C", NULL);

        // Measure C width to place ABLE
        float cWidth = nvgTextBounds(args.vg, 0, 0, "C", NULL, NULL);
        float ablePos = cx + gap + cWidth;

        // ABLE with white outline
        nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    nvgText(args.vg, ablePos + dx * 0.3f, cy + dy * 0.3f, "ABLE", NULL);
                }
            }
        }
        nvgFillColor(args.vg, nvgRGB(0x2d, 0x2d, 0x2d));
        nvgText(args.vg, ablePos, cy, "ABLE", NULL);

        // Reset alignment
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    }
};

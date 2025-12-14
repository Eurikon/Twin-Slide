//***********************************************************************************************
// MicroKeyboard - Tiny 12-key keyboard widget for microtuning selection
//***********************************************************************************************

#pragma once

#include "../TwinSlidePlugin.hpp"
#include "Components.hpp"

using namespace rack;


// Temperament preset data
struct TemperamentPreset {
	const char* name;
	float cents[12];
};

static const TemperamentPreset TEMPERAMENT_PRESETS[] = {
	// Initialize (resets to 12-TET)
	{"INITIALIZE", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	// Western historical
	{"JUST", {0, 11.7, 3.9, 15.6, -13.7, -2.0, -9.8, 2.0, 13.7, -15.6, 17.6, -11.7}},
	{"PYTH", {0, -9.8, 3.9, -5.9, 7.8, -2.0, -11.7, 2.0, -7.8, 5.9, -3.9, 9.8}},
	{"MEAN1/4", {0, -24.0, -6.8, 10.3, -13.7, 3.4, -20.5, -3.4, -27.4, -10.3, 6.8, -17.1}},
	{"MEAN1/6", {0, -14.0, -4.0, 6.0, -8.0, 2.0, -12.0, -2.0, -16.0, -6.0, 4.0, -10.0}},
	{"WERCK3", {0, -10.0, -8.0, -6.0, -10.0, -2.0, -12.0, -4.0, -8.0, -12.0, -4.0, -8.0}},
	{"WERCK4", {0, -8.0, -4.0, -8.0, -4.0, 2.0, -6.0, -2.0, -10.0, -2.0, -6.0, -2.0}},
	{"KIRN3", {0, -10.0, -7.0, -3.0, -14.0, -2.0, -12.0, -4.0, -8.0, -11.0, 0.0, -12.0}},
	{"VALLOT", {0, -6.0, -4.0, -2.0, -8.0, 0.0, -6.0, -2.0, -4.0, -6.0, 0.0, -6.0}},
	{"YOUNG", {0, -6.0, -4.0, -2.0, -8.0, 2.0, -8.0, -2.0, -4.0, -6.0, 0.0, -6.0}},
	{"KELLNR", {0, -9.0, -5.0, -3.0, -10.0, 0.0, -9.0, -2.0, -7.0, -7.0, -2.0, -7.0}},
	{"BARNES", {0, -8.0, -6.0, -4.0, -10.0, 0.0, -10.0, -2.0, -6.0, -8.0, -2.0, -8.0}},
	// Extended equal divisions
	{"17-TET", {0, -29.0, -6.0, 18.0, -12.0, 12.0, -35.0, 6.0, -24.0, 24.0, -18.0, 6.0}},
	{"19-TET", {0, -37.0, -11.0, 16.0, -21.0, 5.0, -32.0, -5.0, -42.0, 11.0, -26.0, 0.0}},
	{"22-TET", {0, -18.0, 9.0, -9.0, 0.0, 18.0, -27.0, 0.0, -18.0, 9.0, -9.0, 18.0}},
	{"24-TET", {0, -50.0, 0.0, -50.0, 0.0, -50.0, 0.0, 0.0, -50.0, 0.0, -50.0, 0.0}},
	{"31-TET", {0, -13.0, -3.0, 6.0, -10.0, 3.0, -13.0, -3.0, -16.0, 3.0, -10.0, 0.0}},
	{"53-TET", {0, -9.0, 4.0, -6.0, -2.0, 2.0, -11.0, 2.0, -8.0, 6.0, -4.0, 0.0}},
	// Arabic maqam (Egyptian/Syrian measured values)
	{"RAST-E", {0, 4.0, 4.0, 45.0, -2.0, 2.0, 2.0, 2.0, 4.0, 45.0, 0.0, -10.0}},
	{"RAST-S", {0, 4.0, 4.0, 53.0, -2.0, 2.0, 2.0, 2.0, 4.0, 53.0, -4.0, -10.0}},
	{"BAYATI", {0, 51.0, 4.0, 16.0, -14.0, -2.0, 51.0, 2.0, 51.0, 16.0, 18.0, -12.0}},
	{"SABA", {0, 51.0, 4.0, 35.0, -14.0, 0.0, 51.0, 2.0, 51.0, 40.0, 0.0, -10.0}},
	{"HIJAZ", {0, 20.0, 4.0, 53.0, -14.0, -2.0, 0.0, 2.0, 20.0, 53.0, 0.0, -10.0}},
	{"SEGAH", {0, 0.0, 4.0, 51.0, 16.0, -2.0, 0.0, 2.0, 0.0, 51.0, 18.0, -10.0}},
	{"NAHWND", {0, 4.0, 4.0, 16.0, -14.0, -2.0, 4.0, 2.0, 4.0, 16.0, 0.0, -10.0}},
	{"KURD", {0, 16.0, 4.0, 16.0, -14.0, -2.0, 16.0, 2.0, 16.0, 16.0, 0.0, -10.0}},
	// Persian radif (dastgah systems)
	{"SHUR", {0, 45.0, 4.0, 16.0, -14.0, -2.0, 45.0, 2.0, 45.0, 16.0, 0.0, -10.0}},
	{"SEGAH-P", {0, 0.0, 4.0, 55.0, 20.0, -2.0, 0.0, 2.0, 0.0, 55.0, 20.0, -10.0}},
	{"CHAHAR", {0, 0.0, 4.0, 55.0, -14.0, -2.0, 0.0, 2.0, 0.0, 55.0, 0.0, -10.0}},
	{"MAHUR", {0, -6.0, 4.0, -2.0, -14.0, -2.0, -6.0, 2.0, -6.0, -2.0, 0.0, -10.0}},
	{"HOMYUN", {0, 45.0, 4.0, 16.0, -14.0, -2.0, 0.0, 2.0, 45.0, 16.0, 0.0, -10.0}},
	{"NAVA", {0, 4.0, 4.0, 16.0, -14.0, -2.0, 45.0, 2.0, 4.0, 16.0, 0.0, -10.0}},
	// Sundanese gamelan
	{"SLNDRO", {0, 0.0, 40.0, 0.0, 0.0, -20.0, 0.0, 40.0, 0.0, 0.0, -20.0, 0.0}},
	{"PELOG", {0, 30.0, 0.0, 25.0, 0.0, -30.0, 0.0, 20.0, 0.0, 35.0, 0.0, -25.0}},
	{"DEGUNG", {0, 0.0, 40.0, 0.0, 0.0, -60.0, 0.0, 40.0, 0.0, 0.0, -60.0, 0.0}},
	{"MADNDA", {0, 0.0, 20.0, 0.0, 0.0, -40.0, 0.0, 20.0, 0.0, 0.0, -40.0, 0.0}},
	// Thai/Asian
	{"THAI", {0, 29.0, -14.0, 14.0, -29.0, 0.0, 29.0, -14.0, 14.0, -29.0, 0.0, 29.0}},
	{"SHRUTI", {0, 12.0, -8.0, 16.0, -14.0, -2.0, 12.0, 2.0, 14.0, -16.0, 18.0, -12.0}},
	// Harmonic series based
	{"HARM7", {0, 0.0, 4.0, -31.0, -14.0, -2.0, -31.0, 2.0, 14.0, -16.0, -31.0, -12.0}},
	{"HARM11", {0, 0.0, 4.0, -49.0, -14.0, -2.0, -49.0, 2.0, 14.0, -16.0, -49.0, -12.0}},
	{"HARM13", {0, 0.0, 4.0, 41.0, -14.0, -2.0, -49.0, 2.0, 14.0, 41.0, -49.0, -12.0}},
	// Wendy Carlos (non-octave scales mapped to 12-note)
	{"ALPHA", {0, -22.0, 56.0, 34.0, 12.0, -10.0, -32.0, 46.0, 24.0, 2.0, -20.0, 58.0}},
	{"BETA", {0, -36.0, 28.0, -8.0, 56.0, 20.0, -16.0, 48.0, 12.0, -24.0, 40.0, 4.0}},
	{"GAMMA", {0, -15.0, 20.0, 5.0, -10.0, 25.0, 10.0, -5.0, 30.0, 15.0, 0.0, 35.0}},
	// Experimental/modern
	{"LUCY", {0, -18.0, -4.0, 10.0, -8.0, 6.0, -22.0, 2.0, -16.0, -2.0, 12.0, -6.0}},
	{"GOLDEN", {0, -10.0, 18.0, -8.0, 10.0, -18.0, 0.0, 18.0, -10.0, 8.0, -18.0, 10.0}},
	{"BOHLEN", {0, 46.0, -7.0, 39.0, -14.0, 32.0, -21.0, 25.0, -28.0, 18.0, -35.0, 11.0}}
};
static const int NUM_TEMPERAMENTS = 47;


struct TemperamentSelector : OpaqueWidget {
	int* presetIndex = nullptr;
	float* microtuningCents = nullptr;
	bool leftPressed = false;
	bool rightPressed = false;

	static constexpr float WIDTH_MM = 32.0f;
	static constexpr float HEIGHT_MM = 5.0f;

	const NVGcolor bgColor = nvgRGB(0x1a, 0x1a, 0x1a);
	const NVGcolor textColor = nvgRGB(0xff, 0xc0, 0x50);
	const NVGcolor arrowColor = nvgRGB(0x80, 0x60, 0x28);
	const NVGcolor arrowHoverColor = nvgRGB(0xff, 0xc0, 0x50);

	TemperamentSelector() {
		box.size = mm2px(Vec(WIDTH_MM, HEIGHT_MM));
	}

	void draw(const DrawArgs& args) override {
		float arrowBoxSize = 16.0f;
		float padding = 2.0f;

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0f);
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0f);
		NVGpaint grad = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, colTop, colBot);
		nvgStrokePaint(args.vg, grad);
		nvgStrokeWidth(args.vg, 0.75f);
		nvgStroke(args.vg);

		NVGpaint gradReverse = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, colBot, colTop);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, padding, padding, arrowBoxSize, box.size.y - padding * 2, 1.5f);
		nvgStrokePaint(args.vg, leftPressed ? grad : gradReverse);
		nvgStrokeWidth(args.vg, 0.3f);
		nvgStroke(args.vg);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.size.x - arrowBoxSize - padding, padding, arrowBoxSize, box.size.y - padding * 2, 1.5f);
		nvgStrokePaint(args.vg, rightPressed ? grad : gradReverse);
		nvgStrokeWidth(args.vg, 0.3f);
		nvgStroke(args.vg);

		int idx = presetIndex ? *presetIndex : 0;
		const char* name = TEMPERAMENT_PRESETS[idx].name;

		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
		if (font) {
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 10.0f);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgFillColor(args.vg, textColor);
			nvgText(args.vg, box.size.x / 2.0f, box.size.y / 2.0f, name, NULL);

			nvgFillColor(args.vg, arrowColor);
			nvgText(args.vg, padding + arrowBoxSize / 2.0f, box.size.y / 2.0f, "<", NULL);
			nvgText(args.vg, box.size.x - padding - arrowBoxSize / 2.0f, box.size.y / 2.0f, ">", NULL);
		}
	}

	void applyPreset(int idx) {
		if (!presetIndex || !microtuningCents) return;
		*presetIndex = idx;
		for (int i = 0; i < 12; i++) {
			microtuningCents[i] = TEMPERAMENT_PRESETS[idx].cents[i];
		}
	}

	void createPresetSubmenu(ui::Menu* menu, const char* label, int startIdx, int endIdx) {
		menu->addChild(createSubmenuItem(label, "", [=](ui::Menu* subMenu) {
			for (int i = startIdx; i <= endIdx; i++) {
				int idx = i;
				subMenu->addChild(createMenuItem(TEMPERAMENT_PRESETS[i].name, "", [=]() {
					applyPreset(idx);
				}));
			}
		}));
	}

	void onButton(const event::Button& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			float arrowZone = 20.0f;

			if (e.action == GLFW_PRESS) {
				if (e.pos.x < arrowZone) {
					leftPressed = true;
				} else if (e.pos.x > box.size.x - arrowZone) {
					rightPressed = true;
				}

				if (!presetIndex || !microtuningCents) {
					e.consume(this);
					return;
				}

				if (e.pos.x < arrowZone) {
					*presetIndex = (*presetIndex - 1 + NUM_TEMPERAMENTS) % NUM_TEMPERAMENTS;
					for (int i = 0; i < 12; i++) {
						microtuningCents[i] = TEMPERAMENT_PRESETS[*presetIndex].cents[i];
					}
				} else if (e.pos.x > box.size.x - arrowZone) {
					*presetIndex = (*presetIndex + 1) % NUM_TEMPERAMENTS;
					for (int i = 0; i < 12; i++) {
						microtuningCents[i] = TEMPERAMENT_PRESETS[*presetIndex].cents[i];
					}
				} else {
					ui::Menu* menu = createMenu();
					menu->addChild(createMenuLabel("Temperaments"));
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuItem("Initialize (12-TET)", "", [=]() { applyPreset(0); }));
					createPresetSubmenu(menu, "Western Historical", 1, 11);
					createPresetSubmenu(menu, "Extended Equal", 12, 17);
					createPresetSubmenu(menu, "Arabic Maqam", 18, 25);
					createPresetSubmenu(menu, "Persian Radif", 26, 31);
					createPresetSubmenu(menu, "Sundanese Gamelan", 32, 35);
					createPresetSubmenu(menu, "Thai/Asian", 36, 37);
					createPresetSubmenu(menu, "Harmonic Series", 38, 40);
					createPresetSubmenu(menu, "Wendy Carlos", 41, 43);
					createPresetSubmenu(menu, "Experimental", 44, 46);
				}
				e.consume(this);
			} else if (e.action == GLFW_RELEASE) {
				leftPressed = false;
				rightPressed = false;
			}
		}
	}

	void onDragEnd(const event::DragEnd& e) override {
		leftPressed = false;
		rightPressed = false;
	}
};


struct MicroKeyboard : OpaqueWidget {
	int* selectedKey = nullptr;

	static constexpr float WIDTH_MM = 32.0f;
	static constexpr float HEIGHT_MM = 8.0f;
	static constexpr float WHITE_KEY_COUNT = 7.0f;

	const NVGcolor amberColor = nvgRGB(0xff, 0xc0, 0x50);
	const NVGcolor amberDimColor = nvgRGB(0x80, 0x60, 0x28);
	const NVGcolor blackKeyColor = nvgRGB(0x1a, 0x1a, 0x1a);
	const NVGcolor blackKeySelectedColor = nvgRGB(0x40, 0x40, 0x40);
	const NVGcolor outlineColor = nvgRGB(0x3a, 0x3a, 0x3a);

	const int whiteKeyNotes[7] = {0, 2, 4, 5, 7, 9, 11};
	const int blackKeyNotes[5] = {1, 3, 6, 8, 10};
	const int blackKeyPositions[5] = {0, 1, 3, 4, 5};

	MicroKeyboard() {
		box.size = mm2px(Vec(WIDTH_MM, HEIGHT_MM));
	}

	void draw(const DrawArgs& args) override {
		float whiteKeyWidth = box.size.x / WHITE_KEY_COUNT;
		float whiteKeyHeight = box.size.y;
		float blackKeyWidth = whiteKeyWidth * 0.6f;
		float blackKeyHeight = whiteKeyHeight * 0.6f;

		int selected = selectedKey ? *selectedKey : -1;

		for (int i = 0; i < 7; i++) {
			float x = i * whiteKeyWidth;
			int note = whiteKeyNotes[i];
			bool isSelected = (note == selected);

			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, x + 0.5f, 0.5f, whiteKeyWidth - 1.0f, whiteKeyHeight - 1.0f, 1.5f);
			nvgFillColor(args.vg, isSelected ? amberColor : amberDimColor);
			nvgFill(args.vg);

			nvgStrokeColor(args.vg, outlineColor);
			nvgStrokeWidth(args.vg, 0.3f);
			nvgStroke(args.vg);
		}

		for (int i = 0; i < 5; i++) {
			int pos = blackKeyPositions[i];
			float x = (pos + 1) * whiteKeyWidth - blackKeyWidth / 2.0f;
			int note = blackKeyNotes[i];
			bool isSelected = (note == selected);

			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, x, 0, blackKeyWidth, blackKeyHeight, 1.5f);
			nvgFillColor(args.vg, isSelected ? blackKeySelectedColor : blackKeyColor);
			nvgFill(args.vg);

			if (isSelected) {
				nvgStrokeColor(args.vg, amberColor);
				nvgStrokeWidth(args.vg, 1.0f);
				nvgStroke(args.vg);
			}
		}
	}

	void onButton(const event::Button& e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			float whiteKeyWidth = box.size.x / WHITE_KEY_COUNT;
			float blackKeyWidth = whiteKeyWidth * 0.6f;
			float blackKeyHeight = box.size.y * 0.6f;

			float x = e.pos.x;
			float y = e.pos.y;

			if (y < blackKeyHeight) {
				for (int i = 0; i < 5; i++) {
					int pos = blackKeyPositions[i];
					float keyX = (pos + 1) * whiteKeyWidth - blackKeyWidth / 2.0f;
					if (x >= keyX && x < keyX + blackKeyWidth) {
						if (selectedKey) *selectedKey = blackKeyNotes[i];
						e.consume(this);
						return;
					}
				}
			}

			int whiteKeyIndex = (int)(x / whiteKeyWidth);
			if (whiteKeyIndex >= 0 && whiteKeyIndex < 7) {
				if (selectedKey) *selectedKey = whiteKeyNotes[whiteKeyIndex];
				e.consume(this);
			}
		}
	}
};

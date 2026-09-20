//***********************************************************************************************
// MicroKeyboard - Tiny 12-key keyboard widget for microtuning selection
//***********************************************************************************************

#pragma once

#include "../TwinSlidePlugin.hpp"
#include "Components.hpp"
#include <atomic>
#include <cmath>
#include <cstring>

using namespace rack;


enum TemperamentCategory {
	TEMPERAMENT_INITIALIZE,
	TEMPERAMENT_WESTERN,
	TEMPERAMENT_JUST,
	TEMPERAMENT_EDO
};

enum TemperamentKind {
	TEMPERAMENT_CENTS,
	TEMPERAMENT_RATIOS,
	TEMPERAMENT_EDO_MAP
};

struct TuningRatio {
	int numerator;
	int denominator;
};

struct TemperamentPreset {
	const char* id;
	const char* name;
	TemperamentCategory category;
	TemperamentKind kind;
	const float* cents;
	const TuningRatio* ratios;
	int edo;
	int legacyIndex;
};

static const float INITIALIZE_CENTS[12] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static const float PYTH_CENTS[12] = {0.0f, 13.68501f, 3.91f, -5.865f, 7.82f, -1.955f, 11.73001f, 1.955f, 15.64001f, 5.865f, -3.91f, 9.775f};
static const float MEAN_QUARTER_CENTS[12] = {0.0f, -23.951f, -6.84314f, 10.26471f, -13.68629f, 3.42157f, -20.52943f, -3.42157f, -27.37257f, -10.26471f, 6.84314f, -17.10786f};
static const float MEAN_SIXTH_CENTS[12] = {0.0f, -11.40567f, -3.25876f, 4.88814f, -6.51752f, 1.62938f, -9.77628f, -1.62938f, -13.03505f, -4.88814f, 3.25876f, -8.1469f};
static const float WERCKMEISTER_III_CENTS[12] = {0.0f, -9.775f, -7.82f, -5.865f, -9.775f, -1.955f, -11.73001f, -3.91f, -7.82f, -11.73001f, -3.91f, -7.82f};
static const float WERCKMEISTER_IV_CENTS[12] = {0.0f, -17.59501f, -3.91f, -5.865f, -7.82f, -1.955f, -11.73001f, -5.865f, -15.64001f, -9.775f, 3.91f, -13.68501f};
static const float KIRNBERGER_III_CENTS[12] = {0.0f, -7.82128f, -6.84314f, -5.865f, -13.68629f, -1.955f, -9.77628f, -3.42157f, -5.86628f, -10.26471f, -3.91f, -11.73129f};
static const float VALLOTTI_CENTS[12] = {0.0f, -5.865f, -3.91f, -1.955f, -7.82f, 1.955f, -7.82f, -1.955f, -3.91f, -5.865f, 0.0f, -9.775f};
static const float YOUNG_CENTS[12] = {0.0f, -9.775f, -3.91f, -5.865f, -7.82f, -1.955f, -11.73001f, -1.955f, -7.82f, -5.865f, -3.91f, -9.775f};
static const float KELLNER_CENTS[12] = {0.0f, -9.775f, -5.474f, -5.865f, -10.948f, -1.955f, -11.73001f, -2.737f, -7.82f, -8.211f, -3.91f, -8.993f};
static const float BARNES_CENTS[12] = {0.0f, -5.865f, -3.91f, -1.955f, -7.82f, 1.955f, -7.82f, -1.955f, -3.91f, -5.865f, 0.0f, -5.865f};

static const TuningRatio JUST_5_LIMIT_RATIOS[12] = {{1, 1}, {16, 15}, {9, 8}, {6, 5}, {5, 4}, {4, 3}, {45, 32}, {3, 2}, {8, 5}, {5, 3}, {9, 5}, {15, 8}};
static const TuningRatio JUST_7_LIMIT_RATIOS[12] = {{1, 1}, {21, 20}, {9, 8}, {7, 6}, {5, 4}, {4, 3}, {7, 5}, {3, 2}, {14, 9}, {5, 3}, {7, 4}, {15, 8}};
static const TuningRatio JUST_11_LIMIT_RATIOS[12] = {{1, 1}, {33, 32}, {9, 8}, {7, 6}, {5, 4}, {4, 3}, {11, 8}, {3, 2}, {14, 9}, {5, 3}, {7, 4}, {15, 8}};
static const TuningRatio CARLOS_SUPER_JUST_RATIOS[12] = {{1, 1}, {17, 16}, {9, 8}, {6, 5}, {5, 4}, {4, 3}, {11, 8}, {3, 2}, {13, 8}, {5, 3}, {7, 4}, {15, 8}};

static const TemperamentPreset TEMPERAMENT_PRESETS[] = {
	{"initialize", "INITIALIZE", TEMPERAMENT_INITIALIZE, TEMPERAMENT_CENTS, INITIALIZE_CENTS, nullptr, 0, 0},
	{"western.pythagorean", "PYTH", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, PYTH_CENTS, nullptr, 0, 2},
	{"western.meantone-quarter", "MEAN1/4", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, MEAN_QUARTER_CENTS, nullptr, 0, 3},
	{"western.meantone-sixth", "MEAN1/6", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, MEAN_SIXTH_CENTS, nullptr, 0, 4},
	{"western.werckmeister-iii", "WERCK3", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, WERCKMEISTER_III_CENTS, nullptr, 0, 5},
	{"western.werckmeister-iv", "WERCK4", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, WERCKMEISTER_IV_CENTS, nullptr, 0, 6},
	{"western.kirnberger-iii", "KIRN3", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, KIRNBERGER_III_CENTS, nullptr, 0, 7},
	{"western.vallotti", "VALLOT", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, VALLOTTI_CENTS, nullptr, 0, 8},
	{"western.young", "YOUNG", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, YOUNG_CENTS, nullptr, 0, 9},
	{"western.kellner", "KELLNR", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, KELLNER_CENTS, nullptr, 0, 10},
	{"western.barnes", "BARNES", TEMPERAMENT_WESTERN, TEMPERAMENT_CENTS, BARNES_CENTS, nullptr, 0, 11},
	{"just.5-limit", "JI 5-LIMIT", TEMPERAMENT_JUST, TEMPERAMENT_RATIOS, nullptr, JUST_5_LIMIT_RATIOS, 0, 1},
	{"just.7-limit", "JI 7-LIMIT", TEMPERAMENT_JUST, TEMPERAMENT_RATIOS, nullptr, JUST_7_LIMIT_RATIOS, 0, 38},
	{"just.11-limit", "JI 11-LIMIT", TEMPERAMENT_JUST, TEMPERAMENT_RATIOS, nullptr, JUST_11_LIMIT_RATIOS, 0, 39},
	{"just.carlos-super", "CARLOS SJ", TEMPERAMENT_JUST, TEMPERAMENT_RATIOS, nullptr, CARLOS_SUPER_JUST_RATIOS, 0, 0},
	{"edo.17-nearest", "17-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 17, 12},
	{"edo.19-nearest", "19-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 19, 13},
	{"edo.22-nearest", "22-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 22, 14},
	{"edo.26-nearest", "26-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 26, 0},
	{"edo.27-nearest", "27-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 27, 0},
	{"edo.31-nearest", "31-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 31, 16},
	{"edo.34-nearest", "34-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 34, 0},
	{"edo.53-nearest", "53-EDO MAP", TEMPERAMENT_EDO, TEMPERAMENT_EDO_MAP, nullptr, nullptr, 53, 17}
};

static const int NUM_TEMPERAMENTS = sizeof(TEMPERAMENT_PRESETS) / sizeof(TEMPERAMENT_PRESETS[0]);
static const int CUSTOM_TEMPERAMENT = -1;

static bool isValidTemperamentPreset(int presetIndex) {
	return presetIndex >= 0 && presetIndex < NUM_TEMPERAMENTS;
}

static float getTemperamentPresetCents(int presetIndex, int pitchClass) {
	if (!isValidTemperamentPreset(presetIndex) || pitchClass < 0 || pitchClass >= 12)
		return 0.0f;
	const TemperamentPreset& preset = TEMPERAMENT_PRESETS[presetIndex];
	if (preset.kind == TEMPERAMENT_CENTS)
		return preset.cents[pitchClass];
	if (preset.kind == TEMPERAMENT_RATIOS) {
		const TuningRatio& ratio = preset.ratios[pitchClass];
		return 1200.0f * std::log2((float)ratio.numerator / (float)ratio.denominator) - 100.0f * pitchClass;
	}
	int scaledDegree = pitchClass * preset.edo;
	int degree = scaledDegree / 12;
	// Exact half-degree ties stay below so every EDO map has one deterministic rule.
	if ((scaledDegree % 12) > 6)
		degree++;
	return 1200.0f * degree / preset.edo - 100.0f * pitchClass;
}

static int findTemperamentPresetById(const char* id) {
	if (!id)
		return CUSTOM_TEMPERAMENT;
	for (int i = 0; i < NUM_TEMPERAMENTS; i++) {
		if (std::strcmp(id, TEMPERAMENT_PRESETS[i].id) == 0)
			return i;
	}
	return CUSTOM_TEMPERAMENT;
}

static int findTemperamentPresetByLegacyIndex(int legacyIndex) {
	for (int i = 0; i < NUM_TEMPERAMENTS; i++) {
		if (TEMPERAMENT_PRESETS[i].legacyIndex == legacyIndex && (legacyIndex != 0 || i == 0))
			return i;
	}
	return CUSTOM_TEMPERAMENT;
}

static bool temperamentPresetMatches(const float* cents, int presetIndex, float tolerance = 0.001f) {
	if (!cents || !isValidTemperamentPreset(presetIndex))
		return false;
	for (int i = 0; i < 12; i++) {
		if (std::fabs(cents[i] - getTemperamentPresetCents(presetIndex, i)) > tolerance)
			return false;
	}
	return true;
}

static int findMatchingTemperamentPreset(const float* cents) {
	for (int i = 0; i < NUM_TEMPERAMENTS; i++) {
		if (temperamentPresetMatches(cents, i))
			return i;
	}
	return CUSTOM_TEMPERAMENT;
}


struct TemperamentSelector : OpaqueWidget {
	std::atomic<int>* currentPreset = nullptr;
	std::atomic<int>* pendingPreset = nullptr;
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

		int idx = currentPreset ? currentPreset->load(std::memory_order_relaxed) : 0;
		const char* name = isValidTemperamentPreset(idx) ? TEMPERAMENT_PRESETS[idx].name : "CUSTOM";

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
		if (!pendingPreset || !isValidTemperamentPreset(idx))
			return;
		pendingPreset->store(idx, std::memory_order_relaxed);
	}

	void createPresetSubmenu(ui::Menu* menu, const char* label, TemperamentCategory category) {
		menu->addChild(createSubmenuItem(label, "", [=](ui::Menu* subMenu) {
			for (int i = 0; i < NUM_TEMPERAMENTS; i++) {
				if (TEMPERAMENT_PRESETS[i].category != category)
					continue;
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

				if (!currentPreset || !pendingPreset) {
					e.consume(this);
					return;
				}

				int current = currentPreset->load(std::memory_order_relaxed);
				if (e.pos.x < arrowZone) {
					applyPreset(isValidTemperamentPreset(current) ? (current - 1 + NUM_TEMPERAMENTS) % NUM_TEMPERAMENTS : NUM_TEMPERAMENTS - 1);
				} else if (e.pos.x > box.size.x - arrowZone) {
					applyPreset(isValidTemperamentPreset(current) ? (current + 1) % NUM_TEMPERAMENTS : 0);
				} else {
					ui::Menu* menu = createMenu();
					menu->addChild(createMenuLabel("Microtuning presets"));
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuItem("Initialize (12-TET)", "", [=]() { applyPreset(0); }));
					createPresetSubmenu(menu, "Western Historical", TEMPERAMENT_WESTERN);
					createPresetSubmenu(menu, "Just Intonation", TEMPERAMENT_JUST);
					createPresetSubmenu(menu, "EDO Maps", TEMPERAMENT_EDO);
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
	std::atomic<int>* selectedKey = nullptr;

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

		int selected = selectedKey ? selectedKey->load(std::memory_order_relaxed) : -1;

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
						if (selectedKey) selectedKey->store(blackKeyNotes[i], std::memory_order_relaxed);
						e.consume(this);
						return;
					}
				}
			}

			int whiteKeyIndex = (int)(x / whiteKeyWidth);
			if (whiteKeyIndex >= 0 && whiteKeyIndex < 7) {
				if (selectedKey) selectedKey->store(whiteKeyNotes[whiteKeyIndex], std::memory_order_relaxed);
				e.consume(this);
			}
		}
	}
};

//***********************************************************************************************
// TwinSlide Expander - CV/Gate I/O expansion module
// 8HP expander for TwinSlide dual acid bass synthesizer
//***********************************************************************************************

#include "TwinSlidePlugin.hpp"
#include "../shared/include/TCLogo.hpp"

// Use VCV Rack's built-in dark port
using DarkPJ301MPort = rack::componentlibrary::DarkPJ301MPort;

struct TwinSlideExpander : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CVA_INPUT,
		GATEA_INPUT,
		SLIDEA_INPUT,
		FINEA_INPUT,
		CVB_INPUT,
		GATEB_INPUT,
		SLIDEB_INPUT,
		FINEB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CVA_OUTPUT,
		GATEA_OUTPUT,
		SLIDEA_OUTPUT,
		CVB_OUTPUT,
		GATEB_OUTPUT,
		SLIDEB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// Expander messages: [0-7] voltages (CV,Gate,Slide,Fine x2), [8] presence, [9-16] connection flags
	float leftMessages[2][18] = {};

	// Connection indicator fade (0.0 = white, 1.0 = amber)
	float connectionFade = 0.0f;

	TwinSlideExpander() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(CVA_INPUT, "Track A CV");
		configInput(GATEA_INPUT, "Track A Gate");
		configInput(SLIDEA_INPUT, "Track A Slide");
		configInput(FINEA_INPUT, "Track A Fine Tune CV");
		configInput(CVB_INPUT, "Track B CV");
		configInput(GATEB_INPUT, "Track B Gate");
		configInput(SLIDEB_INPUT, "Track B Slide");
		configInput(FINEB_INPUT, "Track B Fine Tune CV");

		configOutput(CVA_OUTPUT, "Track A CV");
		configOutput(GATEA_OUTPUT, "Track A Gate");
		configOutput(SLIDEA_OUTPUT, "Track A Slide");
		configOutput(CVB_OUTPUT, "Track B CV");
		configOutput(GATEB_OUTPUT, "Track B Gate");
		configOutput(SLIDEB_OUTPUT, "Track B Slide");

		leftExpander.producerMessage = leftMessages[0];
		leftExpander.consumerMessage = leftMessages[1];
	}

	void process(const ProcessArgs &args) override {
		bool motherPresent = leftExpander.module && leftExpander.module->model == modelTwinSlide;

		if (motherPresent) {
			// SEND TO MOTHER: Write to mother's rightExpander.producerMessage
			float *toMother = static_cast<float*>(leftExpander.module->rightExpander.producerMessage);
			// Input voltages: [0-3] Track A (CV, Gate, Slide, Fine), [4-7] Track B
			// All inputs clamped to safe ranges
			toMother[0] = clamp(inputs[CVA_INPUT].getVoltage(), -10.f, 10.f);
			toMother[1] = clamp(inputs[GATEA_INPUT].getVoltage(), -10.f, 10.f);
			toMother[2] = clamp(inputs[SLIDEA_INPUT].getVoltage(), -10.f, 10.f);
			toMother[3] = clamp(inputs[FINEA_INPUT].getVoltage(), -10.f, 10.f);
			toMother[4] = clamp(inputs[CVB_INPUT].getVoltage(), -10.f, 10.f);
			toMother[5] = clamp(inputs[GATEB_INPUT].getVoltage(), -10.f, 10.f);
			toMother[6] = clamp(inputs[SLIDEB_INPUT].getVoltage(), -10.f, 10.f);
			toMother[7] = clamp(inputs[FINEB_INPUT].getVoltage(), -10.f, 10.f);
			toMother[8] = 1.0f;  // Presence flag
			// Connection flags: [9-12] Track A, [13-16] Track B
			toMother[9] = inputs[CVA_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[10] = inputs[GATEA_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[11] = inputs[SLIDEA_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[12] = inputs[FINEA_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[13] = inputs[CVB_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[14] = inputs[GATEB_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[15] = inputs[SLIDEB_INPUT].isConnected() ? 1.0f : 0.0f;
			toMother[16] = inputs[FINEB_INPUT].isConnected() ? 1.0f : 0.0f;

			// REQUEST FLIP on mother's right side
			leftExpander.module->rightExpander.messageFlipRequested = true;

			// RECEIVE FROM MOTHER: Read OWN leftExpander.consumerMessage
			float *fromMother = static_cast<float*>(leftExpander.consumerMessage);
			outputs[CVA_OUTPUT].setVoltage(fromMother[0]);
			outputs[GATEA_OUTPUT].setVoltage(fromMother[1]);
			outputs[SLIDEA_OUTPUT].setVoltage(fromMother[2]);
			outputs[CVB_OUTPUT].setVoltage(fromMother[3]);
			outputs[GATEB_OUTPUT].setVoltage(fromMother[4]);
			outputs[SLIDEB_OUTPUT].setVoltage(fromMother[5]);
		}
		else {
			outputs[CVA_OUTPUT].setVoltage(0.0f);
			outputs[GATEA_OUTPUT].setVoltage(0.0f);
			outputs[SLIDEA_OUTPUT].setVoltage(0.0f);
			outputs[CVB_OUTPUT].setVoltage(0.0f);
			outputs[GATEB_OUTPUT].setVoltage(0.0f);
			outputs[SLIDEB_OUTPUT].setVoltage(0.0f);
		}

		// Smooth fade for connection indicator (200ms fade time)
		float targetFade = motherPresent ? 1.0f : 0.0f;
		float fadeSpeed = 5.0f;  // 5 units/sec = 200ms fade
		connectionFade += (targetFade - connectionFade) * fadeSpeed * args.sampleTime;
	}
};


struct ControlLabel : Widget {
	std::string text;
	ControlLabel(std::string t) : text(t) {}
	void draw(const DrawArgs& args) override {
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
		if (!font) return;
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, 10.0f);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, dx * 0.5f, dy * 0.5f, text.c_str(), NULL);
				}
			}
		}
		nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
		nvgText(args.vg, 0, 0, text.c_str(), NULL);
	}
};

struct TwinSlideExpanderWidget : ModuleWidget {
	TwinSlideExpanderWidget(TwinSlideExpander *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TwinSlideExpander.svg")));

		// Screws (TC Design: dark screws) - moved 1 hole inward
		addChild(createWidget<ScrewBlack>(Vec(15, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 30, 365)));

		// TC Logo at bottom (FULL style for 8HP)
		TCLogoWidget* tcLogo = new TCLogoWidget(TCLogoWidget::FULL, module);
		tcLogo->box.pos = Vec(box.size.x / 2.f - 3.f, 355);
		addChild(tcLogo);

		// Layout constants - 8HP = 40.64mm = 121.92px
		static const float colLeft = 30.f;   // Track A column
		static const float colRight = 92.f;  // Track B column
		// INPUT section jacks (aligned to amber grid)
		static const float inRow1 = 82.f;    // CV
		static const float inRow2 = 126.f;   // Gate
		static const float inRow3 = 170.f;   // Slide
		// OUTPUT section jacks
		static const float outRow1 = 235.f;
		static const float outRow2 = 279.f;  // Gate
		static const float outRow3 = 323.f;
		// Labels 22px above jacks
		static const float inLabel1 = 60.f;
		static const float inLabel2 = 104.f;
		static const float inLabel3 = 148.f;
		static const float outLabel1 = 213.f;
		static const float outLabel2 = 257.f;
		static const float outLabel3 = 301.f;

		// INPUT section - all 6 inputs (Track A left, Track B right)
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colLeft, inRow1), module, TwinSlideExpander::CVA_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colLeft, inRow2), module, TwinSlideExpander::GATEA_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colLeft, inRow3), module, TwinSlideExpander::SLIDEA_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colRight, inRow1), module, TwinSlideExpander::CVB_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colRight, inRow2), module, TwinSlideExpander::GATEB_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colRight, inRow3), module, TwinSlideExpander::SLIDEB_INPUT));

		// INPUT labels (ControlLabel style, 16px above jacks)
		ControlLabel* cvAInLabel = new ControlLabel("CV");
		cvAInLabel->box.pos = Vec(colLeft, inLabel1);
		addChild(cvAInLabel);
		ControlLabel* gateAInLabel = new ControlLabel("GATE");
		gateAInLabel->box.pos = Vec(colLeft, inLabel2);
		addChild(gateAInLabel);
		ControlLabel* slideAInLabel = new ControlLabel("SLIDE");
		slideAInLabel->box.pos = Vec(colLeft, inLabel3);
		addChild(slideAInLabel);
		ControlLabel* cvBInLabel = new ControlLabel("CV");
		cvBInLabel->box.pos = Vec(colRight, inLabel1);
		addChild(cvBInLabel);
		ControlLabel* gateBInLabel = new ControlLabel("GATE");
		gateBInLabel->box.pos = Vec(colRight, inLabel2);
		addChild(gateBInLabel);
		ControlLabel* slideBInLabel = new ControlLabel("SLIDE");
		slideBInLabel->box.pos = Vec(colRight, inLabel3);
		addChild(slideBInLabel);

		// FINE inputs (centered between A and B columns)
		static const float colCenter = 61.f;  // (colLeft + colRight) / 2
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colCenter, inRow2), module, TwinSlideExpander::FINEA_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(colCenter, inRow3), module, TwinSlideExpander::FINEB_INPUT));

		// FINE labels
		ControlLabel* fineALabel = new ControlLabel("FINE A");
		fineALabel->box.pos = Vec(colCenter, inLabel2);
		addChild(fineALabel);
		ControlLabel* fineBLabel = new ControlLabel("FINE B");
		fineBLabel->box.pos = Vec(colCenter, inLabel3);
		addChild(fineBLabel);

		// OUTPUT section - all 6 outputs (Track A left, Track B right)
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colLeft, outRow1), module, TwinSlideExpander::CVA_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colLeft, outRow2), module, TwinSlideExpander::GATEA_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colLeft, outRow3), module, TwinSlideExpander::SLIDEA_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colRight, outRow1), module, TwinSlideExpander::CVB_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colRight, outRow2), module, TwinSlideExpander::GATEB_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(colRight, outRow3), module, TwinSlideExpander::SLIDEB_OUTPUT));

		// OUTPUT labels (ControlLabel style, 16px above jacks)
		ControlLabel* cvAOutLabel = new ControlLabel("CV");
		cvAOutLabel->box.pos = Vec(colLeft, outLabel1);
		addChild(cvAOutLabel);
		ControlLabel* gateAOutLabel = new ControlLabel("GATE");
		gateAOutLabel->box.pos = Vec(colLeft, outLabel2);
		addChild(gateAOutLabel);
		ControlLabel* slideAOutLabel = new ControlLabel("SLIDE");
		slideAOutLabel->box.pos = Vec(colLeft, outLabel3);
		addChild(slideAOutLabel);
		ControlLabel* cvBOutLabel = new ControlLabel("CV");
		cvBOutLabel->box.pos = Vec(colRight, outLabel1);
		addChild(cvBOutLabel);
		ControlLabel* gateBOutLabel = new ControlLabel("GATE");
		gateBOutLabel->box.pos = Vec(colRight, outLabel2);
		addChild(gateBOutLabel);
		ControlLabel* slideBOutLabel = new ControlLabel("SLIDE");
		slideBOutLabel->box.pos = Vec(colRight, outLabel3);
		addChild(slideBOutLabel);

		// A/B column headers next to INPUT header (Y=47)
		ControlLabel* inputALabel = new ControlLabel("A");
		inputALabel->box.pos = Vec(colLeft, 47.f);
		addChild(inputALabel);
		ControlLabel* inputBLabel = new ControlLabel("B");
		inputBLabel->box.pos = Vec(colRight, 47.f);
		addChild(inputBLabel);

		// A/B column headers next to OUTPUT header (Y=210)
		ControlLabel* outputALabel = new ControlLabel("A");
		outputALabel->box.pos = Vec(colLeft, 202.f);
		addChild(outputALabel);
		ControlLabel* outputBLabel = new ControlLabel("B");
		outputBLabel->box.pos = Vec(colRight, 202.f);
		addChild(outputBLabel);
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);

		std::shared_ptr<Font> sonoBold = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Bold.ttf"));
		std::shared_ptr<Font> sonoMedium = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
		if (!sonoBold || !sonoMedium) return;

		float centerX = box.size.x / 2.f;

		nvgFontFaceId(args.vg, sonoBold->handle);
		nvgFontSize(args.vg, 18.0f);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgTextLetterSpacing(args.vg, 2.f);

		// Black outline for "TS"
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, 10.f + dy * 0.5f, "TS", NULL);
				}
			}
		}
		// White fill for "TS"
		nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
		nvgText(args.vg, centerX, 10.f, "TS", NULL);

		// "· ·" and " X " on second line - smooth color fade for dots
		nvgTextLetterSpacing(args.vg, 0.f);
		TwinSlideExpander* expModule = dynamic_cast<TwinSlideExpander*>(module);
		float fade = expModule ? expModule->connectionFade : 0.0f;

		// Interpolate color from white (255,255,255) to amber (255,192,80)
		uint8_t dotR = 255;
		uint8_t dotG = (uint8_t)(255 - fade * (255 - 192));  // 255 → 192
		uint8_t dotB = (uint8_t)(255 - fade * (255 - 80));   // 255 → 80
		uint8_t dotA = (uint8_t)(255 - fade * 76);           // 255 → 179 (70% opacity)

		// Black outline for dots "· ·"
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, 30.5f + dy * 0.5f, "· ·", NULL);
				}
			}
		}
		// Colored dots with opacity fade
		nvgFillColor(args.vg, nvgRGBA(dotR, dotG, dotB, dotA));
		nvgText(args.vg, centerX, 30.5f, "· ·", NULL);

		// Black outline for X
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, 30.5f + dy * 0.5f, " X ", NULL);
				}
			}
		}
		// White X (always white)
		nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
		nvgText(args.vg, centerX, 30.5f, " X ", NULL);

		// Section labels - Medium 10pt
		nvgFontFaceId(args.vg, sonoMedium->handle);
		nvgFontSize(args.vg, 10.0f);

		// INPUT header (TC Amber)
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, 47.f + dy * 0.5f, "INPUT", NULL);
				}
			}
		}
		nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
		nvgText(args.vg, centerX, 47.f, "INPUT", NULL);

		// OUTPUT header (TC Amber)
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, 202.f + dy * 0.5f, "OUTPUT", NULL);
				}
			}
		}
		nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));
		nvgText(args.vg, centerX, 202.f, "OUTPUT", NULL);
	}
};

Model *modelTwinSlideExpander = createModel<TwinSlideExpander, TwinSlideExpanderWidget>("TwinSlideExpander");

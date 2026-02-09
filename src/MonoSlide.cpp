//***********************************************************************************************
// MonoSlide - Standalone 303 synthesizer voice
// Mono acid bass synthesizer with external CV/Gate control
//***********************************************************************************************

#include "TwinSlidePlugin.hpp"
#include "SynthDSP.hpp"
#include "../shared/include/TCLogo.hpp"

using DarkPJ301MPort = rack::componentlibrary::DarkPJ301MPort;

struct MonoSlide : Module {
	enum ParamIds {
		CUTOFF_PARAM,
		ENVMOD_PARAM,
		DECAY_PARAM,
		DRIVE_PARAM,
		ACCENT_PARAM,
		RES_PARAM,
		FINE_PARAM,
		LEVEL_PARAM,
		WAVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CUTOFF_INPUT,
		ENVMOD_INPUT,
		DECAY_INPUT,
		DRIVE_INPUT,
		ACCENT_INPUT,
		FINE_INPUT,
		LEVEL_INPUT,
		CV_INPUT,
		GATE_INPUT,
		SLIDE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	SynthVoice voice;
	Trigger gateTrigger;
	bool prevGate = false;
	float slideTime = 0.75f;
	int prevWaveform = 0;
	float vcfRange = 0.0f;
	float decayRange = 0.0f;
	float sustain = 0.0f;
	float release = 0.0f;
	float blend = 0.0f;
	float bassComp = 1.0f;
	int lastTouchedParam = CUTOFF_PARAM;

	MonoSlide() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "Cutoff");
		configParam(ENVMOD_PARAM, 0.0f, 1.0f, 0.5f, "Env Mod");
		configParam(DECAY_PARAM, 0.0f, 1.0f, 0.5f, "Decay");
		configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.75f, "Drive");
		configParam(ACCENT_PARAM, 0.0f, 1.0f, 0.5f, "Accent");
		configParam(RES_PARAM, 0.0f, 1.0f, 0.0f, "Resonance");
		configParam(FINE_PARAM, -1.0f, 1.0f, 0.0f, "Fine Tune", " cents", 0.f, 700.f);
		configParam(LEVEL_PARAM, 0.0f, 1.0f, 0.75f, "Level", "%", 0.f, 100.f);
		configSwitch(WAVE_PARAM, 0.0f, 1.0f, 0.0f, "Waveform", {"Saw", "Square"});
		configInput(CUTOFF_INPUT, "Cutoff CV");
		configInput(ENVMOD_INPUT, "Env Mod CV");
		configInput(DECAY_INPUT, "Decay CV");
		configInput(DRIVE_INPUT, "Drive CV");
		configInput(ACCENT_INPUT, "Accent CV");
		configInput(FINE_INPUT, "Fine Tune CV");
		configInput(LEVEL_INPUT, "Level CV");
		configInput(CV_INPUT, "V/Oct");
		configInput(GATE_INPUT, "Gate");
		configInput(SLIDE_INPUT, "Slide");
		configOutput(AUDIO_OUTPUT, "Audio");
		voice.setSampleRate(APP->engine->getSampleRate());
	}

	void onReset() override {
		voice.reset();
		prevWaveform = 0;
		slideTime = 0.75f;
		vcfRange = 0.0f;
		decayRange = 0.0f;
		sustain = 0.0f;
		release = 0.0f;
		blend = 0.0f;
		bassComp = 1.0f;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "slideTime", json_real(slideTime));
		json_object_set_new(rootJ, "vcfRange", json_real(vcfRange));
		json_object_set_new(rootJ, "decayRange", json_real(decayRange));
		json_object_set_new(rootJ, "sustain", json_real(sustain));
		json_object_set_new(rootJ, "release", json_real(release));
		json_object_set_new(rootJ, "blend", json_real(blend));
		json_object_set_new(rootJ, "bassComp", json_real(bassComp));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* slideTimeJ = json_object_get(rootJ, "slideTime");
		if (slideTimeJ) slideTime = json_number_value(slideTimeJ);
		json_t* vcfRangeJ = json_object_get(rootJ, "vcfRange");
		if (vcfRangeJ) vcfRange = json_number_value(vcfRangeJ);
		json_t* decayRangeJ = json_object_get(rootJ, "decayRange");
		if (decayRangeJ) decayRange = json_number_value(decayRangeJ);
		json_t* sustainJ = json_object_get(rootJ, "sustain");
		if (sustainJ) sustain = json_number_value(sustainJ);
		json_t* releaseJ = json_object_get(rootJ, "release");
		if (releaseJ) release = json_number_value(releaseJ);
		json_t* blendJ = json_object_get(rootJ, "blend");
		if (blendJ) blend = json_number_value(blendJ);
		json_t* bassCompJ = json_object_get(rootJ, "bassComp");
		if (bassCompJ) bassComp = json_number_value(bassCompJ);
	}

	void onSampleRateChange() override {
		voice.setSampleRate(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs &args) override {
		if (!outputs[AUDIO_OUTPUT].isConnected()) return;

		float pitch = inputs[CV_INPUT].getVoltage();
		bool gate = inputs[GATE_INPUT].getVoltage() >= 1.0f;
		bool slide = inputs[SLIDE_INPUT].getVoltage() >= 1.0f;
		bool accent = clamp(params[ACCENT_PARAM].getValue() + clamp(inputs[ACCENT_INPUT].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f) > 0.7f;

		float cutoff = params[CUTOFF_PARAM].getValue();
		float cutoffCv = clamp(inputs[CUTOFF_INPUT].getVoltage(), -10.f, 10.f);
		float res = params[RES_PARAM].getValue();
		float envMod = clamp(params[ENVMOD_PARAM].getValue() + clamp(inputs[ENVMOD_INPUT].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);
		float decay = clamp(params[DECAY_PARAM].getValue() + clamp(inputs[DECAY_INPUT].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);
		float accentAmt = params[ACCENT_PARAM].getValue();
		float drive = clamp(params[DRIVE_PARAM].getValue() + clamp(inputs[DRIVE_INPUT].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);
		float fineCv = inputs[FINE_INPUT].getVoltage() / 5.0f;
		float fine = clamp(params[FINE_PARAM].getValue() + fineCv, -1.0f, 1.0f);
		float level = clamp(params[LEVEL_PARAM].getValue() + clamp(inputs[LEVEL_INPUT].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);

		int waveform = (int)params[WAVE_PARAM].getValue();
		if (waveform != prevWaveform) {
			blend = (waveform == 0) ? 0.0f : 1.0f;
			prevWaveform = waveform;
		}

		float audio = voice.process(pitch, gate, accent, slide,
		                            cutoff, cutoffCv, res, envMod, decay, accentAmt, drive, fine, blend, slideTime,
		                            vcfRange, decayRange, sustain, release,
		                            bassComp);
		float out = audio * std::min(level, 0.75f) / 0.75f;
		if (level > 0.75f) {
			float satAmt = (level - 0.75f) / 0.25f;
			float satDrive = 1.0f + satAmt * 3.0f;
			float normalized = out / 5.0f;
			normalized = std::tanh(normalized * satDrive) / std::tanh(satDrive);
			out = normalized * 5.0f;
		}
		outputs[AUDIO_OUTPUT].setVoltage(clamp(out, -10.f, 10.f));
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

struct ThinSwitchOutline : Widget {
	ThinSwitchOutline(Vec size, Vec pos) {
		box.size = size;
		box.pos = pos;
	}
	void draw(const DrawArgs& args) override {
		nvgBeginPath(args.vg);
		NVGpaint grad = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, colTop, colBot);
		nvgRoundedRect(args.vg, -0.5f, -0.5f, box.size.x + 1, box.size.y + 1, 1.0f);
		nvgFillPaint(args.vg, grad);
		nvgFill(args.vg);
		Widget::draw(args);
	}
};

struct WaveformSymbol : TransparentWidget {
	MonoSlide* module;
	float blend = -1.f;

	WaveformSymbol(MonoSlide* m) : module(m) {
		box.size = Vec(mm2px(2.0f), mm2px(6.0f));
	}

	void step() override {
		int waveform = module ? (int)module->params[MonoSlide::WAVE_PARAM].getValue() : 0;
		float target = (float)waveform;
		if (blend < 0.f) {
			blend = target;
		} else {
			blend += (target - blend) * 0.15f;
		}
		TransparentWidget::step();
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1) return;

		float symbolH = mm2px(2.0f);
		float gap = mm2px(2.0f);
		float sawAlpha = 1.0f - blend;
		float sqrAlpha = blend;

		nvgStrokeWidth(args.vg, mm2px(0.3f));
		nvgLineCap(args.vg, NVG_ROUND);
		nvgLineJoin(args.vg, NVG_ROUND);

		// Draw square at top position (visible when switch UP)
		if (sqrAlpha > 0.01f) {
			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, (int)(sqrAlpha * 204)));
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, 0, symbolH);
			nvgLineTo(args.vg, 0, 0);
			nvgLineTo(args.vg, box.size.x * 0.5f, 0);
			nvgLineTo(args.vg, box.size.x * 0.5f, symbolH);
			nvgLineTo(args.vg, box.size.x, symbolH);
			nvgLineTo(args.vg, box.size.x, 0);
			nvgStroke(args.vg);
		}

		// Draw saw at bottom position (visible when switch DOWN)
		if (sawAlpha > 0.01f) {
			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, (int)(sawAlpha * 204)));
			nvgBeginPath(args.vg);
			float y0 = symbolH + gap;
			nvgMoveTo(args.vg, 0, y0 + symbolH);
			nvgLineTo(args.vg, box.size.x * 0.5f, y0);
			nvgLineTo(args.vg, box.size.x * 0.5f, y0 + symbolH);
			nvgLineTo(args.vg, box.size.x, y0);
			nvgLineTo(args.vg, box.size.x, y0 + symbolH);
			nvgStroke(args.vg);
		}

		Widget::drawLayer(args, layer);
	}
};

struct ParamDisplay : TransparentWidget {
	MonoSlide* module;
	std::string paramNames[8] = {"FILTER", "ENVELOPE", "DECAY", "DRIVE", "ACCENT", "RESONANCE", "FINE TUNE", "LEVEL"};

	ParamDisplay(MonoSlide* m) : module(m) {
		box.size = Vec(mm2px(12.0f), mm2px(6.0f));
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1) return;
		if (!module) return;

		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
		if (!font) return;

		int paramId = module->lastTouchedParam;
		if (paramId < 0 || paramId > 7) return;

		float value = module->params[paramId].getValue();
		std::string name = paramNames[paramId];
		std::string valueStr;

		if (paramId == MonoSlide::FINE_PARAM) {
			valueStr = string::f("%.0f ct", value * 700.f);
		} else if (paramId == MonoSlide::LEVEL_PARAM) {
			valueStr = string::f("%.0f%%", value * 100.f);
		} else {
			valueStr = string::f("%.0f%%", value * 100.f);
		}

		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, nvgRGB(0xFF, 0xC0, 0x50));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

		float centerX = box.size.x / 2.0f;
		nvgFontSize(args.vg, 9.0f);
		nvgText(args.vg, centerX, 0, name.c_str(), NULL);

		nvgFontSize(args.vg, 9.0f);
		nvgText(args.vg, centerX, 10, valueStr.c_str(), NULL);

		Widget::drawLayer(args, layer);
	}
};

struct SmileyWidget : TransparentWidget {
	MonoSlide* module;
	float visibility = 0.f;

	SmileyWidget(MonoSlide* m) : module(m) {
		box.size = Vec(mm2px(6.5f), mm2px(6.5f));
	}

	void step() override {
		float target = (module && module->inputs[MonoSlide::SLIDE_INPUT].isConnected()) ? 1.f : 0.f;
		visibility += (target - visibility) * 0.15f;
		TransparentWidget::step();
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1) return;
		if (!module) return;
		if (visibility < 0.01f) return;

		float cutoff = module->params[MonoSlide::CUTOFF_PARAM].getValue();
		float envMod = module->params[MonoSlide::ENVMOD_PARAM].getValue();

		int alpha = (int)(204 * visibility);
		NVGcolor amber = nvgRGBA(0xFF, 0xC0, 0x50, alpha);
		float cx = box.size.x / 2.0f;
		float cy = box.size.y / 2.0f;
		float radius = box.size.x / 2.0f - 1.0f;

		// Face outline
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, cx, cy, radius);
		nvgStrokeColor(args.vg, amber);
		nvgStrokeWidth(args.vg, 1.0f);
		nvgStroke(args.vg);

		// Eyes - size based on cutoff (round at center, oval at max)
		float baseRadius = 1.2f + cutoff * 1.2f;
		float ovalStretch = (cutoff > 0.5f) ? (cutoff - 0.5f) * 0.8f : 0.f;
		float eyeRadiusX = baseRadius;
		float eyeRadiusY = baseRadius + ovalStretch * baseRadius;
		float eyeY = cy - radius * 0.25f;
		float eyeSpacing = radius * 0.4f;

		nvgBeginPath(args.vg);
		nvgEllipse(args.vg, cx - eyeSpacing, eyeY, eyeRadiusX, eyeRadiusY);
		nvgEllipse(args.vg, cx + eyeSpacing, eyeY, eyeRadiusX, eyeRadiusY);
		nvgStrokeColor(args.vg, amber);
		nvgStrokeWidth(args.vg, 0.8f);
		nvgStroke(args.vg);

		// Mouth - curve based on envMod (always smiling, more with higher envMod)
		float mouthY = cy + radius * 0.3f;
		float mouthWidth = radius * 0.6f;
		float curve = radius * (0.30f + envMod * 0.65f);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, cx - mouthWidth, mouthY);
		nvgQuadTo(args.vg, cx, mouthY + curve, cx + mouthWidth, mouthY);
		nvgStrokeColor(args.vg, amber);
		nvgStrokeWidth(args.vg, 1.2f);
		nvgStroke(args.vg);

		Widget::drawLayer(args, layer);
	}
};

struct ConnectionDisplay : TransparentWidget {
	MonoSlide* module;
	float cvVisibility = 0.f;
	float gateVisibility = 0.f;

	ConnectionDisplay(MonoSlide* m) : module(m) {
		box.size = Vec(mm2px(10.0f), mm2px(6.0f));
	}

	void step() override {
		if (!module) {
			TransparentWidget::step();
			return;
		}
		bool slideConnected = module->inputs[MonoSlide::SLIDE_INPUT].isConnected();

		float cvTarget = (!slideConnected && module->inputs[MonoSlide::CV_INPUT].isConnected()) ? 1.f : 0.f;
		float gateTarget = (!slideConnected && module->inputs[MonoSlide::GATE_INPUT].isConnected()) ? 1.f : 0.f;

		if (slideConnected) {
			cvVisibility = 0.f;
			gateVisibility = 0.f;
		} else {
			cvVisibility += (cvTarget - cvVisibility) * 0.15f;
			gateVisibility += (gateTarget - gateVisibility) * 0.15f;
		}
		TransparentWidget::step();
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1) return;
		if (!module) return;
		if (cvVisibility < 0.01f && gateVisibility < 0.01f) return;

		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
		if (!font) return;

		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, 9.0f);
		nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		if (cvVisibility > 0.01f) {
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, (int)(255 * cvVisibility)));
			nvgText(args.vg, 0, 0, "\xe2\x88\x9a CV", NULL);
		}

		if (gateVisibility > 0.01f) {
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, (int)(255 * gateVisibility)));
			nvgText(args.vg, 0, 10, "\xe2\x88\x9a GATE", NULL);
		}

		Widget::drawLayer(args, layer);
	}
};

// Tracking knob template - updates lastTouchedParam on drag
template <typename TBase>
struct TrackingKnob : TBase {
	MonoSlide* msModule = nullptr;
	bool hideTooltip = false;

	void onDragStart(const event::DragStart& e) override {
		TBase::onDragStart(e);
		if (msModule) {
			engine::ParamQuantity* pq = this->getParamQuantity();
			if (pq) {
				msModule->lastTouchedParam = pq->paramId;
			}
		}
	}

	void onEnter(const event::Enter& e) override {
		if (!hideTooltip) {
			TBase::onEnter(e);
		}
	}
};

// MSBigKnob - custom big knob using plugin's SVGs
struct MSBigKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	MSBigKnob() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/MSBigKnob.svg")));

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/MSBigKnob_bg.svg")));
	}
};

// Title widget matching TwinSlide style
struct MonoSlideTitleWidget : TransparentWidget {
	std::shared_ptr<Font> fontBold;

	MonoSlideTitleWidget() {
		box.size = Vec(0, 20);  // Width set by parent widget
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1) return;

		fontBold = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono-Bold.ttf"));
		if (!fontBold) return;

		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgFontFaceId(args.vg, fontBold->handle);
		nvgFontSize(args.vg, 18.f);
		nvgTextLetterSpacing(args.vg, 2.f);

		float centerX = box.size.x / 2.f;
		float centerY = box.size.y / 2.f;

		// Black outline
		nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx != 0 || dy != 0) {
					nvgText(args.vg, centerX + dx * 0.5f, centerY + dy * 0.5f, "MONO SLIDE", NULL);
				}
			}
		}
		// White fill
		nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
		nvgText(args.vg, centerX, centerY, "MONO SLIDE", NULL);
		nvgTextLetterSpacing(args.vg, 0.f);

		Widget::drawLayer(args, layer);
	}
};

struct MonoSlideWidget : ModuleWidget {
	// SYNTH CONTROLS section layout constants (sub-container: X=12-168px, Y=198-340px)
	static constexpr float col1x = 30.f;
	static constexpr float col2x = 75.f;
	static constexpr float col3x = 120.f;
	static constexpr float col4x = 165.f;
	static constexpr float col1y = 235.f;   // Top row
	static constexpr float col2y = 279.f;   // Middle row
	static constexpr float col3y = 323.f;

	MonoSlideWidget(MonoSlide *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/MonoSlide.svg")));

		// Screws (TC Design: dark screws) - 1 hole inward from corners
		addChild(createWidget<ScrewBlack>(Vec(15, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewBlack>(Vec(15, 365)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 30, 365)));

		// Title display at top - centered on module width
		MonoSlideTitleWidget *titleDisplay = new MonoSlideTitleWidget();
		titleDisplay->box.size.x = box.size.x;
		titleDisplay->box.pos = Vec(0, 0);
		addChild(titleDisplay);

		// TC Logo at bottom (FULL style) - centered at 12HP module width/2
		TCLogoWidget* tcLogo = new TCLogoWidget(TCLogoWidget::FULL, module);
		tcLogo->box.pos = Vec(box.size.x / 2.f - 3.f, 355);
		addChild(tcLogo);

		// Row 2 (Y=170): DRV, ACC, RES, FINE amber knobs
		auto* driveKnob = createParamCentered<TrackingKnob<C1Knob280>>(Vec(col1x, 170.f), module, MonoSlide::DRIVE_PARAM);
		driveKnob->msModule = module;
		driveKnob->hideTooltip = true;
		addParam(driveKnob);
		LedRingOverlay* driveRing = new LedRingOverlay(module, MonoSlide::DRIVE_PARAM);
		driveRing->box.pos = Vec(col1x - 25, 145.f);
		addChild(driveRing);

		auto* accentKnob = createParamCentered<TrackingKnob<C1Knob280>>(Vec(col2x, 170.f), module, MonoSlide::ACCENT_PARAM);
		accentKnob->msModule = module;
		accentKnob->hideTooltip = true;
		addParam(accentKnob);
		LedRingOverlay* accentRing = new LedRingOverlay(module, MonoSlide::ACCENT_PARAM);
		accentRing->box.pos = Vec(col2x - 25, 145.f);
		addChild(accentRing);

		auto* resKnob = createParamCentered<TrackingKnob<C1Knob280>>(Vec(col3x, 170.f), module, MonoSlide::RES_PARAM);
		resKnob->msModule = module;
		resKnob->hideTooltip = true;
		addParam(resKnob);
		LedRingOverlay* resRing = new LedRingOverlay(module, MonoSlide::RES_PARAM);
		resRing->box.pos = Vec(col3x - 25, 145.f);
		addChild(resRing);

		auto* fineKnob = createParamCentered<TrackingKnob<C1Knob280>>(Vec(col4x, 170.f), module, MonoSlide::FINE_PARAM);
		fineKnob->msModule = module;
		fineKnob->hideTooltip = true;
		addParam(fineKnob);
		LedRingOverlay* fineRing = new LedRingOverlay(module, MonoSlide::FINE_PARAM);
		fineRing->box.pos = Vec(col4x - 25, 145.f);
		addChild(fineRing);

		ControlLabel* drvKnobLabel = new ControlLabel("DRV");
		drvKnobLabel->box.pos = Vec(col1x, 148.f);
		addChild(drvKnobLabel);

		ControlLabel* accKnobLabel = new ControlLabel("ACC");
		accKnobLabel->box.pos = Vec(col2x, 148.f);
		addChild(accKnobLabel);

		ControlLabel* resKnobLabel = new ControlLabel("RES");
		resKnobLabel->box.pos = Vec(col3x, 148.f);
		addChild(resKnobLabel);

		ControlLabel* fineKnobLabel = new ControlLabel("FINE");
		fineKnobLabel->box.pos = Vec(col4x, 148.f);
		addChild(fineKnobLabel);

		// Row 1 (Y=77): VCF, ENV, DEC big knobs (tooltips hidden)
		auto* cutoffKnob = createParamCentered<TrackingKnob<MSBigKnob>>(Vec(52.5f, 77.f), module, MonoSlide::CUTOFF_PARAM);
		cutoffKnob->msModule = module;
		cutoffKnob->hideTooltip = true;
		addParam(cutoffKnob);

		ControlLabel* cutoffKnobLabel = new ControlLabel("VCF");
		cutoffKnobLabel->box.pos = Vec(52.5f, 45.f);
		addChild(cutoffKnobLabel);

		auto* envmodKnob = createParamCentered<TrackingKnob<MSBigKnob>>(Vec(97.5f, 77.f), module, MonoSlide::ENVMOD_PARAM);
		envmodKnob->msModule = module;
		envmodKnob->hideTooltip = true;
		addParam(envmodKnob);

		ControlLabel* envmodKnobLabel = new ControlLabel("ENV");
		envmodKnobLabel->box.pos = Vec(97.5f, 45.f);
		addChild(envmodKnobLabel);

		auto* decayKnob = createParamCentered<TrackingKnob<MSBigKnob>>(Vec(142.5f, 77.f), module, MonoSlide::DECAY_PARAM);
		decayKnob->msModule = module;
		decayKnob->hideTooltip = true;
		addParam(decayKnob);

		ControlLabel* decayKnobLabel = new ControlLabel("DEC");
		decayKnobLabel->box.pos = Vec(142.5f, 45.f);
		addChild(decayKnobLabel);

		// Waveform switch (inside display area)
		SvgPanel* svgPanel = dynamic_cast<SvgPanel*>(getPanel());
		auto* waveSwitch = createParamCentered<TS2switchVn>(Vec(40, 119), module, MonoSlide::WAVE_PARAM);
		svgPanel->fb->addChild(new ThinSwitchOutline(waveSwitch->box.size, waveSwitch->box.pos));
		addParam(waveSwitch);

		// Waveform symbol in display, aligned with switch positions
		WaveformSymbol* waveSymbol = new WaveformSymbol(module);
		waveSymbol->box.pos = Vec(50, 119 - waveSymbol->box.size.y / 2.0f);
		addChild(waveSymbol);

		// Parameter display (centered under ENV knob)
		ParamDisplay* paramDisplay = new ParamDisplay(module);
		paramDisplay->box.pos = Vec(97.5f - paramDisplay->box.size.x / 2.0f, 119 - paramDisplay->box.size.y / 2.0f);
		addChild(paramDisplay);

		// Smiley widget (centered under DEC knob)
		SmileyWidget* smiley = new SmileyWidget(module);
		smiley->box.pos = Vec(142.5f - smiley->box.size.x / 2.0f, 119 - smiley->box.size.y / 2.0f);
		addChild(smiley);

		// Connection display (centered under DEC knob, hidden when slide connected)
		ConnectionDisplay* connDisplay = new ConnectionDisplay(module);
		connDisplay->box.pos = Vec(142.5f - connDisplay->box.size.x / 2.0f, 119 - connDisplay->box.size.y / 2.0f);
		addChild(connDisplay);

		// Level knob at SYNTH CONTROLS top row (Y=col1y)
		auto* levelKnob = createParamCentered<TrackingKnob<C1Knob280>>(Vec(col4x, col1y), module, MonoSlide::LEVEL_PARAM);
		levelKnob->msModule = module;
		levelKnob->hideTooltip = true;
		addParam(levelKnob);
		LedRingOverlay* levelRing = new LedRingOverlay(module, MonoSlide::LEVEL_PARAM);
		levelRing->box.pos = Vec(col4x - 25, col1y - 25);
		addChild(levelRing);

		ControlLabel* levelKnobLabel = new ControlLabel("LEVEL");
		levelKnobLabel->box.pos = Vec(col4x, col1y - 22.f);
		addChild(levelKnobLabel);

		// SYNTH CONTROLS top row: VCF, ENV, DEC inputs (Y=col1y)
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col1x, col1y), module, MonoSlide::CUTOFF_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col2x, col1y), module, MonoSlide::ENVMOD_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col3x, col1y), module, MonoSlide::DECAY_INPUT));

		// Labels 22px above jacks
		ControlLabel* vcfLabel = new ControlLabel("VCF");
		vcfLabel->box.pos = Vec(col1x, col1y - 22.f);
		addChild(vcfLabel);
		ControlLabel* envLabel = new ControlLabel("ENV");
		envLabel->box.pos = Vec(col2x, col1y - 22.f);
		addChild(envLabel);
		ControlLabel* decLabel = new ControlLabel("DEC");
		decLabel->box.pos = Vec(col3x, col1y - 22.f);
		addChild(decLabel);

		// Middle row: DRV, ACC, FINE, LEVEL inputs
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col1x, col2y), module, MonoSlide::DRIVE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col2x, col2y), module, MonoSlide::ACCENT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col3x, col2y), module, MonoSlide::FINE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col4x, col2y), module, MonoSlide::LEVEL_INPUT));

		// Labels 22px above jacks
		ControlLabel* driveLabel = new ControlLabel("DRV");
		driveLabel->box.pos = Vec(col1x, col2y - 22.f);
		addChild(driveLabel);
		ControlLabel* accentLabel = new ControlLabel("ACC");
		accentLabel->box.pos = Vec(col2x, col2y - 22.f);
		addChild(accentLabel);
		ControlLabel* fineLabel = new ControlLabel("FINE");
		fineLabel->box.pos = Vec(col3x, col2y - 22.f);
		addChild(fineLabel);
		ControlLabel* levelLabel = new ControlLabel("LEVEL");
		levelLabel->box.pos = Vec(col4x, col2y - 22.f);
		addChild(levelLabel);

		// Bottom row: CV, GATE, SLIDE inputs + AUDIO output
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col1x, col3y), module, MonoSlide::CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col2x, col3y), module, MonoSlide::GATE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(Vec(col3x, col3y), module, MonoSlide::SLIDE_INPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(Vec(col4x, col3y), module, MonoSlide::AUDIO_OUTPUT));

		// Labels 22px above jacks
		ControlLabel* cvLabel = new ControlLabel("CV");
		cvLabel->box.pos = Vec(col1x, col3y - 22.f);
		addChild(cvLabel);
		ControlLabel* gateLabel = new ControlLabel("GATE");
		gateLabel->box.pos = Vec(col2x, col3y - 22.f);
		addChild(gateLabel);
		ControlLabel* slideLabel = new ControlLabel("SLIDE");
		slideLabel->box.pos = Vec(col3x, col3y - 22.f);
		addChild(slideLabel);
		ControlLabel* audioLabel = new ControlLabel("OUT");
		audioLabel->box.pos = Vec(col4x, col3y - 22.f);
		addChild(audioLabel);
	}

	struct ExpertQuantity : Quantity {
		float* value;
		float defaultValue;
		float minVal, maxVal;
		std::string label;
		std::function<std::string(float)> formatFunc;

		ExpertQuantity(float* val, float def, float minV, float maxV,
		               const std::string& lbl, std::function<std::string(float)> fmt)
		    : value(val), defaultValue(def), minVal(minV), maxVal(maxV), label(lbl), formatFunc(fmt) {}

		void setValue(float v) override { *value = clamp(v, minVal, maxVal); }
		float getValue() override { return *value; }
		float getDefaultValue() override { return defaultValue; }
		float getMinValue() override { return minVal; }
		float getMaxValue() override { return maxVal; }
		std::string getLabel() override { return label; }
		std::string getDisplayValueString() override { return formatFunc(*value); }
	};

	struct ExpertSlider : ui::Slider {
		ExpertSlider(float* val, float def, float minV, float maxV,
		             const std::string& lbl, std::function<std::string(float)> fmt) {
			quantity = new ExpertQuantity(val, def, minV, maxV, lbl, fmt);
			box.size.x = 200.0f;
		}
		~ExpertSlider() { delete quantity; }
	};

	void appendContextMenu(Menu* menu) override {
		MonoSlide* module = dynamic_cast<MonoSlide*>(this->module);
		if (!module) return;

		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("Expert Mode", "", [=](Menu* menu) {
			menu->addChild(new ExpertSlider(
				&module->vcfRange, 0.0f, -1.0f, 0.0f, "VCF Range",
				[](float v) {
					float t = v + 1.0f;
					float vcfMin = 100.0f * std::pow(314.0f / 100.0f, t);
					float vcfMax = 760.0f * std::pow(2394.0f / 760.0f, t);
					return string::f("%.0f-%.0f Hz", vcfMin, vcfMax);
				}
			));
			menu->addChild(new ExpertSlider(
				&module->decayRange, 0.0f, -1.0f, 1.0f, "Decay Speed",
				[](float v) {
					float mult = 4.0f * (1.0f - v * 0.5f);
					return string::f("%.1fx", mult);
				}
			));
			menu->addChild(new ExpertSlider(
				&module->sustain, 0.0f, 0.0f, 1.0f, "Sustain",
				[](float v) { return string::f("%.0f%%", v * 100.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->release, 0.0f, 0.0f, 1.0f, "Release",
				[](float v) {
					float ms = 21.0f + v * 479.0f;
					return string::f("%.0f ms", ms);
				}
			));
			menu->addChild(new ExpertSlider(
				&module->blend, 0.0f, 0.0f, 1.0f, "Blend",
				[](float v) { return string::f("%.0f%% Sqr", v * 100.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->slideTime, 0.75f, 0.0f, 2.0f, "Slide Time",
				[](float v) { return string::f("%.0f ms", v * 50.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->bassComp, 1.0f, 0.0f, 1.0f, "Bass Comp",
				[](float v) { return string::f("%.0f%%", (1.0f - v) * 100.0f); }
			));
		}));
	}
};

Model *modelMonoSlide = createModel<MonoSlide, MonoSlideWidget>("MonoSlide");

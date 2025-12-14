//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************

#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin *pluginInstance;

// LED colors
extern NVGcolor SCHEME_RED_TS;
extern NVGcolor SCHEME_GREEN_TS;

static const NVGcolor colTop = nvgRGBA(138, 96, 32, 179);
static const NVGcolor colBot = nvgRGBA(230, 176, 96, 179);



// Lights
// ----------

struct WhiteLightTS : GrayModuleLightWidget {
	WhiteLightTS() {
		addBaseColor(SCHEME_WHITE);
	}
};

struct RedLightTS : GrayModuleLightWidget {
	RedLightTS() {
		addBaseColor(SCHEME_RED_TS);
	}
};

struct GreenRedLightTS : GrayModuleLightWidget {
	GreenRedLightTS() {
		addBaseColor(SCHEME_GREEN_TS);
		addBaseColor(SCHEME_RED_TS);
	}
};

struct GreenRedWhiteLightTS : GrayModuleLightWidget {
	GreenRedWhiteLightTS() {
		addBaseColor(SCHEME_GREEN_TS);
		addBaseColor(SCHEME_RED_TS);
		addBaseColor(SCHEME_WHITE);
	}
};



// Helpers
// ----------------------------------------

template <class TDynamicPort>
TDynamicPort* createDynamicPortCentered(Vec pos, bool isInput, Module *module, int portId, int* /*mode*/) {
	TDynamicPort *dynPort = isInput ?
		createInput<TDynamicPort>(pos, module, portId) :
		createOutput<TDynamicPort>(pos, module, portId);
	dynPort->box.pos = dynPort->box.pos.minus(dynPort->box.size.div(2));
	return dynPort;
}

template <class TDynamicParam>
TDynamicParam* createDynamicParamCentered(Vec pos, Module *module, int paramId, int* /*mode*/) {
	TDynamicParam *dynParam = createParam<TDynamicParam>(pos, module, paramId);
	dynParam->box.pos = dynParam->box.pos.minus(dynParam->box.size.div(2));
	return dynParam;
}



// Variations on existing knobs, lights, etc
// ----------------------------------------

// Ports
// ----------

struct DynamicSVGPort : SvgPort {
    std::vector<std::shared_ptr<Svg>> frames;

    void addFrame(std::shared_ptr<Svg> svg);
    void step() override;
};


struct JackPort : DynamicSVGPort  {
	JackPort() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/PJ301M-dark.svg")));
	}
};



// Buttons and switches
// ----------

struct LEDButtonSmall : LEDButton {
	TransformWidget *tw;
	LEDButtonSmall() {
		float ratio = 0.75f;
		sw->box.size = sw->box.size.mult(ratio);
		fb->removeChild(sw);
		tw = new TransformWidget();
		tw->addChild(sw);
		tw->scale(Vec(ratio, ratio));
		tw->box.size = sw->box.size;
		fb->addChild(tw);
		box.size = sw->box.size;
		shadow->box.size = sw->box.size;
	}
};

template <typename TLight>
struct SmallLightTS : TLight {
	SmallLightTS() {
		this->box.size = mm2px(Vec(1.6f, 1.6f));
	}
};


struct IMBigPushButton : CKD6 {
	TransformWidget *tw;
	IMBigPushButton() {
		setSizeRatio(0.9f);
	}
	void setSizeRatio(float ratio) {
		sw->box.size = sw->box.size.mult(ratio);
		fb->removeChild(sw);
		tw = new TransformWidget();
		tw->addChild(sw);
		tw->scale(Vec(ratio, ratio));
		tw->box.size = sw->box.size; 
		fb->addChild(tw);
		box.size = sw->box.size; 
		shadow->box.size = sw->box.size; 
	}
};


struct SwitchOutlineWidget : Widget {
	SwitchOutlineWidget(int* /*_mode*/, Vec _size, Vec _pos) {
		box.size = _size;
		box.pos = _pos;
	}
	void draw(const DrawArgs& args) override;
};

template <class TDynamicParam>
TDynamicParam* createDynamicSwitchCentered(Vec pos, Module *module, int paramId, int* /*mode*/, SvgPanel* panel) {
	TDynamicParam *dynParam = createParam<TDynamicParam>(pos, module, paramId);
	dynParam->box.pos = dynParam->box.pos.minus(dynParam->box.size.div(2));
	SwitchOutlineWidget* sow = new SwitchOutlineWidget(nullptr, dynParam->box.size, dynParam->box.pos);
	panel->fb->addChild(sow);
	return dynParam;
}

struct TS2switchV : app::SvgSwitch {
	TS2switchV();
};

struct TS2switchVInv : app::SvgSwitch {
	TS2switchVInv();
};

struct TS2switchVn : app::SvgSwitch {
	TS2switchVn();
};

struct IMSwitch2H : CKSS {
	IMSwitch2H();
};

struct IMSwitch3VInv : SvgSwitch {
	IMSwitch3VInv() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));
	}
};

struct IMSwitch3H : SvgSwitch {
	IMSwitch3H();
};




// Knobs
// ----------

struct Rogan1SWhite : Rogan {
	Rogan1SWhite() {
		// setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite.svg")));
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/Rogan1S.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/Rogan1PS_bg.svg")));
		// fg->setSvg(Svg::load(asset::system("res/ComponentLibrary/Rogan1PSWhite_fg.svg")));
		fg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/Rogan1PSWhite_fg.svg")));
	}
};


struct IMBigKnobInf : Rogan1SWhite {
	IMBigKnobInf() {
		speed = 0.9f;
	}
};


struct TrimpotSmall : app::SvgKnob {
	widget::SvgWidget* bg;

	TrimpotSmall() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/Trimpot.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/Trimpot_bg.svg")));
	}
};


struct IMSmallKnob : TrimpotSmall {
	IMSmallKnob() {}
};









// Svg Widgets
// ----------

struct DisplayBackground : TransparentWidget {
	DisplayBackground(Vec(_pos), Vec(_size), int* /*_mode*/) {
		box.size = _size;
		box.pos = _pos;
	}
	void draw(const DrawArgs& args) override;
};


static const float smaKeyBlkdy = 1.35f;
static const float smaKeyWhtdy = 8.61f;
static const Vec smaKeysPos[12] = {	Vec(1.30f,  smaKeyWhtdy), Vec(6.08f, smaKeyBlkdy),
									Vec(10.85f, smaKeyWhtdy), Vec(15.58f, smaKeyBlkdy),
									Vec(20.34f, smaKeyWhtdy), 
									Vec(30.01f, smaKeyWhtdy), Vec(34.78f, smaKeyBlkdy),
									Vec(39.49f, smaKeyWhtdy), Vec(44.25f, smaKeyBlkdy),
									Vec(48.97f, smaKeyWhtdy), Vec(53.78f, smaKeyBlkdy),
									Vec(58.50f, smaKeyWhtdy)};

struct KeyboardSmall : SvgWidget {
	KeyboardSmall(Vec(_pos), int* /*_mode*/) {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/KeyboardSmall.svg")));
		box.pos = _pos;
	}
	void draw(const DrawArgs& args) override;
};


struct AqLedBg : SvgWidget {
	AqLedBg(Vec(_pos), int* /*_mode*/) {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/AqLedBg.svg")));
		box.pos = _pos;
	}
};


// C1 Knobs (from C1-ChannelStrip)
struct C1Knob : RoundKnob {
	C1Knob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/C1Knob_bg.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/C1Knob_bg.svg")));
	}
};

struct C1Knob280 : RoundKnob {
	C1Knob280() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/C1Knob_bg.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/comp/C1Knob_bg.svg")));
		minAngle = -140.0f * (M_PI / 180.0f);
		maxAngle = +140.0f * (M_PI / 180.0f);
	}
};


// LED Ring Overlay Widget (amber LED dots around C1Knob)
struct LedRingOverlay : widget::TransparentWidget {
	Module* module;
	int paramId;

	LedRingOverlay(Module* m, int pid) : module(m), paramId(pid) {
		box.size = Vec(50, 50);
	}

	void draw(const DrawArgs& args) override {
		if (!module) return;
		ParamQuantity* pq = module->paramQuantities[paramId];
		if (!pq) return;

		float paramValue = pq->getScaledValue();

		// LED ring parameters
		const int dotCount = 15;
		const float gapDeg = 80.0f;
		const float gap = gapDeg * (M_PI / 180.0f);
		const float start = -M_PI * 1.5f + gap * 0.5f;
		const float end   =  M_PI * 0.5f  - gap * 0.5f;
		const float totalSpan = end - start;

		const float knobRadius = 24.095f / 2.0f;
		const float ringOffset = 1.5f;
		const float radius = knobRadius + ringOffset;
		const float dotRadius = 0.9f;
		const Vec center = Vec(box.size.x / 2.0f, box.size.y / 2.0f);

		float exactPos = paramValue * (dotCount - 1);
		int led1 = (int)exactPos;
		int led2 = led1 + 1;
		float frac = exactPos - led1;

		led1 = clamp(led1, 0, dotCount - 1);
		led2 = clamp(led2, 0, dotCount - 1);

		for (int i = 0; i < dotCount; ++i) {
			float t = (float)i / (dotCount - 1);
			float angle = start + t * totalSpan;
			float x = center.x + radius * std::cos(angle);
			float y = center.y + radius * std::sin(angle);

			int alpha = 71;
			if (i == led1 && led1 != led2) {
				alpha = 71 + (int)((230 - 71) * (1.0f - frac));
			} else if (i == led2 && led1 != led2) {
				alpha = 71 + (int)((230 - 71) * frac);
			} else if (i == led1 && led1 == led2) {
				alpha = 230;
			}

			nvgBeginPath(args.vg);
			nvgCircle(args.vg, x, y, dotRadius);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, alpha));
			nvgFill(args.vg);
		}
	}
};





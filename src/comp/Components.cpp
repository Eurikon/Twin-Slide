//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************


#include "Components.hpp"


// LED color definitions
NVGcolor SCHEME_RED_TS = SCHEME_RED;
NVGcolor SCHEME_GREEN_TS = SCHEME_GREEN;


// Helpers
// ----------------------------------------

// Variations on existing knobs, lights, etc
// ----------------------------------------

void DynamicSVGPort::addFrame(std::shared_ptr<Svg> svg) {
	frames.push_back(svg);
	if (frames.size() == 1) {
		setSvg(svg);
	}
}

void DynamicSVGPort::step() {
	SvgPort::step();
}


void SwitchOutlineWidget::draw(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	NVGpaint grad = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, colTop, colBot);
	nvgRoundedRect(args.vg, 0 - 1, 0 - 1, box.size.x + 2, box.size.y + 2, 1.5f);
	nvgFillPaint(args.vg, grad);
	nvgFill(args.vg);
	Widget::draw(args);
}


TS2switchV::TS2switchV() {
	shadow->opacity = 0.0;
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV_0.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV_1.svg")));
}

TS2switchVInv::TS2switchVInv() {
	shadow->opacity = 0.0;
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV_0.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV_1.svg")));

	TransformWidget *tw = new TransformWidget();
	tw->box.size = sw->box.size;
	fb->removeChild(sw);
	tw->addChild(sw);
	fb->addChild(tw);

	tw->rotate(float(M_PI));
	tw->translate(Vec(-sw->box.size.x, -sw->box.size.y));

	tw->box.size = sw->box.size;
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
}

TS2switchVn::TS2switchVn() {
	shadow->opacity = 0.0;
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV-n_0.svg")));
	addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/2waySwitchV-n_1.svg")));
}


IMSwitch2H::IMSwitch2H() {
	shadow->hide();

	TransformWidget *tw = new TransformWidget();
	tw->box.size = sw->box.size;
	fb->removeChild(sw);
	tw->addChild(sw);
	fb->addChild(tw);

	tw->rotate(float(M_PI_2));
	tw->translate(Vec(0, -sw->box.size.y));

	sw->box.size = sw->box.size.flip();
	tw->box.size = sw->box.size;
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
}

IMSwitch3H::IMSwitch3H() {
	addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
	addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
	addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));

	shadow->opacity = 0.0;

	TransformWidget *tw = new TransformWidget();
	tw->box.size = sw->box.size;
	fb->removeChild(sw);
	tw->addChild(sw);
	fb->addChild(tw);

	tw->rotate(float(-M_PI_2));
	tw->translate(Vec(-sw->box.size.x, 0));

	sw->box.size = sw->box.size.flip();
	tw->box.size = sw->box.size;
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
}


// Svg Widgets
// ----------------------------------------

void DisplayBackground::draw(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	NVGpaint grad = nvgLinearGradient(args.vg, 0, 0, 0, box.size.y, colTop, colBot);
	nvgRoundedRect(args.vg, -1.0f, 1.5f, box.size.x + 2.0f, box.size.y - 3.0f, 1.0f);
	nvgFillPaint(args.vg, grad);
	nvgFill(args.vg);

	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0.5f, 2.0f, box.size.x - 1.0f, box.size.y - 4.0f, 2.0f);
	nvgFillColor(args.vg, nvgRGB(0x10, 0x10, 0x10));
	nvgFill(args.vg);
}


void KeyboardSmall::draw(const DrawArgs& args) {
	SvgWidget::draw(args);

	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	NVGcolor innerOutlineColor = nvgRGBA(0x88, 0x88, 0x88, 102);

	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, -1.0f, -1.0f, box.size.x + 2.0f, box.size.y + 2.0f, 2.3f);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, borderColor);
	nvgStroke(args.vg);

	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0.0f, 0.0f, box.size.x, box.size.y, 1.8f);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, innerOutlineColor);
	nvgStroke(args.vg);
}
//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************


#include "SeqUtil.hpp"
#include <ctime>
#include "comp/PianoKey.hpp"
#include "comp/MicroKeyboard.hpp"
#include "../shared/include/TCLogo.hpp"
#include "SynthDSP.hpp"


struct TwinSlide : Module {
	enum ParamIds {
		SEQSONG_PARAM,
		ENCODER_PARAM,
		RUN_PARAM,
		COPY_PARAM,
		PASTE_PARAM,
		RESET_PARAM,
		ENUMS(OCTAVE_PARAM, 5),
		GATE_PARAM,
		SLIDE_BTN_PARAM,
		SLIDE_KNOB_PARAM,
		ATTACH_PARAM,
		RUNMODE_PARAM,
		TRAN_ROT_PARAM,
		PROB_KNOB_PARAM,
		PROB_PARAM,
		TIE_PARAM,// Legato
		CPMODE_PARAM,
		ENUMS(STEP_PHRASE_PARAMS, 32),
		TRACKSELECT_PARAM,
		KEYNOTE_PARAM,
		KEYGATE_PARAM,
		ACCENT_PARAM,
		COND_PARAM,
		// Synth A parameters
		CUTOFFA_PARAM,
		RESA_PARAM,
		ENVMODA_PARAM,
		DECAYA_PARAM,
		ACCENTA_KNOB_PARAM,
		DRIVEA_PARAM,
		FINEA_PARAM,
		WAVEA_PARAM,
		LEVELA_PARAM,
		// Synth B parameters
		CUTOFFB_PARAM,
		RESB_PARAM,
		ENVMODB_PARAM,
		DECAYB_PARAM,
		ACCENTB_KNOB_PARAM,
		DRIVEB_PARAM,
		FINEB_PARAM,
		WAVEB_PARAM,
		LEVELB_PARAM,
		// Microtuning parameters
		MICRO_CENTS_PARAM,
		MICRO_OCTAVE_PARAM,
		MICRO_MASTER_PARAM,
		// Per-step release parameters (added at end for backwards compatibility)
		REL_PARAM,
		REL_KNOB_PARAM,
		CLR_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RESET_INPUT = 0,
		CLOCK_INPUT,
		RUNCV_INPUT,
		CVA_INPUT,
		CVB_INPUT,
		GATEA_INPUT,
		GATEB_INPUT,
		CUTOFFA_INPUT,
		CUTOFFB_INPUT,
		ENVMODA_INPUT,
		ENVMODB_INPUT,
		DECAYA_INPUT,
		DECAYB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CVA_OUTPUT,
		GATEA_OUTPUT,
		CVB_OUTPUT,
		GATEB_OUTPUT,
		AUDIOA_OUTPUT,
		AUDIOB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_PHRASE_LIGHTS, 32 * 3),// room for GreenRedWhite
		ENUMS(OCTAVE_LIGHTS, 5 * 2),// octaves -2 to +2, room for GreenRed
		ENUMS(KEY_LIGHTS, 12 * 3),// room for GreenRedWhite
		ENUMS(RUN_LIGHT, 2),// room for GreenRed (amber)
		RESET_LIGHT,
		GATE_LIGHT,
		SLIDE_LIGHT,
		ENUMS(ATTACH_LIGHT, 2),// room for GreenRed (TC amber)
		PROB_LIGHT,
		TIE_LIGHT,
		KEYNOTE_LIGHT,
		KEYGATE_LIGHT,
		ACCENT_LIGHT,
		COND_LIGHT,
		REL_LIGHT,
		NUM_LIGHTS
	};
	
	
	
	// Constants
	enum DisplayStateIds {DISP_NORMAL, DISP_MODE, DISP_CLOCKDIV, DISP_LENGTH, DISP_TRANSPOSE, DISP_ROTATE};


	// Expander messages
	// To expander: [0]=CVA, [1]=GateA, [2]=SlideA, [3]=CVB, [4]=GateB, [5]=SlideB
	// From expander: [0-3]=TrackA (CV,Gate,Slide,Fine), [4-7]=TrackB, [8]=presence, [9-16]=connection flags
	float rightMessages[2][18] = {};

	// Output ID mappings for track indexing
	static inline constexpr int CV_OUTPUTS[2] = {CVA_OUTPUT, CVB_OUTPUT};
	static inline constexpr int GATE_OUTPUTS[2] = {GATEA_OUTPUT, GATEB_OUTPUT};
	static inline constexpr int AUDIO_OUTPUTS[2] = {AUDIOA_OUTPUT, AUDIOB_OUTPUT};

	// Synth param/input ID mappings for track indexing
	static inline constexpr int CUTOFF_PARAMS[2] = {CUTOFFA_PARAM, CUTOFFB_PARAM};
	static inline constexpr int RES_PARAMS[2] = {RESA_PARAM, RESB_PARAM};
	static inline constexpr int ENVMOD_PARAMS[2] = {ENVMODA_PARAM, ENVMODB_PARAM};
	static inline constexpr int DECAY_PARAMS[2] = {DECAYA_PARAM, DECAYB_PARAM};
	static inline constexpr int ACCENT_KNOB_PARAMS[2] = {ACCENTA_KNOB_PARAM, ACCENTB_KNOB_PARAM};
	static inline constexpr int DRIVE_PARAMS[2] = {DRIVEA_PARAM, DRIVEB_PARAM};
	static inline constexpr int FINE_PARAMS[2] = {FINEA_PARAM, FINEB_PARAM};
	static inline constexpr int WAVE_PARAMS[2] = {WAVEA_PARAM, WAVEB_PARAM};
	static inline constexpr int LEVEL_PARAMS[2] = {LEVELA_PARAM, LEVELB_PARAM};
	static inline constexpr int CUTOFF_INPUTS[2] = {CUTOFFA_INPUT, CUTOFFB_INPUT};
	static inline constexpr int ENVMOD_INPUTS[2] = {ENVMODA_INPUT, ENVMODB_INPUT};
	static inline constexpr int DECAY_INPUTS[2] = {DECAYA_INPUT, DECAYB_INPUT};

	// Need to save, no reset
	int defaultPulsesPerStep;// user-configurable default PPS, survives Initialize

	// Need to save, with reset
	bool holdTiedNotes;
	int pulsesPerStep;// 1 means normal gate mode, alt choices are 4, 6, 12, 24 PPS (Pulses per step)
	bool running;
	int runModeSong;
	int stepIndexEdit;
	int seqIndexEdit;
	int phraseIndexEdit;
	int phrases;//1 to 32
	int phrase[32];// This is the song (series of phases; a phrase is a patten number)
	float cv[32][2][16];// [-3.0 : 3.917]. [sequence][track][step] - Track 0=A (top row), Track 1=B (bottom row)
	StepAttributes attributes[32][2][16];// [sequence][track][step]
	float releaseTime[32][2][16];// [sequence][track][step] - per-step release time in ms (21-500ms)
	// Track settings [sequence][track]
	int trackLength[32][2];     // independent lengths per track (1-16)
	int trackRunMode[32][2];    // independent run mode per track
	int trackTranspose[32][2];  // independent transpose per track
	int trackRotate[32][2];     // independent rotate per track
	int trackClockDiv[32][2];   // clock division 1-5
	// Playback state
	bool resetOnRun;
	int retrigGatesOnReset;
	bool attached;
	bool stopAtEndOfSong;
	// Microtuning state
	float microtuningCents[12];  // Cent offset per note (-100 to +100)
	int microtuningOctave;       // Octave shift (-1, 0, +1)
	float masterTuneHz;          // Master tuning (420-460Hz, default 440Hz)
	int selectedMicroKey;        // Currently selected key for editing (0-11)
	int temperamentPreset;       // Current temperament preset index (0-4)
	// Expert mode settings (per synth)
	struct ExpertSettings {
		float vcfRange = 0.0f;    // -1 to 0 (range shift), default 0
		float decayRange = 0.0f;  // -1 to +1 (±20% shift), default 0
		float sustain = 0.0f;     // 0-1, default 0 (no sustain)
		float release = 0.0f;     // 0-1, default 0 (instant)
		float blend = 0.0f;       // 0-1, saw to square blend
		float bassComp = 1.0f;    // 0-1, bass compensation (1 = 303 default, 0 = full bass preserved)
	};
	ExpertSettings expertA, expertB;
	// RNDP (Acid) pattern generation settings
	int rndp2Density;            // 0-100%, default 75
	int rndp2Spread;             // 0-100%, default 50
	int rndp2AccentDensity;      // 0-100%, default 25
	int rndp2SlideDensity;       // 0-100%, default 25
	int rndp2Seed;               // 0-999, default 303
	bool rndp2PinSeed;           // default false - when true, seed doesn't auto-increment
	int rndp2Transpose;          // -12 to +12, default 0
	bool rndp2StartWithNote;     // default true
	bool rndp2StartWithAccent;   // default false
	int rndpScaleIndex;          // -1 = random, 0-4 = specific scale
	uint32_t instanceSalt;       // Random per module instance, makes seeds unique

	// Randomizer exclusion flags (UI-only, not saved)
	enum RndExcl { RND_PITCH, RND_GATESHAPE, RND_GATE, RND_TIE, RND_REL, RND_ACC, RND_COND, RND_PROB, RND_SLIDE, NUM_RND_EXCL };
	bool rndExclude[NUM_RND_EXCL] = {};
	void clearRndExclusions() { for (int i = 0; i < NUM_RND_EXCL; i++) rndExclude[i] = false; }

	// No need to save, with reset
	int displayState;
	int lastTrackSwitch;// track previous switch state to detect manual changes
	int lastSelectedMicroKey;// track previous selected micro key to sync knob
	int lastTemperamentPreset;// track previous temperament preset to detect changes
	float cvCPbuffer[16];// copy paste buffer for CVs (max 16 steps per track)
	StepAttributes attribCPbuffer[16];
	int phraseCPbuffer[32];
	bool seqCopied;
	int countCP;// number of steps to paste (in case CPMODE_PARAM changes between copy and paste)
	int startCP;
	unsigned long editingGate;// 0 when no edit gate, downward step counter timer when edit gate
	unsigned long editingType;
	long infoCopyPaste;// 0 when no info, positive downward step counter timer when copy, negative upward when paste
	unsigned long clockPeriod;// counts number of step() calls upward from last clock (reset after clock processed)
	long tiedWarning;// 0 when no warning, positive downward step counter timer when warning
	long attachedWarning;// 0 when no warning, positive downward step counter timer when warning
	long attachFlashCounter;// 0 when no flash, positive downward counter for 3-pulse flash in song mode
	long revertDisplay;
	long editingGateLength;// 0 when no info, positive when gate
	long editingPpqn;// 0 when no info, positive downward step counter timer when editing ppqn
	long clockIgnoreOnReset;

	// Precalculated tick values (updated in onSampleRateChange)
	long gateTimeTicks;
	long revertDisplayTimeTicks;
	long lengthDisplayTimeTicks;
	long warningTimeTicks;
	long holdDetectTimeTicks;
	long editGateLengthTimeTicks;
	long editCursorTimeTicks;
	long attachFlashTimeTicks;

	int phraseIndexRun;	
	unsigned long phraseIndexRunHistory;
	int stepIndexRun[2];
	unsigned long stepIndexRunHistory[2];// per-track step history for run modes
	int clockDivCounter[2];// per-track counter for clock division
	bool stepActive[2];// per-track flag: true during active steps, false during skipped (division)
	int ppqnCount;
	int gateCode[2];
	bool lastProbGateEnable[2];
	int loopCount[2];// per-track loop counter for condition feature
	unsigned long slideStepsRemain[2];// 0 when no slide under way, downward step counter when sliding

	// No need to save, no reset
	RefreshCounter refresh;
	float slideCVdelta[2];// no need to initialize, this is a companion to slideStepsRemain
	float latchedReleaseAmt[2] = {0.0f, 0.0f};// latched at gate-on for correct per-step release
	bool prevGate[2] = {false, false};// track gate transitions for latching
	int prevWave[2] = {0, 0};// track wave switch for blend sync
	float editingGateCV;// no need to initialize, this is a companion to editingGate (output this only when editingGate > 0)
	int editingGateKeyLight;// no need to initialize, this is a companion to editingGate (use this only when editingGate > 0)
	int editingChannel;// 0 means channel A, 1 means channel B. no need to initialize, this is a companion to editingGate
	float resetLight = 0.0f;
	float keyLightPulsePhase = 0.0f;// for slow pulsing key light
	float lastRelKnobValue = 100.0f;// tracks REL_KNOB for per-step updates
	long editCursorTimeout = 0;// countdown for edit cursor visibility (5 seconds)
	int flashCounter = 0;// for flashing LEDs in DISP_MODE
	int encoderState = 0;
	Trigger resetTrigger;
	Trigger runningTrigger;
	Trigger clockTrigger;
	Trigger octTriggers[5];
	Trigger gateTrigger;
	Trigger probTrigger;
	Trigger slideTrigger;
	Trigger accentTrigger;
	dsp::BooleanTrigger keyTrigger;
	Trigger attachedTrigger;
	Trigger copyTrigger;
	Trigger pasteTrigger;
	Trigger clrTrigger;
	Trigger modeTrigger;
	Trigger transposeTrigger;
	Trigger tiedTrigger;
	Trigger relTrigger;
	Trigger condTrigger;
	Trigger stepTriggers[32];
	Trigger keyNoteTrigger;
	Trigger keyGateTrigger;
	HoldDetect modeHoldDetect;
	PianoKeyInfo pkInfo;

	SynthVoice voice[2];


	bool isSeqMode(void) {return params[SEQSONG_PARAM].getValue() > 0.5f;}

	// Get track from TRACKSELECT switch (0 = Track A, 1 = Track B)
	int getEditTrack() { return (int)(params[TRACKSELECT_PARAM].getValue() + 0.5f); }
	int getEditStep() { return stepFromIndex(stepIndexEdit); }

	// Generic track/step extraction from 0-31 index
	int trackFromIndex(int idx) { return (idx < 16) ? 0 : 1; }
	int stepFromIndex(int idx) { return (idx < 16) ? idx : (idx - 16); }
	int trackStartIndex(int track) { return track * 16; }
	int getEditStepCondition() {
		int track = getEditTrack();
		int step = getEditStep();
		return attributes[seqIndexEdit][track][step].getCond();
	}


	void fillStepIndexRunVector() {
		// TwinSlide: Each track maintains independent position and run mode
		// RN2 mode: non-repeating random per track (never same step twice in a row)
		int seq = (isSeqMode() ? seqIndexEdit : phrase[phraseIndexRun]);
		for (int t = 0; t < 2; t++) {
			if (trackRunMode[seq][t] == MODE_RN2) {
				int len = trackLength[seq][t];
				if (len > 1) {
					int oldIndex = stepIndexRun[t];
					stepIndexRun[t] = random::u32() % (len - 1);
					if (stepIndexRun[t] >= oldIndex)
						stepIndexRun[t]++;
				}
			}
		}
	}

	TwinSlide() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		// Expander setup
		rightExpander.producerMessage = rightMessages[0];
		rightExpander.consumerMessage = rightMessages[1];

		configSwitch(TRACKSELECT_PARAM, 0.0f, 1.0f, 0.0f, "Track Select", {"Track A", "Track B"});
		for (int x = 0; x < 16; x++) {
			configButton(STEP_PHRASE_PARAMS + x, string::f("Track A - Step %i", x + 1));
			configButton(STEP_PHRASE_PARAMS + x + 16, string::f("Track B - Step %i", x + 1));
		}
		configParam(ATTACH_PARAM, 0.0f, 1.0f, 0.0f, "Attach");
		configParam(KEYNOTE_PARAM, 0.0f, 1.0f, 0.0f, "Step Note");
		configParam(KEYGATE_PARAM, 0.0f, 1.0f, 0.0f, "Step Gate");
		for (int i = 0; i < 5; i++) {
			configButton(OCTAVE_PARAM + i, string::f("Octave %i", 2 - i));
		}
		
		configSwitch(SEQSONG_PARAM, 0.0f, 1.0f, 1.0f, "Seq/song mode", {"Song", "Sequence"});// 1.0f is top position
		configParam(RUNMODE_PARAM, 0.0f, 1.0f, 0.0f, "Length / run mode");
		configParam(RUN_PARAM, 0.0f, 1.0f, 0.0f, "Run");
		configParam(ENCODER_PARAM, -INFINITY, INFINITY, 0.0f, "Data Encoder");		
		configParam(TRAN_ROT_PARAM, 0.0f, 1.0f, 0.0f, "Transpose / rotate");
		
		configParam(RESET_PARAM, 0.0f, 1.0f, 0.0f, "Reset");
		configParam(COPY_PARAM, 0.0f, 1.0f, 0.0f, "Copy");
		configParam(PASTE_PARAM, 0.0f, 1.0f, 0.0f, "Paste");
		configParam(CLR_PARAM, 0.0f, 1.0f, 0.0f, "Clear track");
		configSwitch(CPMODE_PARAM, 0.0f, 2.0f, 0.0f, "Copy", {"4 steps", "8 steps", "Track"});// 0.0f is top position

		configParam(GATE_PARAM, 0.0f, 1.0f, 0.0f, "Gate");
		configParam(TIE_PARAM, 0.0f, 1.0f, 0.0f, "Tied");
		configParam(REL_PARAM, 0.0f, 1.0f, 0.0f, "Release");
		configParam(REL_KNOB_PARAM, 21.0f, 250.0f, 100.0f, "Release Time", " ms");
		configParam(ACCENT_PARAM, 0.0f, 1.0f, 0.0f, "Accent");
		configParam(COND_PARAM, 0.0f, 1.0f, 0.0f, "Condition");// ConditionParamQuantity set in TwinSlideWidget constructor

		configParam(PROB_PARAM, 0.0f, 1.0f, 0.0f, "Prob");
		configParam(PROB_KNOB_PARAM, 0.0f, 1.0f, 0.85f, "Probability");
		configParam(SLIDE_BTN_PARAM, 0.0f, 1.0f, 0.0f, "CV slide");
		configParam(SLIDE_KNOB_PARAM, 0.0f, 2.0f, 0.75f, "Slide rate");

		configInput(RESET_INPUT, "Reset");
		configInput(CLOCK_INPUT, "Clock");
		configInput(RUNCV_INPUT, "Run");
		configInput(CVA_INPUT, "Track A CV");
		configInput(CVB_INPUT, "Track B CV");
		configInput(GATEA_INPUT, "Track A Gate");
		configInput(GATEB_INPUT, "Track B Gate");

		configOutput(CVA_OUTPUT, "Track A CV");
		configOutput(GATEA_OUTPUT, "Track A Gate");
		configOutput(CVB_OUTPUT, "Track B CV");
		configOutput(GATEB_OUTPUT, "Track B Gate");
		configOutput(AUDIOA_OUTPUT, "Synth A Audio");
		configOutput(AUDIOB_OUTPUT, "Synth B Audio");

		// Synth parameters (A and B)
		static const char* trackNames[2] = {"A", "B"};
		for (int t = 0; t < 2; t++) {
			std::string p = std::string("Synth ") + trackNames[t] + " ";
			configParam(CUTOFF_PARAMS[t], 0.0f, 1.0f, 0.5f, (p + "Cutoff").c_str(), "%", 0.f, 100.f);
			configParam(RES_PARAMS[t], 0.0f, 1.0f, 0.0f, (p + "Resonance").c_str(), "%", 0.f, 100.f);
			configParam(ENVMOD_PARAMS[t], 0.0f, 1.0f, 0.5f, (p + "Env Mod").c_str(), "%", 0.f, 100.f);
			configParam(DECAY_PARAMS[t], 0.0f, 1.0f, 0.5f, (p + "Decay").c_str(), "%", 0.f, 100.f);
			configParam(ACCENT_KNOB_PARAMS[t], 0.0f, 1.0f, 0.5f, (p + "Accent").c_str(), "%", 0.f, 100.f);
			configParam(DRIVE_PARAMS[t], 0.0f, 1.0f, 0.75f, (p + "Drive").c_str(), "%", 0.f, 100.f);
			configParam(FINE_PARAMS[t], -1.0f, 1.0f, 0.0f, (p + "Fine Tune").c_str(), " cents", 0.f, 700.f);
			configSwitch(WAVE_PARAMS[t], 0.0f, 1.0f, 0.0f, (p + "Waveform").c_str(), {"Saw", "Square"});
			configParam(LEVEL_PARAMS[t], 0.0f, 1.0f, 0.75f, (p + "Level").c_str(), "%", 0.f, 100.f);
			configInput(CUTOFF_INPUTS[t], (p + "Cutoff CV").c_str());
			configInput(ENVMOD_INPUTS[t], (p + "Env Mod CV").c_str());
			configInput(DECAY_INPUTS[t], (p + "Decay CV").c_str());
		}

		// Microtuning parameters
		configParam(MICRO_CENTS_PARAM, -100.0f, 100.0f, 0.0f, "Cents", " cents");
		configSwitch(MICRO_OCTAVE_PARAM, -1.0f, 1.0f, 0.0f, "Octave Shift", {"-1", "0", "+1"});
		configParam(MICRO_MASTER_PARAM, 420.0f, 460.0f, 440.0f, "Master Tune", " Hz");

		// Disable randomization for all parameters - only sequence data is randomized
		for (int i = 0; i < NUM_PARAMS; i++) {
			getParamQuantity(i)->randomizeEnabled = false;
		}

		// Initialize synth voices with current sample rate
		for (int t = 0; t < 2; t++)
			voice[t].setSampleRate(APP->engine->getSampleRate());

		defaultPulsesPerStep = 24;
		onReset();
	}

	void onSampleRateChange() override {
		double sr = (double)APP->engine->getSampleRate();
		for (int t = 0; t < 2; t++)
			voice[t].setSampleRate(sr);

		// Precalculate tick values for timers
		// Formula: ticks = seconds * sampleRate / displayRefreshStepSkips
		double tickFactor = sr / RefreshCounter::displayRefreshStepSkips;
		gateTimeTicks = (long)(0.4 * tickFactor);
		revertDisplayTimeTicks = (long)(0.7 * tickFactor);
		lengthDisplayTimeTicks = (long)(5.0 * tickFactor);
		warningTimeTicks = (long)(0.7 * tickFactor);
		holdDetectTimeTicks = (long)(2.0 * tickFactor);
		editGateLengthTimeTicks = (long)(3.5 * tickFactor);
		editCursorTimeTicks = (long)(10.0 * tickFactor);
		attachFlashTimeTicks = (long)(0.8 * tickFactor);// 0.8 seconds for 2 pulses
	}

	
	void onReset() override final {
		clearRndExclusions();
		holdTiedNotes = true;
		pulsesPerStep = defaultPulsesPerStep;
		running = false;
		runModeSong = MODE_FWD;
		stepIndexEdit = 0;
		seqIndexEdit = 0;
		phraseIndexEdit = 0;
		phrases = 4;
		for (int i = 0; i < 32; i++) {
			phrase[i] = 0;
			for (int t = 0; t < 2; t++) {
				trackLength[i][t] = 16;// default 16 steps per track
				trackRunMode[i][t] = MODE_FWD;// default run mode per track
				trackTranspose[i][t] = 0;// default transpose per track
				trackRotate[i][t] = 0;// default rotate per track
				trackClockDiv[i][t] = 1;// default clock division (no division)
				for (int s = 0; s < 16; s++) {
					cv[i][t][s] = 0.0f;
					attributes[i][t][s].init();
					releaseTime[i][t][s] = 100.0f;// default 100ms release
				}
			}
		}
		resetOnRun = false;
		retrigGatesOnReset = RGOR_NRUN;
		attached = false;
		stopAtEndOfSong = false;
		// Microtuning reset
		for (int i = 0; i < 12; i++) {
			microtuningCents[i] = 0.0f;
		}
		microtuningOctave = 0;
		masterTuneHz = 440.0f;
		selectedMicroKey = 0;
		temperamentPreset = 0;
		// Expert mode defaults
		expertA = {};
		expertB = {};
		// RNDP defaults
		rndp2Density = 75;
		rndp2Spread = 50;
		rndp2AccentDensity = 25;
		rndp2SlideDensity = 25;
		rndp2Seed = 303;
		rndp2PinSeed = false;
		rndp2Transpose = 0;
		rndp2StartWithNote = true;
		rndp2StartWithAccent = false;
		rndpScaleIndex = -1;  // Random scale selection
		instanceSalt = random::u32();  // Unique per module instance
		for (int t = 0; t < 2; t++)
			voice[t].reset();
		resetNonJson(false);
	}
	void resetNonJson(bool delayed) {// delay thread sensitive parts (i.e. schedule them so that process() will do them)
		displayState = DISP_NORMAL;
		lastTrackSwitch = 0;
		lastSelectedMicroKey = 0;
		lastTemperamentPreset = 0;
		for (int i = 0; i < 16; i++) {
			cvCPbuffer[i] = 0.0f;
			attribCPbuffer[i].init();
		}
		for (int i = 0; i < 32; i++) {
			phraseCPbuffer[i] = 0;
		}
		seqCopied = true;
		countCP = 16;
		startCP = 0;
		editingGate = 0ul;
		editingType = 0ul;
		infoCopyPaste = 0l;
		clockPeriod = 0ul;
		tiedWarning = 0ul;
		attachedWarning = 0l;
		attachFlashCounter = 0l;
		revertDisplay = 0l;
		editingGateLength = 0l;
		editingPpqn = 0l;
		if (delayed) {
			// Signal deferred init after JSON load
		}
		else {
			initRun();
		}
	}
	void initRun() {// run button activated, or run edge in run input jack, or fromJson()
		clockIgnoreOnReset = (long) (clockIgnoreOnResetDuration * APP->engine->getSampleRate());
		phraseIndexRun = (runModeSong == MODE_REV ? phrases - 1 : 0);
		phraseIndexRunHistory = 0;

		int seq = (isSeqMode() ? seqIndexEdit : phrase[phraseIndexRun]);
		// Initialize each track's step position based on its own length and run mode
		for (int t = 0; t < 2; t++) {
			stepIndexRun[t] = (trackRunMode[seq][t] == MODE_REV ? trackLength[seq][t] - 1 : 0);
			stepIndexRunHistory[t] = 0;
			clockDivCounter[t] = 0;
			stepActive[t] = true;// first step is always active
		}

		ppqnCount = 0;
		for (int i = 0; i < 2; i++) {
			lastProbGateEnable[i] = true;
			loopCount[i] = 0;
			calcGateCode(attributes[seq][i][stepIndexRun[i]], i);
		}
		slideStepsRemain[0] = 0ul;
		slideStepsRemain[1] = 0ul;
	}	

	
	void onRandomize() override {
		randomizeTrack();
	}

	// Randomize track content using acid generator (applies to selected track only)
	void randomizeTrack() {
		if (!isSeqMode()) return;

		int track = getEditTrack();
		AcidStep steps[16];

		// Build params
		AcidGenParams params;
		params.density = rndp2Density;
		params.spread = rndp2Spread;
		params.accentDensity = rndp2AccentDensity;
		params.slideDensity = rndp2SlideDensity;
		params.seed = (uint32_t)rndp2Seed + instanceSalt;
		params.transpose = rndp2Transpose;

		// Auto-increment seed for next generation (wraps at 999) unless pinned
		if (!rndp2PinSeed) {
			rndp2Seed = (rndp2Seed + 1) % 1000;
		}
		params.startWithNote = rndp2StartWithNote;
		params.startWithAccent = rndp2StartWithAccent;
		params.forceScaleIndex = rndpScaleIndex;  // Use configured scale (-1 = random)

		// Generate track content
		generateAcidPattern(params, steps, nullptr);

		// Apply to selected track (respecting exclusion flags)
		for (int s = 0; s < 16; s++) {
			if (!rndExclude[RND_PITCH])
				cv[seqIndexEdit][track][s] = steps[s].cv;

			StepAttributes& attr = attributes[seqIndexEdit][track][s];
			if (!rndExclude[RND_GATE]) {
				attr.setGate(steps[s].gate);
			}
			if (!rndExclude[RND_ACC])
				attr.setAccent(steps[s].accent);
			if (!rndExclude[RND_SLIDE])
				attr.setSlide(steps[s].slide);
			if (attr.getGate()) {
				if (!rndExclude[RND_PROB]) {
					if (s != 0 && random::u32() % 20 == 0) attr.setGateP(true);
				}
				if (!rndExclude[RND_GATESHAPE]) {
					static const int gateShapes[] = {0, 5, 11};
					attr.setGateMode(gateShapes[random::u32() % 3]);
				}
				if (!rndExclude[RND_REL]) {
					if (s != 0 && random::u32() % 20 == 0) {
						attr.setRelease(true);
						releaseTime[seqIndexEdit][track][s] = 21.0f + random::uniform() * (250.0f - 21.0f);
					} else {
						attr.setRelease(false);
						releaseTime[seqIndexEdit][track][s] = 21.0f;
					}
				}
				if (!rndExclude[RND_COND]) {
					if (s != 0 && random::u32() % 20 == 0) attr.setCond(1 + (random::u32() % 4));
				}
				if (!rndExclude[RND_TIE] && !rndExclude[RND_GATE]) {
					if (random::u32() % 20 == 0) attr.setTied(true);
				}
			}
		}
		// clearRndExclusions(); // TESTING: disabled for persistent exclusion testing
	}


	// JSON array helpers
	template<typename T>
	void saveArray3D(json_t* rootJ, const char* key, T arr[32][2][16], bool isInteger) {
		json_t* arrJ = json_array();
		for (int i = 0; i < 32; i++)
			for (int t = 0; t < 2; t++)
				for (int s = 0; s < 16; s++)
					json_array_insert_new(arrJ, s + (t * 16) + (i * 32),
						isInteger ? json_integer((int)arr[i][t][s]) : json_real(arr[i][t][s]));
		json_object_set_new(rootJ, key, arrJ);
	}

	template<typename T>
	void saveArray2D(json_t* rootJ, const char* key, T arr[32][2]) {
		json_t* arrJ = json_array();
		for (int i = 0; i < 32; i++)
			for (int t = 0; t < 2; t++)
				json_array_insert_new(arrJ, t + (i * 2), json_integer(arr[i][t]));
		json_object_set_new(rootJ, key, arrJ);
	}

	template<typename T>
	bool loadArray3D(json_t* rootJ, const char* key, T arr[32][2][16], bool isInteger) {
		json_t* arrJ = json_object_get(rootJ, key);
		if (arrJ) {
			for (int i = 0; i < 32; i++)
				for (int t = 0; t < 2; t++)
					for (int s = 0; s < 16; s++) {
						json_t* elemJ = json_array_get(arrJ, s + (t * 16) + (i * 32));
						if (elemJ)
							arr[i][t][s] = isInteger ? (T)json_integer_value(elemJ) : (T)json_number_value(elemJ);
					}
			return true;
		}
		return false;
	}

	template<typename T>
	bool loadArray2D(json_t* rootJ, const char* key, T arr[32][2]) {
		json_t* arrJ = json_object_get(rootJ, key);
		if (arrJ) {
			for (int i = 0; i < 32; i++)
				for (int t = 0; t < 2; t++) {
					json_t* elemJ = json_array_get(arrJ, t + (i * 2));
					if (elemJ)
						arr[i][t] = (T)json_integer_value(elemJ);
				}
			return true;
		}
		return false;
	}


	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "holdTiedNotes", json_boolean(holdTiedNotes));
		json_object_set_new(rootJ, "pulsesPerStep", json_integer(pulsesPerStep));
		json_object_set_new(rootJ, "defaultPulsesPerStep", json_integer(defaultPulsesPerStep));
		json_object_set_new(rootJ, "running", json_boolean(running));
		json_object_set_new(rootJ, "runModeSong3", json_integer(runModeSong));
		json_object_set_new(rootJ, "sequence", json_integer(seqIndexEdit));

		json_t *phraseJ = json_array();
		for (int i = 0; i < 32; i++)
			json_array_insert_new(phraseJ, i, json_integer(phrase[i]));
		json_object_set_new(rootJ, "phrase", phraseJ);

		json_object_set_new(rootJ, "phrases", json_integer(phrases));

		saveArray3D(rootJ, "cv", cv, false);

		json_t *attributesJ = json_array();
		for (int i = 0; i < 32; i++)
			for (int t = 0; t < 2; t++)
				for (int s = 0; s < 16; s++)
					json_array_insert_new(attributesJ, s + (t * 16) + (i * 32), json_integer(attributes[i][t][s].getAttribute()));
		json_object_set_new(rootJ, "attributes", attributesJ);

		saveArray3D(rootJ, "releaseTime", releaseTime, false);

		saveArray2D(rootJ, "trackLength", trackLength);
		saveArray2D(rootJ, "trackRunMode", trackRunMode);
		saveArray2D(rootJ, "trackTranspose", trackTranspose);
		saveArray2D(rootJ, "trackRotate", trackRotate);
		saveArray2D(rootJ, "trackClockDiv", trackClockDiv);

		json_object_set_new(rootJ, "attached", json_boolean(attached));
		json_object_set_new(rootJ, "stopAtEndOfSong", json_boolean(stopAtEndOfSong));
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		json_object_set_new(rootJ, "retrigGatesOnReset2", json_integer(retrigGatesOnReset));
		json_object_set_new(rootJ, "stepIndexEdit", json_integer(stepIndexEdit));
		json_object_set_new(rootJ, "phraseIndexEdit", json_integer(phraseIndexEdit));

		json_t *microtuningCentsJ = json_array();
		for (int i = 0; i < 12; i++)
			json_array_insert_new(microtuningCentsJ, i, json_real(microtuningCents[i]));
		json_object_set_new(rootJ, "microtuningCents", microtuningCentsJ);
		json_object_set_new(rootJ, "microtuningOctave", json_integer(microtuningOctave));
		json_object_set_new(rootJ, "masterTuneHz", json_real(masterTuneHz));
		json_object_set_new(rootJ, "temperamentPreset", json_integer(temperamentPreset));

		json_t *expertAJ = json_object();
		json_object_set_new(expertAJ, "vcfRange", json_real(expertA.vcfRange));
		json_object_set_new(expertAJ, "decayRange", json_real(expertA.decayRange));
		json_object_set_new(expertAJ, "sustain", json_real(expertA.sustain));
		json_object_set_new(expertAJ, "release", json_real(expertA.release));
		json_object_set_new(expertAJ, "blend", json_real(expertA.blend));
		json_object_set_new(expertAJ, "bassComp", json_real(expertA.bassComp));
		json_object_set_new(rootJ, "expertA", expertAJ);

		json_t *expertBJ = json_object();
		json_object_set_new(expertBJ, "vcfRange", json_real(expertB.vcfRange));
		json_object_set_new(expertBJ, "decayRange", json_real(expertB.decayRange));
		json_object_set_new(expertBJ, "sustain", json_real(expertB.sustain));
		json_object_set_new(expertBJ, "release", json_real(expertB.release));
		json_object_set_new(expertBJ, "blend", json_real(expertB.blend));
		json_object_set_new(expertBJ, "bassComp", json_real(expertB.bassComp));
		json_object_set_new(rootJ, "expertB", expertBJ);

		json_object_set_new(rootJ, "rndp2Density", json_integer(rndp2Density));
		json_object_set_new(rootJ, "rndp2Spread", json_integer(rndp2Spread));
		json_object_set_new(rootJ, "rndp2AccentDensity", json_integer(rndp2AccentDensity));
		json_object_set_new(rootJ, "rndp2SlideDensity", json_integer(rndp2SlideDensity));
		json_object_set_new(rootJ, "rndp2Seed", json_integer(rndp2Seed));
		json_object_set_new(rootJ, "rndp2PinSeed", json_boolean(rndp2PinSeed));
		json_object_set_new(rootJ, "rndp2Transpose", json_integer(rndp2Transpose));
		json_object_set_new(rootJ, "rndp2StartWithNote", json_boolean(rndp2StartWithNote));
		json_object_set_new(rootJ, "rndp2StartWithAccent", json_boolean(rndp2StartWithAccent));
		json_object_set_new(rootJ, "rndpScaleIndex", json_integer(rndpScaleIndex));
		json_object_set_new(rootJ, "instanceSalt", json_integer(instanceSalt));

		return rootJ;
	}

	
	void dataFromJson(json_t *rootJ) override {
		json_t *holdTiedNotesJ = json_object_get(rootJ, "holdTiedNotes");
		if (holdTiedNotesJ)
			holdTiedNotes = json_is_true(holdTiedNotesJ);
		else
			holdTiedNotes = false;

		json_t *pulsesPerStepJ = json_object_get(rootJ, "pulsesPerStep");
		if (pulsesPerStepJ)
			pulsesPerStep = json_integer_value(pulsesPerStepJ);

		json_t *defaultPulsesPerStepJ = json_object_get(rootJ, "defaultPulsesPerStep");
		if (defaultPulsesPerStepJ)
			defaultPulsesPerStep = json_integer_value(defaultPulsesPerStepJ);

		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		json_t *runModeSongJ = json_object_get(rootJ, "runModeSong3");
		if (runModeSongJ)
			runModeSong = json_integer_value(runModeSongJ);
		else {
			runModeSongJ = json_object_get(rootJ, "runModeSong");
			if (runModeSongJ) {
				runModeSong = json_integer_value(runModeSongJ);
				if (runModeSong >= MODE_PEN)
					runModeSong++;
			}
		}

		json_t *sequenceJ = json_object_get(rootJ, "sequence");
		if (sequenceJ)
			seqIndexEdit = json_integer_value(sequenceJ);

		json_t *phraseJ = json_object_get(rootJ, "phrase");
		if (phraseJ)
			for (int i = 0; i < 32; i++)
			{
				json_t *phraseArrayJ = json_array_get(phraseJ, i);
				if (phraseArrayJ)
					phrase[i] = json_integer_value(phraseArrayJ);
			}

		json_t *phrasesJ = json_object_get(rootJ, "phrases");
		if (phrasesJ)
			phrases = json_integer_value(phrasesJ);

		loadArray3D(rootJ, "cv", cv, false);

		json_t *attributesJ = json_object_get(rootJ, "attributes");
		if (attributesJ) {
			for (int i = 0; i < 32; i++)
				for (int t = 0; t < 2; t++)
					for (int s = 0; s < 16; s++) {
						json_t *attributesArrayJ = json_array_get(attributesJ, s + (t * 16) + (i * 32));
						if (attributesArrayJ)
							attributes[i][t][s].setAttribute((unsigned short)json_integer_value(attributesArrayJ));
					}
		}

		loadArray3D(rootJ, "releaseTime", releaseTime, false);

		loadArray2D(rootJ, "trackLength", trackLength);
		loadArray2D(rootJ, "trackRunMode", trackRunMode);
		loadArray2D(rootJ, "trackTranspose", trackTranspose);
		loadArray2D(rootJ, "trackRotate", trackRotate);
		loadArray2D(rootJ, "trackClockDiv", trackClockDiv);

		json_t *attachedJ = json_object_get(rootJ, "attached");
		if (attachedJ)
			attached = json_is_true(attachedJ);

		json_t *stopAtEndOfSongJ = json_object_get(rootJ, "stopAtEndOfSong");
		if (stopAtEndOfSongJ)
			stopAtEndOfSong = json_is_true(stopAtEndOfSongJ);

		json_t *resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_is_true(resetOnRunJ);

		json_t *retrigGatesOnResetJ = json_object_get(rootJ, "retrigGatesOnReset2");
		if (retrigGatesOnResetJ)
			retrigGatesOnReset = json_integer_value(retrigGatesOnResetJ);

		json_t *stepIndexEditJ = json_object_get(rootJ, "stepIndexEdit");
		if (stepIndexEditJ)
			stepIndexEdit = json_integer_value(stepIndexEditJ);

		json_t *phraseIndexEditJ = json_object_get(rootJ, "phraseIndexEdit");
		if (phraseIndexEditJ)
			phraseIndexEdit = json_integer_value(phraseIndexEditJ);

		json_t *microtuningCentsJ = json_object_get(rootJ, "microtuningCents");
		if (microtuningCentsJ) {
			for (int i = 0; i < 12; i++) {
				json_t *centsJ = json_array_get(microtuningCentsJ, i);
				if (centsJ)
					microtuningCents[i] = json_number_value(centsJ);
			}
		}
		json_t *microtuningOctaveJ = json_object_get(rootJ, "microtuningOctave");
		if (microtuningOctaveJ)
			microtuningOctave = json_integer_value(microtuningOctaveJ);
		json_t *masterTuneHzJ = json_object_get(rootJ, "masterTuneHz");
		if (masterTuneHzJ)
			masterTuneHz = json_number_value(masterTuneHzJ);
		json_t *temperamentPresetJ = json_object_get(rootJ, "temperamentPreset");
		if (temperamentPresetJ)
			temperamentPreset = json_integer_value(temperamentPresetJ);

		json_t *expertAJ = json_object_get(rootJ, "expertA");
		if (expertAJ) {
			json_t *vcfRangeJ = json_object_get(expertAJ, "vcfRange");
			if (vcfRangeJ) expertA.vcfRange = json_number_value(vcfRangeJ);
			json_t *decayRangeJ = json_object_get(expertAJ, "decayRange");
			if (decayRangeJ) expertA.decayRange = json_number_value(decayRangeJ);
			json_t *sustainJ = json_object_get(expertAJ, "sustain");
			if (sustainJ) expertA.sustain = json_number_value(sustainJ);
			json_t *releaseJ = json_object_get(expertAJ, "release");
			if (releaseJ) expertA.release = json_number_value(releaseJ);
			json_t *blendJ = json_object_get(expertAJ, "blend");
			if (blendJ) expertA.blend = json_number_value(blendJ);
			json_t *bassCompJ = json_object_get(expertAJ, "bassComp");
			if (bassCompJ) expertA.bassComp = json_number_value(bassCompJ);
		}

		json_t *expertBJ = json_object_get(rootJ, "expertB");
		if (expertBJ) {
			json_t *vcfRangeJ = json_object_get(expertBJ, "vcfRange");
			if (vcfRangeJ) expertB.vcfRange = json_number_value(vcfRangeJ);
			json_t *decayRangeJ = json_object_get(expertBJ, "decayRange");
			if (decayRangeJ) expertB.decayRange = json_number_value(decayRangeJ);
			json_t *sustainJ = json_object_get(expertBJ, "sustain");
			if (sustainJ) expertB.sustain = json_number_value(sustainJ);
			json_t *releaseJ = json_object_get(expertBJ, "release");
			if (releaseJ) expertB.release = json_number_value(releaseJ);
			json_t *blendJ = json_object_get(expertBJ, "blend");
			if (blendJ) expertB.blend = json_number_value(blendJ);
			json_t *bassCompJ = json_object_get(expertBJ, "bassComp");
			if (bassCompJ) expertB.bassComp = json_number_value(bassCompJ);
		}

		json_t *rndp2DensityJ = json_object_get(rootJ, "rndp2Density");
		if (rndp2DensityJ) rndp2Density = json_integer_value(rndp2DensityJ);
		json_t *rndp2SpreadJ = json_object_get(rootJ, "rndp2Spread");
		if (rndp2SpreadJ) rndp2Spread = json_integer_value(rndp2SpreadJ);
		json_t *rndp2AccentDensityJ = json_object_get(rootJ, "rndp2AccentDensity");
		if (rndp2AccentDensityJ) rndp2AccentDensity = json_integer_value(rndp2AccentDensityJ);
		json_t *rndp2SlideDensityJ = json_object_get(rootJ, "rndp2SlideDensity");
		if (rndp2SlideDensityJ) rndp2SlideDensity = json_integer_value(rndp2SlideDensityJ);
		json_t *rndp2SeedJ = json_object_get(rootJ, "rndp2Seed");
		if (rndp2SeedJ) rndp2Seed = json_integer_value(rndp2SeedJ);
		json_t *rndp2PinSeedJ = json_object_get(rootJ, "rndp2PinSeed");
		if (rndp2PinSeedJ) rndp2PinSeed = json_boolean_value(rndp2PinSeedJ);
		json_t *rndp2TransposeJ = json_object_get(rootJ, "rndp2Transpose");
		if (rndp2TransposeJ) rndp2Transpose = json_integer_value(rndp2TransposeJ);
		json_t *rndp2StartWithNoteJ = json_object_get(rootJ, "rndp2StartWithNote");
		if (rndp2StartWithNoteJ) rndp2StartWithNote = json_boolean_value(rndp2StartWithNoteJ);
		json_t *rndp2StartWithAccentJ = json_object_get(rootJ, "rndp2StartWithAccent");
		if (rndp2StartWithAccentJ) rndp2StartWithAccent = json_boolean_value(rndp2StartWithAccentJ);
		json_t *rndpScaleIndexJ = json_object_get(rootJ, "rndpScaleIndex");
		if (rndpScaleIndexJ) rndpScaleIndex = json_integer_value(rndpScaleIndexJ);
		json_t *instanceSaltJ = json_object_get(rootJ, "instanceSalt");
		if (instanceSaltJ) instanceSalt = json_integer_value(instanceSaltJ);

		resetNonJson(true);
	}
	
	
	IoStep* fillIoSteps(int *seqLenPtr) {// caller must delete return array
		int track = getEditTrack();
		int seqLen = trackLength[seqIndexEdit][track];
		IoStep* ioSteps = new IoStep[seqLen];

		// populate ioSteps array from current track
		for (int i = 0; i < seqLen; i++) {
			ioSteps[i].pitch = cv[seqIndexEdit][track][i];
			StepAttributes stepAttrib = attributes[seqIndexEdit][track][i];
			ioSteps[i].gate = stepAttrib.getGate();
			ioSteps[i].tied = stepAttrib.getTied();
			ioSteps[i].vel = -1.0f;// no concept of velocity in PhraseSequencers
			ioSteps[i].prob = stepAttrib.getGateP() ? params[PROB_KNOB_PARAM].getValue() : -1.0f;// negative means prob is not on for this note
		}

		// return values
		*seqLenPtr = seqLen;
		return ioSteps;
	}
	
	
	void emptyIoSteps(IoStep* ioSteps, int seqLen) {// seqLen is max 16 per track
		int track = getEditTrack();
		trackLength[seqIndexEdit][track] = seqLen;

		// populate steps in the current track
		// first pass is done without ties
		for (int i = 0; i < seqLen; i++) {
			cv[seqIndexEdit][track][i] = ioSteps[i].pitch;

			StepAttributes stepAttrib;
			stepAttrib.init();
			stepAttrib.setGate(ioSteps[i].gate);
			stepAttrib.setGateP(ioSteps[i].prob >= 0.0f);
			attributes[seqIndexEdit][track][i] = stepAttrib;
		}
		// now do ties, has to be done in a separate pass such that non tied that follows tied can be
		//   there in advance for proper gate types
		for (int i = 0; i < seqLen; i++) {
			if (ioSteps[i].tied) {
				activateTiedStep(seqIndexEdit, track, i);
			}
		}
	}
	

	void rotateSeq(int seqNum, bool directionRight, int seqLength, int track) {
		// Rotate the specified track within the sequence
		float rotCV;
		StepAttributes rotAttributes;
		int iEnd = seqLength - 1;
		int iRot = 0;
		int iDelta = 1;
		if (directionRight) {
			iRot = iEnd;
			iDelta = -1;
		}
		rotCV = cv[seqNum][track][iRot];
		rotAttributes = attributes[seqNum][track][iRot];
		for ( ; ; iRot += iDelta) {
			if (iDelta == 1 && iRot >= iEnd) break;
			if (iDelta == -1 && iRot <= 0) break;
			cv[seqNum][track][iRot] = cv[seqNum][track][iRot + iDelta];
			attributes[seqNum][track][iRot] = attributes[seqNum][track][iRot + iDelta];
		}
		cv[seqNum][track][iRot] = rotCV;
		attributes[seqNum][track][iRot] = rotAttributes;
	}
	

	void calcGateCode(StepAttributes attribute, int index) {
		int gateType = attribute.getGateMode();

		if (ppqnCount == 0 && !attribute.getTied()) {
			lastProbGateEnable[index] = !attribute.getGateP() || (random::uniform() < params[PROB_KNOB_PARAM].getValue());
		}

		if (!attribute.getGate() || !lastProbGateEnable[index]) {
			gateCode[index] = 0;
		}
		else if (pulsesPerStep == 1 && gateType == 0) {
			gateCode[index] = 2;// clock high
		}
		else {
			if (gateType == 11) {
				gateCode[index] = (ppqnCount == 0 ? 3 : 0);
			}
			else {
				gateCode[index] = getAdvGate(ppqnCount, pulsesPerStep, gateType);
			}
		}
	}

	// Check if step's condition allows it to play on current loop
	// Conditions: 0=1:1(always), 1=1:2(every 2nd), 2=1:4(every 4th), 3=1st only, 4=NOT 1st
	bool checkCondition(int cond, int loop) {
		switch (cond) {
			case 0: return true;// 1:1 - always play
			case 1: return (loop % 2) == 0;// 1:2 - every 2nd loop
			case 2: return (loop % 4) == 0;// 1:4 - every 4th loop
			case 3: return loop == 0;// 1st only
			case 4: return loop > 0;// NOT 1st
			default: return true;
		}
	}

	// Apply microtuning offset to CV
	float applyMicrotuning(float cvIn) {
		// Extract semitone from CV (0V = C4)
		int semitone = ((int)std::round(cvIn * 12.0f) % 12 + 12) % 12;
		// Convert cents to volts (100 cents = 1 semitone = 1/12 V)
		float centsOffset = microtuningCents[semitone] / 1200.0f;
		// Octave shift (1V per octave)
		float octaveOffset = (float)microtuningOctave;
		// Master tune offset: cents = 1200 * log2(masterTuneHz / 440.0)
		float masterCents = 1200.0f * std::log2(masterTuneHz / 440.0f);
		float masterOffset = masterCents / 1200.0f;
		return cvIn + centsOffset + octaveOffset + masterOffset;
	}

	void process(const ProcessArgs &args) override {
		float sampleRate = args.sampleRate;

		//********** Buttons, knobs, switches and inputs **********
		
		// Edit mode
		bool seqMode = isSeqMode();// true = editing sequence, false = editing song

		// Validate displayState when switching to SONG mode (CLOCKDIV not valid)
		if (!seqMode && displayState == DISP_CLOCKDIV)
			displayState = DISP_NORMAL;

		// Run button
		if (runningTrigger.process(params[RUN_PARAM].getValue() + inputs[RUNCV_INPUT].getVoltage())) {// no input refresh here, don't want to introduce startup skew
			running = !running;
			if (running) {
				if (resetOnRun) {
					initRun();
				}
			}
			displayState = DISP_NORMAL;
		}

		if (refresh.processInputs()) {
			// Track select switch - sync stepIndexEdit when manually changed
			int currentTrackSwitch = (int)(params[TRACKSELECT_PARAM].getValue() + 0.5f);
			if (currentTrackSwitch != lastTrackSwitch) {
				// Switch was manually changed, update stepIndexEdit to same step in new track
				int step = getEditStep();
				stepIndexEdit = (currentTrackSwitch == 0) ? step : (step + 16);
				lastTrackSwitch = currentTrackSwitch;
			}

			// Microtuning knob sync
			if (selectedMicroKey != lastSelectedMicroKey || temperamentPreset != lastTemperamentPreset) {
				// Key selection or preset changed, update knob to show new cents value
				params[MICRO_CENTS_PARAM].setValue(microtuningCents[selectedMicroKey]);
				lastSelectedMicroKey = selectedMicroKey;
				lastTemperamentPreset = temperamentPreset;
			} else {
				// Read knob and update the selected key's cents value
				float centsKnobValue = params[MICRO_CENTS_PARAM].getValue();
				microtuningCents[selectedMicroKey] = centsKnobValue;
			}
			microtuningOctave = (int)std::round(params[MICRO_OCTAVE_PARAM].getValue());
			masterTuneHz = params[MICRO_MASTER_PARAM].getValue();

			// Attach button
			if (attachedTrigger.process(params[ATTACH_PARAM].getValue())) {
				if (seqMode) {
					attached = !attached;
					displayState = DISP_NORMAL;
				}
				else {
					// In song mode, attach is disabled - flash 3 times to indicate
					attachFlashCounter = attachFlashTimeTicks;
				}
			}
			if (running && attached) {
				if (seqMode) {
					// Follow current track's run position
					int track = getEditTrack();
					stepIndexEdit = (track == 0) ? stepIndexRun[0] : (stepIndexRun[1] + 16);
				}
				else
					phraseIndexEdit = phraseIndexRun;
			}
			
			// Copy button
			if (copyTrigger.process(params[COPY_PARAM].getValue())) {
				if (!attached) {
					startCP = seqMode ? stepIndexEdit : phraseIndexEdit;
					int track = trackFromIndex(startCP);
					int stepInTrack = stepFromIndex(startCP);
					countCP = 16;
					if (params[CPMODE_PARAM].getValue() > 1.5f)// all
						startCP = trackStartIndex(track);
					else if (params[CPMODE_PARAM].getValue() < 0.5f)// 4
						countCP = std::min(4, 16 - stepInTrack);
					else// 8
						countCP = std::min(8, 16 - stepInTrack);
					if (seqMode) {
						for (int i = 0, s = stepInTrack; i < countCP; i++, s++) {
							cvCPbuffer[i] = cv[seqIndexEdit][track][s];
							attribCPbuffer[i] = attributes[seqIndexEdit][track][s];
						}
						seqCopied = true;
					}
					else {
						for (int i = 0, p = startCP; i < countCP; i++, p++)
							phraseCPbuffer[i] = phrase[p];
						seqCopied = false;// so that a cross paste can be detected
					}
					infoCopyPaste = revertDisplayTimeTicks;
					displayState = DISP_NORMAL;
				}
				else
					attachedWarning = warningTimeTicks;
			}
			// Paste button
			if (pasteTrigger.process(params[PASTE_PARAM].getValue())) {
				if (!attached) {
					int track = trackFromIndex(stepIndexEdit);
					int stepInTrack = stepFromIndex(stepIndexEdit);
					infoCopyPaste = -revertDisplayTimeTicks;
					startCP = trackStartIndex(track);
					if (countCP <= 8) {
						startCP = seqMode ? stepIndexEdit : phraseIndexEdit;
						countCP = std::min(countCP, 16 - stepInTrack);
					}
					// else nothing to do for ALL

					if (seqMode && seqCopied) {
						for (int i = 0, s = stepInTrack; i < countCP; i++, s++) {
							cv[seqIndexEdit][track][s] = cvCPbuffer[i];
							attributes[seqIndexEdit][track][s] = attribCPbuffer[i];
						}
					}
					else if (!seqMode && !seqCopied) {
						for (int i = 0, p = startCP; i < countCP; i++, p++)
							phrase[p] = phraseCPbuffer[i];
					}
					displayState = DISP_NORMAL;
				}
				else
					attachedWarning = warningTimeTicks;
			}

			// Clear track button (sequence mode only)
			if (clrTrigger.process(params[CLR_PARAM].getValue())) {
				if (seqMode && !attached) {
					int track = getEditTrack();
					for (int s = 0; s < 16; s++) {
						cv[seqIndexEdit][track][s] = 0.0f;
						attributes[seqIndexEdit][track][s].init();
						releaseTime[seqIndexEdit][track][s] = 100.0f;
					}
					trackLength[seqIndexEdit][track] = 16;
					trackRunMode[seqIndexEdit][track] = MODE_FWD;
					trackTranspose[seqIndexEdit][track] = 0;
					trackRotate[seqIndexEdit][track] = 0;
					trackClockDiv[seqIndexEdit][track] = 1;
				}
			}

			// Step button presses
			int stepPressed = -1;
			for (int i = 0; i < 32; i++) {
				if (stepTriggers[i].process(params[STEP_PHRASE_PARAMS + i].getValue()))
					stepPressed = i;
			}
			if (stepPressed != -1) {
				editCursorTimeout = editCursorTimeTicks;
				if (displayState == DISP_LENGTH) {
					if (seqMode) {
						// Set length for the track corresponding to the pressed button's row
						int pressedTrack = trackFromIndex(stepPressed);
						int pressedStep = stepFromIndex(stepPressed);
						trackLength[seqIndexEdit][pressedTrack] = pressedStep + 1;
						stepIndexEdit = stepPressed;// update edit cursor to reflect selected track
						params[TRACKSELECT_PARAM].setValue((float)pressedTrack);// sync switch to clicked track
						lastTrackSwitch = pressedTrack;
					}
					else
						phrases = stepPressed + 1;
					revertDisplay = lengthDisplayTimeTicks;
				}
				else {
					if (!running || !attached) {// not running or detached
						if (seqMode) {
							stepIndexEdit = stepPressed;
							int pressedTrack = trackFromIndex(stepPressed);
							params[TRACKSELECT_PARAM].setValue((float)pressedTrack);// sync switch to clicked track
							lastTrackSwitch = pressedTrack;
							int track = getEditTrack();
							int step = getEditStep();
							if (!attributes[seqIndexEdit][track][step].getTied()) {// play if non-tied step
								editingGate = gateTimeTicks;
								editingGateCV = cv[seqIndexEdit][track][step];
								editingGateKeyLight = -1;
								editingChannel = track;
							}
						}
						else {
							phraseIndexEdit = stepPressed;
							if (!running)
								phraseIndexRun = phraseIndexEdit;
						}
					}
					else {// attached and running
						attachedWarning = warningTimeTicks;
						if (seqMode) {
							// Follow the track that was clicked
							int pressedTrack = trackFromIndex(stepPressed);
							stepIndexEdit = (pressedTrack == 0) ? stepIndexRun[0] : (stepIndexRun[1] + 16);
							params[TRACKSELECT_PARAM].setValue((float)pressedTrack);// sync switch to clicked track
							lastTrackSwitch = pressedTrack;
						}
					}
					displayState = DISP_NORMAL;
				}
			} 
			
			// Mode/Length button
			if (modeTrigger.process(params[RUNMODE_PARAM].getValue())) {
				if (!attached) {
					editingPpqn = 0l;
					if (displayState == DISP_NORMAL || displayState == DISP_TRANSPOSE || displayState == DISP_ROTATE) {
						displayState = DISP_LENGTH;
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_LENGTH) {
						displayState = DISP_MODE;
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_MODE) {
						if (seqMode) {
							displayState = DISP_CLOCKDIV;
							revertDisplay = lengthDisplayTimeTicks;
						}
						else
							displayState = DISP_NORMAL;
					}
					else
						displayState = DISP_NORMAL;
					modeHoldDetect.start(holdDetectTimeTicks);
				}
				else
					attachedWarning = warningTimeTicks;
			}
			
			// Transpose/Rotate button
			if (transposeTrigger.process(params[TRAN_ROT_PARAM].getValue())) {
				if (seqMode && !attached) {
					if (displayState == DISP_NORMAL || displayState == DISP_MODE || displayState == DISP_CLOCKDIV || displayState == DISP_LENGTH) {
						displayState = DISP_TRANSPOSE;
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_TRANSPOSE) {
						displayState = DISP_ROTATE;
						revertDisplay = lengthDisplayTimeTicks;
					}
					else
						displayState = DISP_NORMAL;
				}
				else if (attached)
					attachedWarning = warningTimeTicks;
			}			
			
			// Data encoder
			float seqParamValue = params[ENCODER_PARAM].getValue();
			int newEncoderState = (int)std::round(seqParamValue * 7.0f);
			if (seqParamValue == 0.0f)// true when constructor or dataFromJson() occured
				encoderState = newEncoderState;
			int deltaKnob = newEncoderState - encoderState;
			if (deltaKnob != 0) {
				if (abs(deltaKnob) <= 3) {// avoid discontinuous step (initialize for example)
					// any changes in here should may also require right click behavior to be updated in the knob's onMouseDown()
					if (editingPpqn != 0) {
						pulsesPerStep = indexToPps(ppsToIndex(pulsesPerStep) + deltaKnob);
						editingPpqn = editGateLengthTimeTicks;
					}
					else if (displayState == DISP_MODE) {
						if (seqMode) {
							int track = getEditTrack();
							trackRunMode[seqIndexEdit][track] = clamp(trackRunMode[seqIndexEdit][track] + deltaKnob, 0, NUM_SEQ_MODES - 1);
						}
						else {
							// Find current position in songModes array
							int songModeIdx = 0;
							for (int i = 0; i < NUM_SONG_MODES; i++) {
								if (songModes[i] == runModeSong) { songModeIdx = i; break; }
							}
							songModeIdx = clamp(songModeIdx + deltaKnob, 0, NUM_SONG_MODES - 1);
							runModeSong = songModes[songModeIdx];
						}
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_CLOCKDIV) {
						if (seqMode) {
							int track = getEditTrack();
							trackClockDiv[seqIndexEdit][track] = clamp(trackClockDiv[seqIndexEdit][track] + deltaKnob, 1, 5);
						}
					}
					else if (displayState == DISP_LENGTH) {
						if (seqMode) {
							int track = getEditTrack();
							trackLength[seqIndexEdit][track] = clamp(trackLength[seqIndexEdit][track] + deltaKnob, 1, 16);
						}
						else {
							phrases = clamp(phrases + deltaKnob, 1, 32);
						}
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_TRANSPOSE) {
						if (seqMode) {
							int track = getEditTrack();
							int oldTranspose = trackTranspose[seqIndexEdit][track];
							trackTranspose[seqIndexEdit][track] = clamp(oldTranspose + deltaKnob, -24, 24);
							int actualDelta = trackTranspose[seqIndexEdit][track] - oldTranspose;
							if (actualDelta != 0) {
								float transposeOffsetCV = ((float)(actualDelta))/12.0f;
								for (int s = 0; s < 16; s++)
									cv[seqIndexEdit][track][s] += transposeOffsetCV;
							}
						}
						revertDisplay = lengthDisplayTimeTicks;
					}
					else if (displayState == DISP_ROTATE) {
						if (seqMode) {
							int track = getEditTrack();
							int slength = trackLength[seqIndexEdit][track];
							int oldRotate = trackRotate[seqIndexEdit][track];
							trackRotate[seqIndexEdit][track] = clamp(oldRotate + deltaKnob, -16, 16);
							int actualDelta = trackRotate[seqIndexEdit][track] - oldRotate;
							if (actualDelta > 0) {
								for (int i = actualDelta; i > 0; i--) {
									rotateSeq(seqIndexEdit, true, slength, track);
									int step = getEditStep();
									if (step < slength)
										stepIndexEdit = (track == 0) ? ((step + 1) % slength) : (((step + 1) % slength) + 16);
								}
							}
							else if (actualDelta < 0) {
								for (int i = actualDelta; i < 0; i++) {
									rotateSeq(seqIndexEdit, false, slength, track);
									int step = getEditStep();
									if (step < slength)
										stepIndexEdit = (track == 0) ? ((step + slength - 1) % slength) : (((step + slength - 1) % slength) + 16);
								}
							}
						}
						revertDisplay = lengthDisplayTimeTicks;
					}
					else {// DISP_NORMAL
						if (seqMode) {
							seqIndexEdit = clamp(seqIndexEdit + deltaKnob, 0, 32 - 1);
						}
						else {
							if (!attached || !running) {
								int newPhrase = phrase[phraseIndexEdit] + deltaKnob;
								if (newPhrase < 0)
									newPhrase += (1 - newPhrase / 32) * 32;// newPhrase now positive
								newPhrase = newPhrase % 32;
								phrase[phraseIndexEdit] = newPhrase;
							}
							else 
								attachedWarning = warningTimeTicks;
						}
					}
				}
				encoderState = newEncoderState;
			}

			// Octave buttons
			for (int i = 0; i < 5; i++) {
				if (octTriggers[i].process(params[OCTAVE_PARAM + i].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						int track = getEditTrack();
						int step = getEditStep();
						if (attributes[seqIndexEdit][track][step].getTied())
							tiedWarning = warningTimeTicks;
						else {
							cv[seqIndexEdit][track][step] = applyNewOct(cv[seqIndexEdit][track][step], 2 - i);
							propagateCVtoTied(seqIndexEdit, track, step);
							editingGate = gateTimeTicks;
							editingGateCV = cv[seqIndexEdit][track][step];
							editingGateKeyLight = -1;
							editingChannel = track;
						}
					}
				}
			}
			
			// Keyboard buttons
			if (keyTrigger.process(pkInfo.gate)) {
				if (seqMode) {
					displayState = DISP_NORMAL;
					int track = getEditTrack();
					int step = getEditStep();
					if (editingGateLength != 0l) {
						int newMode = keyIndexToGateMode(pkInfo.key, pulsesPerStep);
						if (newMode != -1) {
							editingPpqn = 0l;
							attributes[seqIndexEdit][track][step].setGateMode(newMode);
							if (pkInfo.isRightClick) {
								stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 32);
								track = getEditTrack();
								step = getEditStep();
								editingType = gateTimeTicks;
								editingGateKeyLight = pkInfo.key;
								if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
									attributes[seqIndexEdit][track][step].setGateMode(newMode);
							}
						}
						else
							editingPpqn = editGateLengthTimeTicks;
					}
					else if (attributes[seqIndexEdit][track][step].getTied()) {
						if (pkInfo.isRightClick)
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 32);
						else
							tiedWarning = warningTimeTicks;
					}
					else {
						float newCV = std::floor(cv[seqIndexEdit][track][step]) + ((float) pkInfo.key) / 12.0f;
						cv[seqIndexEdit][track][step] = newCV;
						propagateCVtoTied(seqIndexEdit, track, step);
						editingGate = gateTimeTicks;
						editingGateCV = cv[seqIndexEdit][track][step];
						editingGateKeyLight = -1;
						editingChannel = track;
						if (pkInfo.isRightClick) {
							stepIndexEdit = moveIndex(stepIndexEdit, stepIndexEdit + 1, 32);
							track = getEditTrack();
							step = getEditStep();
							editingGateKeyLight = pkInfo.key;
							if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
								cv[seqIndexEdit][track][step] = newCV;
						}
					}
				}
			}
			
			// Keyboard mode (note or gate type)
			if (keyNoteTrigger.process(params[KEYNOTE_PARAM].getValue())) {
				editingGateLength = 0l;
			}
			if (keyGateTrigger.process(params[KEYGATE_PARAM].getValue())) {
				editingGateLength = 1l;
			}

			// Gate, Prob, Slide and Tied buttons
			{
				int track = getEditTrack();
				int step = getEditStep();
				if (gateTrigger.process(params[GATE_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						attributes[seqIndexEdit][track][step].toggleGate();
					}
				}
				if (probTrigger.process(params[PROB_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						if (attributes[seqIndexEdit][track][step].getTied())
							tiedWarning = warningTimeTicks;
						else
							attributes[seqIndexEdit][track][step].toggleGateP();
					}
				}
				if (slideTrigger.process(params[SLIDE_BTN_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						if (attributes[seqIndexEdit][track][step].getTied())
							tiedWarning = warningTimeTicks;
						else
							attributes[seqIndexEdit][track][step].toggleSlide();
					}
				}
				if (accentTrigger.process(params[ACCENT_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						if (attributes[seqIndexEdit][track][step].getTied())
							tiedWarning = warningTimeTicks;
						else
							attributes[seqIndexEdit][track][step].toggleAccent();
					}
				}
				if (tiedTrigger.process(params[TIE_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						if (attributes[seqIndexEdit][track][step].getTied()) {
							deactivateTiedStep(seqIndexEdit, track, step);
						}
						else {
							activateTiedStep(seqIndexEdit, track, step);
						}
					}
				}
				if (relTrigger.process(params[REL_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						attributes[seqIndexEdit][track][step].toggleRelease();
						// When enabling release, store current knob value
						if (attributes[seqIndexEdit][track][step].getRelease()) {
							releaseTime[seqIndexEdit][track][step] = params[REL_KNOB_PARAM].getValue();
						}
					}
				}
				if (condTrigger.process(params[COND_PARAM].getValue())) {
					if (seqMode) {
						displayState = DISP_NORMAL;
						attributes[seqIndexEdit][track][step].cycleCond();
					}
				}
				// REL_KNOB: continuously update current step's releaseTime when release is enabled
				if (seqMode) {
					float relKnobValue = params[REL_KNOB_PARAM].getValue();
					if (relKnobValue != lastRelKnobValue) {
						if (attributes[seqIndexEdit][track][step].getRelease()) {
							releaseTime[seqIndexEdit][track][step] = relKnobValue;
						}
						lastRelKnobValue = relKnobValue;
					}
				}
			}
		}// userInputs refresh


		//********** Clock and reset **********

		// Clock
		if (running && clockIgnoreOnReset == 0l) {
			if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
				ppqnCount++;
				if (ppqnCount >= pulsesPerStep)
					ppqnCount = 0;

				int newSeq = seqIndexEdit;// good value when seqMode, overwrite if not seqMode
				bool trackAdvanced[2] = {false, false};// track which tracks actually advanced (for clock division)
				if (ppqnCount == 0) {
					float slideFromCV[2] = {0.0f, 0.0f};
					int oldStepIndexRun0 = stepIndexRun[0];
					int oldStepIndexRun1 = stepIndexRun[1];
					if (seqMode) {
						// Store current CV for slide calculation (both tracks)
						for (int t = 0; t < 2; t++)
							slideFromCV[t] = cv[seqIndexEdit][t][stepIndexRun[t]];
						// Advance both tracks independently with clock division
						for (int t = 0; t < 2; t++) {
							clockDivCounter[t]++;
							if (clockDivCounter[t] >= trackClockDiv[seqIndexEdit][t]) {
								clockDivCounter[t] = 0;
								trackAdvanced[t] = true;
								if (moveIndexRunMode(&stepIndexRun[t], trackLength[seqIndexEdit][t], trackRunMode[seqIndexEdit][t], &stepIndexRunHistory[t]))
									loopCount[t]++;// increment when track loops
							}
						}
					}
					else {
						// Store current CV for slide calculation (both tracks)
						for (int t = 0; t < 2; t++)
							slideFromCV[t] = cv[phrase[phraseIndexRun]][t][stepIndexRun[t]];
						// Track A clock division check
						clockDivCounter[0]++;
						bool advanceTrackA = (clockDivCounter[0] >= trackClockDiv[phrase[phraseIndexRun]][0]);
						if (advanceTrackA) {
							clockDivCounter[0] = 0;
							trackAdvanced[0] = true;
							// Advance track A - if it loops, advance phrase
							if (moveIndexRunMode(&stepIndexRun[0], trackLength[phrase[phraseIndexRun]][0], trackRunMode[phrase[phraseIndexRun]][0], &stepIndexRunHistory[0])) {
								int oldPhraseIndexRun = phraseIndexRun;
								bool songLoopOver = moveIndexRunMode(&phraseIndexRun, phrases, runModeSong, &phraseIndexRunHistory);
								// RN2 non-repeating for song mode - ensure different slot
								if (runModeSong == MODE_RN2 && phrases > 1) {
									phraseIndexRun = random::u32() % (phrases - 1);
									if (phraseIndexRun >= oldPhraseIndexRun)
										phraseIndexRun++;
								}
								// check for end of song if needed
								if (songLoopOver && stopAtEndOfSong) {
									running = false;
									stepIndexRun[0] = oldStepIndexRun0;
									stepIndexRun[1] = oldStepIndexRun1;
									phraseIndexRun = oldPhraseIndexRun;
								}
								else {
									// Reset both tracks to start of new phrase using per-track run modes
									for (int t = 0; t < 2; t++) {
										stepIndexRun[t] = (trackRunMode[phrase[phraseIndexRun]][t] == MODE_REV ? trackLength[phrase[phraseIndexRun]][t] - 1 : 0);
										loopCount[t] = 0;// reset loop count on phrase change
										clockDivCounter[t] = 0;// reset clock dividers on phrase change
									}
									trackAdvanced[1] = true;// track B also moved to new position
								}
							}
						}
						// Track B clock division - only if track A didn't change phrase
						if (!trackAdvanced[1]) {
							clockDivCounter[1]++;
							if (clockDivCounter[1] >= trackClockDiv[phrase[phraseIndexRun]][1]) {
								clockDivCounter[1] = 0;
								trackAdvanced[1] = true;
								if (moveIndexRunMode(&stepIndexRun[1], trackLength[phrase[phraseIndexRun]][1], trackRunMode[phrase[phraseIndexRun]][1], &stepIndexRunHistory[1]))
									loopCount[1]++;// increment when track B loops
							}
						}
						newSeq = phrase[phraseIndexRun];
					}
					if (running)// end of song may have stopped it
						fillStepIndexRunVector();

					// Slide - process only tracks that advanced (respecting clock division)
					for (int t = 0; t < 2; t++) {
						if (trackAdvanced[t]) {
							if (attributes[newSeq][t][stepIndexRun[t]].getSlide()) {
								slideStepsRemain[t] = (unsigned long) (((float)clockPeriod * pulsesPerStep) * params[SLIDE_KNOB_PARAM].getValue() / 2.0f);
								if (slideStepsRemain[t] != 0ul) {
									float slideToCV = cv[newSeq][t][stepIndexRun[t]];
									slideCVdelta[t] = (slideToCV - slideFromCV[t])/(float)slideStepsRemain[t];
								}
							}
							else
								slideStepsRemain[t] = 0ul;
						}
						// Update stepActive at step boundaries
						stepActive[t] = trackAdvanced[t];
					}
				}
				else {
					if (!seqMode)
						newSeq = phrase[phraseIndexRun];
				}
				// Calculate gate codes on every clock (for proper gate shapes)
				for (int t = 0; t < 2; t++) {
					calcGateCode(attributes[newSeq][t][stepIndexRun[t]], t);
				}
				clockPeriod = 0ul;
			}
			clockPeriod++;
		}
		
		// Reset
		if (resetTrigger.process(inputs[RESET_INPUT].getVoltage() + params[RESET_PARAM].getValue())) {
			initRun();
			resetLight = 1.0f;
			displayState = DISP_NORMAL;
			clockTrigger.reset();
		}
		
		
		//********** Outputs and lights **********

		// CV and gates outputs (always output both tracks)
		int seq = seqMode ? seqIndexEdit : phrase[phraseIndexRun];
		if (running) {
			// Mute logic: live mute selected track, other track keeps playing
			bool muteGate[2];
			muteGate[0] = muteGate[1] = !seqMode && (params[GATE_PARAM].getValue() > 0.5f);
			if (!attached && muteGate[0]) {
				int track = getEditTrack();
				muteGate[1 - track] = false;  // unmute the other track
			}

			// Calculate slide offsets and conditions for both tracks
			float slideOffset[2];
			bool cond[2];
			for (int t = 0; t < 2; t++) {
				slideOffset[t] = (slideStepsRemain[t] > 0ul ? (slideCVdelta[t] * (float)slideStepsRemain[t]) : 0.0f);
				cond[t] = checkCondition(attributes[seq][t][stepIndexRun[t]].getCond(), loopCount[t]);
			}

			bool retriggingOnReset = (clockIgnoreOnReset != 0l && calcRGOR(retrigGatesOnReset, &inputs[RUNCV_INPUT]));

			// Output CV and gates for both tracks
			for (int t = 0; t < 2; t++) {
				outputs[CV_OUTPUTS[t]].setVoltage(applyMicrotuning(cv[seq][t][stepIndexRun[t]] - slideOffset[t]));
				outputs[GATE_OUTPUTS[t]].setVoltage((calcGate(gateCode[t], clockTrigger, clockPeriod, sampleRate) && stepActive[t] && !muteGate[t] && !retriggingOnReset && cond[t]) ? 10.0f : 0.0f);
			}
		}
		else {// not running
			int editTrack = getEditTrack();
			int editStep = getEditStep();

			// Calculate CV for each track
			float trackCv[2];
			for (int t = 0; t < 2; t++) {
				int step = (editTrack == t) ? editStep : stepIndexRun[t];
				trackCv[t] = cv[seq][t][step];
			}

			// Output: selected track gets audition CV/gate, other track silent
			for (int t = 0; t < 2; t++) {
				bool isEditingTrack = (editingChannel == t);
				float cvOut = (isEditingTrack && editingGate > 0ul) ? editingGateCV : trackCv[t];
				float gateOut = (isEditingTrack && editingGate > 0ul) ? 10.0f : 0.0f;
				outputs[CV_OUTPUTS[t]].setVoltage(applyMicrotuning(cvOut));
				outputs[GATE_OUTPUTS[t]].setVoltage(gateOut);
			}
		}
		for (int t = 0; t < 2; t++)
			if (slideStepsRemain[t] > 0ul)
				slideStepsRemain[t]--;

		// Expander communication
		// Fine CV from expander (reset each sample, applied later)
		float fineCv[2] = {0.0f, 0.0f};
		float transposeCv = 0.0f;

		if (rightExpander.module && rightExpander.module->model == modelTwinSlideExpander) {
			// Store internal values before any override (for expander output)
			float internalCv[2], internalGate[2], internalSlide[2];
			for (int t = 0; t < 2; t++) {
				internalCv[t] = outputs[CV_OUTPUTS[t]].getVoltage();
				internalGate[t] = outputs[GATE_OUTPUTS[t]].getVoltage();
				internalSlide[t] = slideStepsRemain[t] > 0 ? 10.f : 0.f;
			}

			// RECEIVE FROM EXPANDER: Read OWN rightExpander.consumerMessage
			// Message format: [0-3] Track A (CV,Gate,Slide,Fine), [4-7] Track B, [8] presence, [9-16] connection flags
			float *fromExpander = static_cast<float*>(rightExpander.consumerMessage);
			if (fromExpander[8] > 0.5f) {  // Presence flag
				// Override outputs and state if expander inputs connected
				for (int t = 0; t < 2; t++) {
					int dataOffset = t * 4;   // 0 for A, 4 for B
					int flagOffset = t * 4 + 9;  // 9 for A, 13 for B
					if (fromExpander[flagOffset] > 0.5f)      // CV connected
						outputs[CV_OUTPUTS[t]].setVoltage(fromExpander[dataOffset]);
					if (fromExpander[flagOffset + 1] > 0.5f)  // Gate connected
						outputs[GATE_OUTPUTS[t]].setVoltage(fromExpander[dataOffset + 1] > 1.0f ? 10.0f : 0.0f);
					if (fromExpander[flagOffset + 2] > 0.5f)  // Slide connected
						slideStepsRemain[t] = fromExpander[dataOffset + 2] > 1.0f ? 1 : 0;
					if (fromExpander[flagOffset + 3] > 0.5f)  // Fine connected
						fineCv[t] = fromExpander[dataOffset + 3] / 5.0f;
				}
			}
			transposeCv = fromExpander[17];

			// SEND TO EXPANDER: Always send internal sequencer values (not overridden)
			float *toExpander = static_cast<float*>(rightExpander.module->leftExpander.producerMessage);
			for (int t = 0; t < 2; t++) {
				int offset = t * 3;  // 0 for A, 3 for B
				toExpander[offset] = internalCv[t];
				toExpander[offset + 1] = internalGate[t];
				toExpander[offset + 2] = internalSlide[t];
			}
			// Accent outputs [6] and [7]
			int expSeq = isSeqMode() ? seqIndexEdit : phrase[phraseIndexRun];
			for (int t = 0; t < 2; t++) {
				int expStep = running ? stepIndexRun[t] : getEditStep();
				bool internalAccent = attributes[expSeq][t][expStep].getAccent();
				toExpander[6 + t] = internalAccent ? 10.0f : 0.0f;
			}

			// REQUEST FLIP on expander's left side
			rightExpander.module->leftExpander.messageFlipRequested = true;
		}

		// Synth processing
		if (outputs[AUDIOA_OUTPUT].isConnected() || outputs[AUDIOB_OUTPUT].isConnected()) {
			int seq = isSeqMode() ? seqIndexEdit : phrase[phraseIndexRun];

			// Get current step, pitch, gate, accent, slide for each track
			int step[2];
			float pitch[2];
			bool gate[2], accent[2], slide[2];
			for (int t = 0; t < 2; t++) {
				step[t] = running ? stepIndexRun[t] : getEditStep();
				pitch[t] = outputs[CV_OUTPUTS[t]].getVoltage() + transposeCv;
				gate[t] = outputs[GATE_OUTPUTS[t]].getVoltage() >= 1.0f;
				accent[t] = attributes[seq][t][step[t]].getAccent();
				slide[t] = attributes[seq][t][step[t]].getSlide();
			}

			// Get synth parameters for both voices
			float cutoff[2], cutoffCv[2], res[2], envMod[2], decay[2], accentAmt[2], drive[2], fine[2];
			int wave[2];
			float slideTime = params[SLIDE_KNOB_PARAM].getValue();
			ExpertSettings* expert[2] = {&expertA, &expertB};
			for (int t = 0; t < 2; t++) {
				cutoff[t] = params[CUTOFF_PARAMS[t]].getValue();
				cutoffCv[t] = clamp(inputs[CUTOFF_INPUTS[t]].getVoltage(), -10.f, 10.f);
				res[t] = params[RES_PARAMS[t]].getValue();
				envMod[t] = clamp(params[ENVMOD_PARAMS[t]].getValue() + clamp(inputs[ENVMOD_INPUTS[t]].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);
				decay[t] = clamp(params[DECAY_PARAMS[t]].getValue() + clamp(inputs[DECAY_INPUTS[t]].getVoltage(), -10.f, 10.f) * 0.1f, 0.f, 1.f);
				accentAmt[t] = params[ACCENT_KNOB_PARAMS[t]].getValue();
				drive[t] = params[DRIVE_PARAMS[t]].getValue();
				fine[t] = clamp(params[FINE_PARAMS[t]].getValue() + fineCv[t], -1.0f, 1.0f);
				wave[t] = (int)(params[WAVE_PARAMS[t]].getValue() + 0.5f);
				if (wave[t] != prevWave[t]) {
					expert[t]->blend = (wave[t] == 0) ? 0.0f : 1.0f;
					prevWave[t] = wave[t];
				}
			}

			// Latch release amounts on gate rising edge (so release uses correct step's value)
			// releaseAmt = (releaseTimeMs - 21.0f) / 479.0f converts ms to 0-1 range
			for (int t = 0; t < 2; t++) {
				if (gate[t] && !prevGate[t]) {
					latchedReleaseAmt[t] = attributes[seq][t][step[t]].getRelease()
						? (releaseTime[seq][t][step[t]] - 21.0f) / 479.0f
						: expert[t]->release;
				}
				prevGate[t] = gate[t];
			}

			// Process voices and output audio
			for (int t = 0; t < 2; t++) {
				float audio = voice[t].process(pitch[t], gate[t], accent[t], slide[t],
				                               cutoff[t], cutoffCv[t], res[t], envMod[t], decay[t], accentAmt[t], drive[t], fine[t], expert[t]->blend, slideTime,
				                               expert[t]->vcfRange, expert[t]->decayRange, expert[t]->sustain, latchedReleaseAmt[t],
				                               expert[t]->bassComp);
				float level = params[LEVEL_PARAMS[t]].getValue();
				float out = audio * std::min(level, 0.75f) / 0.75f;
				if (level > 0.75f) {
					float satAmt = (level - 0.75f) / 0.25f;
					float satDrive = 1.0f + satAmt * 3.0f;
					float normalized = out / 5.0f;
					normalized = std::tanh(normalized * satDrive) / std::tanh(satDrive);
					out = normalized * 5.0f;
				}
				outputs[AUDIO_OUTPUTS[t]].setVoltage(clamp(out, -10.f, 10.f));
			}
		}

		// lights
		if (refresh.processLights()) {
			// Decrement edit cursor timeout (only when running)
			if (editCursorTimeout > 0 && running)
				editCursorTimeout--;

			// Increment flash counter for blinking effects
			flashCounter++;

			// Increment pulse phase for key light (slow pulse ~1Hz)
			keyLightPulsePhase += 0.02f;
			if (keyLightPulsePhase > 2.0f * M_PI) keyLightPulsePhase -= 2.0f * M_PI;
			float keyLightPulse = 0.35f + 0.15f * std::sin(keyLightPulsePhase);// 0.2 to 0.5 range

			// Step/phrase lights - both rows active (Track A = 0-15, Track B = 16-31)
			for (int i = 0; i < 32; i++) {
				int track = trackFromIndex(i);
				int col = stepFromIndex(i);// step within track (0-15)
				float amber = 0.0f;
				float white = 0.0f;
				if (infoCopyPaste != 0l) {
					if (i >= startCP && i < (startCP + countCP))
						amber = 0.71f;
				}
				else if (displayState == DISP_LENGTH) {
					if (seqMode) {
						if (track == getEditTrack()) {
							// Show length for selected track only
							if (col < (trackLength[seqIndexEdit][track] - 1))
								amber = 0.32f;
							else if (col == (trackLength[seqIndexEdit][track] - 1))
								amber = 1.0f;
						}
					}
					else {
						if (i < phrases - 1)
							amber = 0.32f;
						else
							amber = (i == phrases - 1) ? 1.0f : 0.0f;
					}
				}
				else if (displayState == DISP_MODE) {
					if (seqMode) {
						if (track == getEditTrack()) {
							// Pulse selected track row at ~2Hz
							if ((flashCounter / 64) % 2 == 0)
								amber = 0.71f;
						}
					}
					else {
						if (i < phrases - 1)
							amber = 0.32f;
						else
							amber = (i == phrases - 1) ? 1.0f : 0.0f;
					}
				}
				else if (displayState == DISP_CLOCKDIV) {
					if (track == getEditTrack()) {
						if ((flashCounter / 64) % 2 == 0)
							amber = 0.71f;
					}
				}
				else if (displayState == DISP_TRANSPOSE) {
					if (track == getEditTrack()) {
						amber = 0.71f;
					}
				}
				else if (displayState == DISP_ROTATE) {
					if (track == getEditTrack()) {
						amber = (i == stepIndexEdit ? 1.0f : (col < trackLength[seqIndexEdit][track] ? 0.45f : 0.0f));
					}
				}
				else {// normal led display (i.e. not length)
					int seq = seqMode ? seqIndexEdit : phrase[phraseIndexRun];
					bool stepInRange = (col < trackLength[seq][track]);

					// Song mode: show dimmed amber for phrase slots within song length
					if (!seqMode && i < phrases) {
						amber = 0.1f;
					}

					// Edit cursor (white) - stays on when not running, times out when running
					if (editCursorTimeout > 0 || !running) {
						if (seqMode)
							white = (i == stepIndexEdit ? 0.5f : 0.0f);
						else
							white = (i == phraseIndexEdit ? 0.5f : 0.0f);
					}

					// Only show other indicators for steps within track length
					if (stepInRange) {
						// Run cursor (amber) - each track has its own run position
						if (seqMode)
							amber = ((running && (col == stepIndexRun[track])) ? 1.0f : 0.0f);
						else {
							// Pulse the current phrase slot in white 4 times per 16 steps (on beats)
							if (running && (i == phraseIndexRun)) {
								float pulseBrightness = ((stepIndexRun[0] % 4) == 0) ? 1.0f : 0.08f;
								white = std::max(white, pulseBrightness);
							}
						}
						// Gate indicator
						bool gate = false;
						if (seqMode)
							gate = attributes[seqIndexEdit][track][col].getGate();
						else if (!seqMode && (attached && running))
							gate = attributes[phrase[phraseIndexRun]][track][col].getGate();
						bool isEditCursor = (editCursorTimeout > 0 || !running) && ((seqMode && i == stepIndexEdit) || (!seqMode && i == phraseIndexEdit));
						bool isPhraseRunPulse = (!seqMode && running && (i == phraseIndexRun));
						if (!isEditCursor && !isPhraseRunPulse) {
							if (amber == 0.0f && gate && displayState != DISP_MODE) {
								amber = 0.14f;// dim amber for gate indicator
							}
						}
					}
					// Steps beyond track length stay dark
				}
				setAmber(STEP_PHRASE_LIGHTS + i * 3, amber);
				lights[STEP_PHRASE_LIGHTS + i * 3 + 2].setBrightness(white);
			}
		
			// Calculate edit track/step once for octave and keyboard lights
			int editTrack = getEditTrack();
			int editStep = getEditStep();
			float cvVal = seqMode ? cv[seqIndexEdit][editTrack][editStep] : cv[phrase[phraseIndexEdit]][0][stepIndexRun[0]];
			int keyLightIndex;
			int octLightIndex;
			calcNoteAndOct(cvVal, &keyLightIndex, &octLightIndex);
			octLightIndex += 2;

			// Octave lights (amber at 0.5 brightness)
			for (int i = 0; i < 5; i++) {
				if (!seqMode && (!attached || !running))// no oct lights when song mode and either detached or stopped
					setAmber(OCTAVE_LIGHTS + i * 2, 0.0f);
				else {
					if (tiedWarning > 0l) {
						bool warningFlashState = calcWarningFlash(tiedWarning, warningTimeTicks);
						bool isActive = warningFlashState && (i == (4 - octLightIndex));
						setAmber(OCTAVE_LIGHTS + i * 2, isActive ? 0.5f : 0.0f);
					}
					else {
						bool isActive = (i == (4 - octLightIndex));
						setAmber(OCTAVE_LIGHTS + i * 2, isActive ? 0.5f : 0.0f);
					}
				}
			}

			// Keyboard lights (white)
			if (editingPpqn != 0) {
				for (int i = 0; i < 12; i++) {
					setAmber(KEY_LIGHTS + i * 3, 0.0f);// clear amber
					if (keyIndexToGateMode(i, pulsesPerStep) != -1) {
						lights[KEY_LIGHTS + i * 3 + 2].setBrightness(1.0f);
					}
					else {
						lights[KEY_LIGHTS + i * 3 + 2].setBrightness(0.0f);
					}
				}
			}
			else if (editingGateLength != 0l && seqMode) {
				int modeLightIndex = gateModeToKeyLightIndex(attributes[seqIndexEdit][editTrack][editStep]);
				for (int i = 0; i < 12; i++) {
					lights[KEY_LIGHTS + i * 3 + 0].setBrightness(0.0f);
					lights[KEY_LIGHTS + i * 3 + 1].setBrightness(0.0f);
					if (editingType > 0ul) {
						if (i == editingGateKeyLight) {
							float dimMult = ((float) editingType / (float)gateTimeTicks);
							lights[KEY_LIGHTS + i * 3 + 2].setBrightness(keyLightPulse * dimMult);
						}
						else {
							lights[KEY_LIGHTS + i * 3 + 2].setBrightness(i == keyLightIndex ? 0.15f : 0.0f);
						}
					}
					else {
						if (i == modeLightIndex) {
							lights[KEY_LIGHTS + i * 3 + 2].setBrightness(keyLightPulse);
						}
						else { // show dim white for note position
							lights[KEY_LIGHTS + i * 3 + 2].setBrightness(i == keyLightIndex ? 0.15f : 0.0f);
						}
					}
				}
			}
			else {
				// Note mode - white at 0.5 brightness
				for (int i = 0; i < 12; i++) {
					lights[KEY_LIGHTS + i * 3 + 0].setBrightness(0.0f);
					lights[KEY_LIGHTS + i * 3 + 1].setBrightness(0.0f);
					if (!seqMode && (!attached || !running))// no lights when song mode and either detached or stopped
						lights[KEY_LIGHTS + i * 3 + 2].setBrightness(0.0f);
					else {
						if (tiedWarning > 0l) {
							bool warningFlashState = calcWarningFlash(tiedWarning, warningTimeTicks);
							lights[KEY_LIGHTS + i * 3 + 2].setBrightness((warningFlashState && i == keyLightIndex) ? keyLightPulse : 0.0f);
						}
						else {
							if (editingGate > 0ul && editingGateKeyLight != -1)
								lights[KEY_LIGHTS + i * 3 + 2].setBrightness(i == editingGateKeyLight ? ((float) editingGate / (float)gateTimeTicks) * keyLightPulse : 0.0f);
							else
								lights[KEY_LIGHTS + i * 3 + 2].setBrightness(i == keyLightIndex ? keyLightPulse : 0.0f);
						}
					}
				}
			}		

			// Key mode light (note or gate type)
			lights[KEYNOTE_LIGHT].setBrightness(editingGateLength == 0l ? 0.5f : 0.0f);
			lights[KEYGATE_LIGHT].setBrightness(editingGateLength == 0l ? 0.0f : 0.5f);
			
			// Gate, Prob, Slide, Accent, Tied, Release and Condition lights (all white)
			if (!seqMode && (!attached || !running)) {// no gate lights when song mode and either detached or stopped
				setGateLight(false, GATE_LIGHT);
				lights[PROB_LIGHT].setBrightness(0.0f);
				lights[SLIDE_LIGHT].setBrightness(0.0f);
				lights[ACCENT_LIGHT].setBrightness(0.0f);
				lights[TIE_LIGHT].setBrightness(0.0f);
				lights[REL_LIGHT].setBrightness(0.0f);
				lights[COND_LIGHT].setBrightness(0.0f);
			}
			else {
				StepAttributes attributesVal = attributes[seqIndexEdit][editTrack][editStep];
				if (!seqMode)
					attributesVal = attributes[phrase[phraseIndexEdit]][0][stepIndexRun[0]];
				//
				setGateLight(attributesVal.getGate(), GATE_LIGHT);
				lights[PROB_LIGHT].setBrightness(attributesVal.getGateP() ? 0.5f : 0.0f);
				lights[SLIDE_LIGHT].setBrightness(attributesVal.getSlide() ? 0.5f : 0.0f);
				lights[ACCENT_LIGHT].setBrightness(attributesVal.getAccent() ? 0.5f : 0.0f);
				if (tiedWarning > 0l) {
					bool warningFlashState = calcWarningFlash(tiedWarning, warningTimeTicks);
					lights[TIE_LIGHT].setBrightness(warningFlashState ? 0.5f : 0.0f);
				}
				else
					lights[TIE_LIGHT].setBrightness(attributesVal.getTied() ? 0.5f : 0.0f);
				lights[REL_LIGHT].setBrightness(attributesVal.getRelease() ? 0.5f : 0.0f);
				lights[COND_LIGHT].setBrightness(attributesVal.getCond() != 0 ? 0.5f : 0.0f);
			}
			
			// Attach light (amber)
			if (attachFlashCounter > 0l) {
				// 2 pulses over 0.8 seconds: divide into 4 phases (on-off-on-off)
				int phase = (int)((attachFlashCounter * 4) / attachFlashTimeTicks);
				bool flashOn = (phase % 2 == 1);// odd phases are ON
				setAmber(ATTACH_LIGHT, flashOn ? 0.5f : 0.0f);
			}
			else if (attachedWarning > 0l) {
				bool warningFlashState = calcWarningFlash(attachedWarning, warningTimeTicks);
				setAmber(ATTACH_LIGHT, warningFlashState ? 0.5f : 0.0f);
			}
			else {
				setAmber(ATTACH_LIGHT, attached ? 0.5f : 0.0f);
			}
			
			// Reset light
			lights[RESET_LIGHT].setSmoothBrightness(resetLight, args.sampleTime * (RefreshCounter::displayRefreshStepSkips >> 2));
			resetLight = 0.0f;
			
			// Run light (amber)
			setAmber(RUN_LIGHT, running ? 1.0f : 0.0f);

			if (editingGate > 0ul)
				editingGate--;
			if (editingType > 0ul)
				editingType--;
			if (infoCopyPaste != 0l) {
				if (infoCopyPaste > 0l)
					infoCopyPaste --;
				if (infoCopyPaste < 0l)
					infoCopyPaste ++;
			}
			if (editingPpqn > 0l)
				editingPpqn--;
			if (tiedWarning > 0l)
				tiedWarning--;
			if (attachedWarning > 0l)
				attachedWarning--;
			if (attachFlashCounter > 0l)
				attachFlashCounter--;
			if (modeHoldDetect.process(params[RUNMODE_PARAM].getValue())) {
				displayState = DISP_NORMAL;
				editingPpqn = editGateLengthTimeTicks;
			}
			if (revertDisplay > 0l) {
				if (revertDisplay == 1)
					displayState = DISP_NORMAL;
				revertDisplay--;
			}
		}// lightRefreshCounter
				
		if (clockIgnoreOnReset > 0l)
			clockIgnoreOnReset--;
	}// process()
	

	inline void setAmber(int id, float amber) {
		lights[id + 0].setBrightness(amber * 0.45f);// green channel
		lights[id + 1].setBrightness(amber);// red channel
	}

	inline void propagateCVtoTied(int seqn, int track, int stepn) {
		for (int i = stepn + 1; i < 16; i++) {
			if (!attributes[seqn][track][i].getTied())
				break;
			cv[seqn][track][i] = cv[seqn][track][i - 1];
		}
	}

	void activateTiedStep(int seqn, int track, int stepn) {
		attributes[seqn][track][stepn].setTied(true);
		if (stepn > 0) {
			propagateCVtoTied(seqn, track, stepn - 1);

			if (holdTiedNotes) {// new method
				attributes[seqn][track][stepn].setGate(true);
				for (int i = stepn; i < 16 && attributes[seqn][track][i].getTied(); i++) {
					attributes[seqn][track][i].setGateMode(attributes[seqn][track][i - 1].getGateMode());
					attributes[seqn][track][i - 1].setGateMode(5);
					attributes[seqn][track][i - 1].setGate(true);
				}
			}
			else {// old method
				attributes[seqn][track][stepn] = attributes[seqn][track][stepn - 1];
				attributes[seqn][track][stepn].setTied(true);
			}
		}
	}

	void deactivateTiedStep(int seqn, int track, int stepn) {
		attributes[seqn][track][stepn].setTied(false);
		if (holdTiedNotes && stepn != 0) {// new method
			int lastGateType = attributes[seqn][track][stepn].getGateMode();
			for (int i = stepn + 1; i < 16 && attributes[seqn][track][i].getTied(); i++)
				lastGateType = attributes[seqn][track][i].getGateMode();
			attributes[seqn][track][stepn - 1].setGateMode(lastGateType);
		}
		//else old method, nothing to do
	}
	
	inline void setGateLight(bool gateOn, int lightIndex) {
		lights[lightIndex].setBrightness(gateOn ? 0.5f : 0.0f);
	}

};


struct ConditionParamQuantity : ParamQuantity {
	std::string getDisplayValueString() override {
		TwinSlide* module = reinterpret_cast<TwinSlide*>(this->module);
		if (!module) return "";
		int cond = module->getEditStepCondition();
		static const std::string condNames[5] = {"1:1", "1:2", "1:4", "1st", "!1st"};
		return condNames[cond];
	}
};

struct StepPhraseParamQuantity : ParamQuantity {
	int buttonIndex = 0;// 0-31

	std::string getLabel() override {
		TwinSlide* module = reinterpret_cast<TwinSlide*>(this->module);
		if (!module) return name;

		if (module->isSeqMode()) {
			// SEQ mode: "Track A - Step x" or "Track B - Step x"
			if (buttonIndex < 16)
				return string::f("Track A - Step %d", buttonIndex + 1);
			else
				return string::f("Track B - Step %d", (buttonIndex - 16) + 1);
		}
		else {
			// SONG mode: "Phrase x - y" where x=slot number, y=sequence number
			int slotNum = buttonIndex + 1;
			int seqNum = module->phrase[buttonIndex] + 1;// phrase[] is 0-indexed, display as 1-indexed
			return string::f("Phrase %d - %d", slotNum, seqNum);
		}
	}

	std::string getDisplayValueString() override {
		return "";// No value display for buttons
	}
};

struct DataEncoderParamQuantity : ParamQuantity {
	std::string getLabel() override {
		TwinSlide* module = reinterpret_cast<TwinSlide*>(this->module);
		if (!module) return "Data Encoder";
		bool seqMode = module->isSeqMode();
		if (module->editingPpqn != 0ul)
			return "Clock Mult";
		if (module->displayState == TwinSlide::DISP_MODE)
			return seqMode ? "Run Mode" : "Song Mode";
		if (module->displayState == TwinSlide::DISP_CLOCKDIV)
			return "Clock Div";
		if (module->displayState == TwinSlide::DISP_LENGTH)
			return seqMode ? "Track Length" : "Phrases";
		if (module->displayState == TwinSlide::DISP_TRANSPOSE)
			return "Transpose";
		if (module->displayState == TwinSlide::DISP_ROTATE)
			return "Rotate";
		return seqMode ? "Sequence" : "Phrase";
	}
	std::string getDisplayValueString() override {
		TwinSlide* module = reinterpret_cast<TwinSlide*>(this->module);
		if (!module) return "1";
		bool seqMode = module->isSeqMode();
		int track = module->getEditTrack();
		char buf[16];
		if (module->editingPpqn != 0ul) {
			snprintf(buf, 16, "x%u", (unsigned)module->pulsesPerStep);
			return buf;
		}
		if (module->displayState == TwinSlide::DISP_MODE) {
			if (seqMode)
				return modeLabels[module->trackRunMode[module->seqIndexEdit][track]];
			return modeLabels[module->runModeSong];
		}
		if (module->displayState == TwinSlide::DISP_CLOCKDIV) {
			if (seqMode) {
				snprintf(buf, 16, "/%d", module->trackClockDiv[module->seqIndexEdit][track]);
				return buf;
			}
			return "-";
		}
		if (module->displayState == TwinSlide::DISP_LENGTH) {
			if (seqMode) {
				snprintf(buf, 16, "%u", (unsigned)module->trackLength[module->seqIndexEdit][track]);
				return buf;
			}
			snprintf(buf, 16, "%u", (unsigned)module->phrases);
			return buf;
		}
		if (module->displayState == TwinSlide::DISP_TRANSPOSE) {
			snprintf(buf, 16, "%+d", module->trackTranspose[module->seqIndexEdit][track]);
			return buf;
		}
		if (module->displayState == TwinSlide::DISP_ROTATE) {
			snprintf(buf, 16, "%+d", module->trackRotate[module->seqIndexEdit][track]);
			return buf;
		}
		// DISP_NORMAL
		if (seqMode)
			snprintf(buf, 16, "%u", (unsigned)(module->seqIndexEdit + 1));
		else
			snprintf(buf, 16, "%u", (unsigned)(module->phrase[module->phraseIndexEdit] + 1));
		return buf;
	}
};


struct TwinSlideWidget : ModuleWidget {
	struct TitleDisplayWidget : TransparentWidget {
		std::shared_ptr<Font> fontBold;

		TitleDisplayWidget() {
			box.size = Vec(162.56f * 3.0f, 20);
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
						nvgText(args.vg, centerX + dx * 0.5f, centerY + dy * 0.5f, "TWIN SLIDE", NULL);
					}
				}
			}
			// White fill
			nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
			nvgText(args.vg, centerX, centerY, "TWIN SLIDE", NULL);
			nvgTextLetterSpacing(args.vg, 0.f);

			Widget::drawLayer(args, layer);
		}
	};

	struct SequenceDisplayWidget : TransparentWidget {
		TwinSlide *module = nullptr;
		std::shared_ptr<Font> font;
		std::string fontPath;
		char displayStr[16] = {};
		int lastNum = -1;// -1 means timedout; >= 0 means we have a first number potential, if ever second key comes fast enough
		clock_t lastTime = 0;
		
		SequenceDisplayWidget() {
			fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
		}


		void onHoverKey(const event::HoverKey& e) override {
			if (e.action == GLFW_PRESS) {
				int num1 = -1;
				clock_t currentTime = clock();
				float realTimeDelta = ((float)(currentTime - lastTime))/CLOCKS_PER_SEC;
				if (realTimeDelta < 1.0f) {
					num1 = lastNum;
				}
			
				int num = -1;
				if (e.key >= GLFW_KEY_0 && e.key <= GLFW_KEY_9) {
					num = e.key - GLFW_KEY_0;
				}
				else if (e.key >= GLFW_KEY_KP_0 && e.key <= GLFW_KEY_KP_9) {
					num = e.key - GLFW_KEY_KP_0;
				}
				else if (e.key == GLFW_KEY_SPACE) {
					if (module->displayState != TwinSlide::DISP_LENGTH)
						module->displayState = TwinSlide::DISP_NORMAL;
					if ((!module->running || !module->attached) && !module->isSeqMode()) {
						module->phraseIndexEdit = moveIndex(module->phraseIndexEdit, module->phraseIndexEdit + 1, 32);
						if (!module->running)
							module->phraseIndexRun = module->phraseIndexEdit;
					}
				}
				if (num != -1) {
					int totalNum = num;
					if (num1 != -1) {
						totalNum += num1 * 10;
					}

					bool seqMode = module->isSeqMode();
					if (module->infoCopyPaste != 0l) {
					}
					else if (module->editingPpqn != 0ul) {
					}
					else if (module->displayState == TwinSlide::DISP_MODE) {
					}
					else if (module->displayState == TwinSlide::DISP_LENGTH) {
						if (seqMode) {
							int track = module->getEditTrack();
							module->trackLength[module->seqIndexEdit][track] = clamp(totalNum, 1, 16);
						}
						else
							module->phrases = clamp(totalNum, 1, 32);
					}
					else if (module->displayState == TwinSlide::DISP_TRANSPOSE) {
					}
					else if (module->displayState == TwinSlide::DISP_ROTATE) {
					}
					else {// DISP_NORMAL
						totalNum = clamp(totalNum, 1, 32);
						if (seqMode) {
							module->seqIndexEdit = totalNum - 1;
						}
						else {
							if (!module->attached || !module->running)
								module->phrase[module->phraseIndexEdit] = totalNum - 1;
						}
					}
				}				
				
				lastTime = currentTime;
				lastNum = num;
			}
		}

		
		void runModeToStr(int num) {
			if (num >= 0 && num < NUM_MODES)
				snprintf(displayStr, 4, "%s", modeLabels[num].c_str());
		}

		void drawLayer(const DrawArgs &args, int layer) override {
			if (layer == 1) {
				if (!(font = APP->window->loadFont(fontPath))) {
					return;
				}
				nvgFontSize(args.vg, 18);
				nvgFontFaceId(args.vg, font->handle);
				Vec textPos = VecPx(6, 22);
				nvgFillColor(args.vg, nvgTransRGBA(nvgRGB(0xff, 0xc0, 0x50), 23));
				nvgText(args.vg, textPos.x, textPos.y, "~~~", NULL);

				nvgFillColor(args.vg, nvgRGB(0xff, 0xc0, 0x50));		
				if (module == NULL) {
					snprintf(displayStr, 4, "  1");
				}
				else {
					bool seqMode = module->isSeqMode();
					if (module->infoCopyPaste != 0l) {
						if (module->infoCopyPaste > 0l)
							snprintf(displayStr, 4, "CPY");
						else
							snprintf(displayStr, 4, "PST");
					}
					else if (module->editingPpqn != 0ul) {
						snprintf(displayStr, 16, "x%2u", (unsigned) module->pulsesPerStep);
					}
					else if (module->displayState == TwinSlide::DISP_MODE) {
						if (seqMode) {
							int track = module->getEditTrack();
							runModeToStr(module->trackRunMode[module->seqIndexEdit][track]);
						}
						else
							runModeToStr(module->runModeSong);
					}
					else if (module->displayState == TwinSlide::DISP_CLOCKDIV) {
						if (seqMode) {
							int track = module->getEditTrack();
							int div = module->trackClockDiv[module->seqIndexEdit][track];
							snprintf(displayStr, 4, "/%d", div);
						}
						else
							snprintf(displayStr, 4, "DIV");
					}
					else if (module->displayState == TwinSlide::DISP_LENGTH) {
						if (seqMode) {
							int track = module->getEditTrack();
							snprintf(displayStr, 16, "%c%02u", (track == 0 ? 'A' : 'B'), (unsigned) module->trackLength[module->seqIndexEdit][track]);
						}
						else
							snprintf(displayStr, 16, "L%2u", (unsigned) module->phrases);
					}
					else if (module->displayState == TwinSlide::DISP_TRANSPOSE) {
						int track = module->getEditTrack();
						int transVal = module->trackTranspose[module->seqIndexEdit][track];
						snprintf(displayStr, 16, "%+3d", transVal);
					}
					else if (module->displayState == TwinSlide::DISP_ROTATE) {
						int track = module->getEditTrack();
						int rotVal = module->trackRotate[module->seqIndexEdit][track];
						snprintf(displayStr, 16, "%+3d", rotVal);
					}
					else {// DISP_NORMAL
						snprintf(displayStr, 16, " %2u", (unsigned) (seqMode ? 
							module->seqIndexEdit : module->phrase[module->phraseIndexEdit]) + 1 );
					}
				}
				nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
			}
		}
	};		
	
	struct InteropSeqItem : MenuItem {
		struct InteropCopySeqItem : MenuItem {
			TwinSlide *module;
			void onAction(const event::Action &e) override {
				int seqLen;
				IoStep* ioSteps = module->fillIoSteps(&seqLen);
				interopCopySequence(seqLen, ioSteps);
				delete[] ioSteps;
			}
		};
		struct InteropPasteSeqItem : MenuItem {
			TwinSlide *module;
			void onAction(const event::Action &e) override {
				int seqLen;
				IoStep* ioSteps = interopPasteSequence(16, &seqLen);// 16 steps per track
				if (ioSteps != nullptr) {
					module->emptyIoSteps(ioSteps, seqLen);
					delete[] ioSteps;
				}
			}
		};
		TwinSlide *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			InteropCopySeqItem *interopCopySeqItem = createMenuItem<InteropCopySeqItem>(portableSequenceCopyID, "");
			interopCopySeqItem->module = module;
			interopCopySeqItem->disabled = disabled;
			menu->addChild(interopCopySeqItem);		
			
			InteropPasteSeqItem *interopPasteSeqItem = createMenuItem<InteropPasteSeqItem>(portableSequencePasteID, "");
			interopPasteSeqItem->module = module;
			interopPasteSeqItem->disabled = disabled;
			menu->addChild(interopPasteSeqItem);		

			return menu;
		}
	};

	// Expert mode slider for context menu
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
		~ExpertSlider() {
			delete quantity;
		}
	};

	// RNDP-2 slider for context menu (uses int*)
	struct TrackRandomizeQuantity : Quantity {
		int* value;
		int defaultValue;
		int minVal, maxVal;
		std::string label;
		std::function<std::string(int)> formatFunc;
		float smoothValue;  // Tracks smooth drag position

		TrackRandomizeQuantity(int* val, int def, int minV, int maxV,
		                         const std::string& lbl, std::function<std::string(int)> fmt)
		    : value(val), defaultValue(def), minVal(minV), maxVal(maxV), label(lbl), formatFunc(fmt) {
			smoothValue = (float)*val;
		}

		void setValue(float v) override {
			smoothValue = clamp(v, (float)minVal, (float)maxVal);
			*value = clamp((int)std::round(smoothValue), minVal, maxVal);
		}
		float getValue() override { return smoothValue; }  // Return smooth value for drag
		float getDefaultValue() override { return (float)defaultValue; }
		float getMinValue() override { return (float)minVal; }
		float getMaxValue() override { return (float)maxVal; }
		std::string getLabel() override { return label; }
		std::string getDisplayValueString() override { return formatFunc(*value); }  // Display rounded
	};

	struct TrackRandomizeSlider : ui::Slider {
		TrackRandomizeSlider(int* val, int def, int minV, int maxV,
		                       const std::string& lbl, std::function<std::string(int)> fmt) {
			quantity = new TrackRandomizeQuantity(val, def, minV, maxV, lbl, fmt);
			box.size.x = 200.0f;
		}
		~TrackRandomizeSlider() {
			delete quantity;
		}
	};

	void appendContextMenu(Menu *menu) override {
		TwinSlide *module = static_cast<TwinSlide*>(this->module);
		assert(module);

		// Randomize Track submenu (at top)
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Randomize Track", "", [=](Menu* menu) {
			// Generate button
			menu->addChild(createMenuItem("Generate", "", [=]() {
				module->randomizeTrack();
			}));

			menu->addChild(new MenuSeparator());

			// Scale selection submenu (24 scales + Random)
			static const char* scaleNames[] = {
				"Major", "Minor", "Dorian", "Mixolydian", "Lydian", "Phrygian", "Locrian",
				"Harmonic Minor", "Harmonic Major", "Dorian #4", "Phrygian Dominant",
				"Melodic Minor", "Lydian Augmented", "Lydian Dominant", "Hungarian Minor",
				"Super Locrian", "Spanish", "Bhairav", "Pentatonic Minor", "Pentatonic Major",
				"Blues Minor", "Whole Tone", "Chromatic", "Japanese In-Sen"
			};
			auto getScaleName = [&]() -> std::string {
				if (module->rndpScaleIndex < 0) return "Random";
				if (module->rndpScaleIndex < 24) return scaleNames[module->rndpScaleIndex];
				return "Random";
			};
			menu->addChild(createSubmenuItem("Scale", getScaleName(), [=](Menu* menu) {
				// Top level: Random, Chromatic, Whole Tone
				menu->addChild(createCheckMenuItem("Random", "",
					[=]() { return module->rndpScaleIndex == -1; },
					[=]() { module->rndpScaleIndex = -1; }
				));
				menu->addChild(createCheckMenuItem("Chromatic", "",
					[=]() { return module->rndpScaleIndex == 22; },
					[=]() { module->rndpScaleIndex = 22; }
				));
				menu->addChild(createCheckMenuItem("Whole Tone", "",
					[=]() { return module->rndpScaleIndex == 21; },
					[=]() { module->rndpScaleIndex = 21; }
				));

				menu->addChild(new MenuSeparator());

				// Modes submenu
				menu->addChild(createSubmenuItem("Modes", "", [=](Menu* menu) {
					static const std::pair<const char*, int> modes[] = {
						{"Major", 0}, {"Minor", 1}, {"Dorian", 2}, {"Mixolydian", 3},
						{"Lydian", 4}, {"Phrygian", 5}, {"Locrian", 6}
					};
					for (auto& m : modes) {
						menu->addChild(createCheckMenuItem(m.first, "",
							[=]() { return module->rndpScaleIndex == m.second; },
							[=]() { module->rndpScaleIndex = m.second; }
						));
					}
				}));

				// Harmonic submenu
				menu->addChild(createSubmenuItem("Harmonic", "", [=](Menu* menu) {
					static const std::pair<const char*, int> scales[] = {
						{"Harmonic Minor", 7}, {"Harmonic Major", 8}, {"Phrygian Dominant", 10}
					};
					for (auto& s : scales) {
						menu->addChild(createCheckMenuItem(s.first, "",
							[=]() { return module->rndpScaleIndex == s.second; },
							[=]() { module->rndpScaleIndex = s.second; }
						));
					}
				}));

				// Melodic submenu
				menu->addChild(createSubmenuItem("Melodic", "", [=](Menu* menu) {
					static const std::pair<const char*, int> scales[] = {
						{"Melodic Minor", 11}, {"Dorian #4", 9}, {"Lydian Augmented", 12},
						{"Lydian Dominant", 13}, {"Super Locrian", 15}
					};
					for (auto& s : scales) {
						menu->addChild(createCheckMenuItem(s.first, "",
							[=]() { return module->rndpScaleIndex == s.second; },
							[=]() { module->rndpScaleIndex = s.second; }
						));
					}
				}));

				// Pentatonic submenu
				menu->addChild(createSubmenuItem("Pentatonic", "", [=](Menu* menu) {
					static const std::pair<const char*, int> scales[] = {
						{"Pentatonic Major", 19}, {"Pentatonic Minor", 18},
						{"Blues Minor", 20}, {"Japanese In-Sen", 23}
					};
					for (auto& s : scales) {
						menu->addChild(createCheckMenuItem(s.first, "",
							[=]() { return module->rndpScaleIndex == s.second; },
							[=]() { module->rndpScaleIndex = s.second; }
						));
					}
				}));

				// World submenu
				menu->addChild(createSubmenuItem("World", "", [=](Menu* menu) {
					static const std::pair<const char*, int> scales[] = {
						{"Hungarian Minor", 14}, {"Spanish", 16}, {"Bhairav", 17}
					};
					for (auto& s : scales) {
						menu->addChild(createCheckMenuItem(s.first, "",
							[=]() { return module->rndpScaleIndex == s.second; },
							[=]() { module->rndpScaleIndex = s.second; }
						));
					}
				}));
			}));

			menu->addChild(new MenuSeparator());

			// Density slider (0-100%)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2Density, 75, 0, 100, "Density",
				[](int v) { return string::f("%d%%", v); }
			));

			// Spread slider (0-100%)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2Spread, 50, 0, 100, "Spread",
				[](int v) { return string::f("%d%%", v); }
			));

			// Accent Density slider (0-100%)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2AccentDensity, 25, 0, 100, "Accent Density",
				[](int v) { return string::f("%d%%", v); }
			));

			// Slide Density slider (0-100%)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2SlideDensity, 25, 0, 100, "Slide Density",
				[](int v) { return string::f("%d%%", v); }
			));

			// Transpose slider (-12 to +12)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2Transpose, 0, -12, 12, "Transpose",
				[](int v) { return string::f("%+d st", v); }
			));

			// Seed slider (0-999)
			menu->addChild(new TrackRandomizeSlider(
				&module->rndp2Seed, 303, 0, 999, "Seed",
				[](int v) { return string::f("%d", v); }
			));

			menu->addChild(new MenuSeparator());

			// Pin Seed checkbox
			menu->addChild(createBoolPtrMenuItem("Pin Seed", "", &module->rndp2PinSeed));

			// Start with Note checkbox
			menu->addChild(createBoolPtrMenuItem("Start with Note", "", &module->rndp2StartWithNote));

			// Start with Accent checkbox
			menu->addChild(createBoolPtrMenuItem("Start with Accent", "", &module->rndp2StartWithAccent));
		}));

		// Synth settings
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Synth settings"));

		bool is303A = (module->expertA.vcfRange == 0.0f && module->expertA.decayRange == 0.0f &&
		               module->expertA.sustain == 0.0f && module->expertA.release == 0.0f &&
		               module->expertA.bassComp == 1.0f);
		menu->addChild(createSubmenuItem("Expert Mode A", is303A ? "303" : "", [=](Menu* menu) {
			// VCF Range slider (-1 = bass, 0 = original 303)
			menu->addChild(new ExpertSlider(
				&module->expertA.vcfRange, 0.0f, -1.0f, 0.0f, "VCF Range",
				[](float v) {
					float t = v + 1.0f;  // -1..0 -> 0..1
					float vcfMin = 100.0f * std::pow(314.0f / 100.0f, t);
					float vcfMax = 760.0f * std::pow(2394.0f / 760.0f, t);
					return string::f("%.0f-%.0f Hz", vcfMin, vcfMax);
				}
			));
			// Decay Range slider
			menu->addChild(new ExpertSlider(
				&module->expertA.decayRange, 0.0f, -1.0f, 1.0f, "Decay Speed",
				[](float v) {
					float mult = 4.0f * (1.0f - v * 0.5f);
					return string::f("%.1fx", mult);
				}
			));
			// Sustain slider
			menu->addChild(new ExpertSlider(
				&module->expertA.sustain, 0.0f, 0.0f, 1.0f, "Sustain",
				[](float v) { return string::f("%.0f%%", v * 100.0f); }
			));
			// Release slider
			menu->addChild(new ExpertSlider(
				&module->expertA.release, 0.0f, 0.0f, 1.0f, "Release",
				[](float v) { return string::f("%.0f ms", 21.0f + v * 479.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->expertA.blend, 0.0f, 0.0f, 1.0f, "Blend",
				[](float v) { return string::f("%.0f%% Sqr", v * 100.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->expertA.bassComp, 1.0f, 0.0f, 1.0f, "Bass Comp",
				[](float v) { return string::f("%.0f%%", (1.0f - v) * 100.0f); }
			));
		}));

		// Expert Mode B
		bool is303B = (module->expertB.vcfRange == 0.0f && module->expertB.decayRange == 0.0f &&
		               module->expertB.sustain == 0.0f && module->expertB.release == 0.0f &&
		               module->expertB.bassComp == 1.0f);
		menu->addChild(createSubmenuItem("Expert Mode B", is303B ? "303" : "", [=](Menu* menu) {
			// VCF Range slider (-1 = bass, 0 = original 303)
			menu->addChild(new ExpertSlider(
				&module->expertB.vcfRange, 0.0f, -1.0f, 0.0f, "VCF Range",
				[](float v) {
					float t = v + 1.0f;  // -1..0 -> 0..1
					float vcfMin = 100.0f * std::pow(314.0f / 100.0f, t);
					float vcfMax = 760.0f * std::pow(2394.0f / 760.0f, t);
					return string::f("%.0f-%.0f Hz", vcfMin, vcfMax);
				}
			));
			// Decay Range slider
			menu->addChild(new ExpertSlider(
				&module->expertB.decayRange, 0.0f, -1.0f, 1.0f, "Decay Speed",
				[](float v) {
					float mult = 4.0f * (1.0f - v * 0.5f);
					return string::f("%.1fx", mult);
				}
			));
			// Sustain slider
			menu->addChild(new ExpertSlider(
				&module->expertB.sustain, 0.0f, 0.0f, 1.0f, "Sustain",
				[](float v) { return string::f("%.0f%%", v * 100.0f); }
			));
			// Release slider
			menu->addChild(new ExpertSlider(
				&module->expertB.release, 0.0f, 0.0f, 1.0f, "Release",
				[](float v) { return string::f("%.0f ms", 21.0f + v * 479.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->expertB.blend, 0.0f, 0.0f, 1.0f, "Blend",
				[](float v) { return string::f("%.0f%% Sqr", v * 100.0f); }
			));
			menu->addChild(new ExpertSlider(
				&module->expertB.bassComp, 1.0f, 0.0f, 1.0f, "Bass Comp",
				[](float v) { return string::f("%.0f%%", (1.0f - v) * 100.0f); }
			));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Sequencer settings"));

		menu->addChild(createSubmenuItem("Default clock resolution", "", [=](Menu* menu) {
			const int ppsValues[] = {1, 4, 6, 12, 24};
			const char* ppsLabels[] = {"x1", "x4", "x6", "x12", "x24"};
			for (int i = 0; i < 5; i++) {
				menu->addChild(createCheckMenuItem(ppsLabels[i], "",
					[=]() { return module->defaultPulsesPerStep == ppsValues[i]; },
					[=]() { module->defaultPulsesPerStep = ppsValues[i]; }
				));
			}
		}));

		menu->addChild(createBoolPtrMenuItem("Reset on run", "", &module->resetOnRun));

		menu->addChild(createSubmenuItem("Retrigger gates on reset", "", [=](Menu* menu) {
			menu->addChild(createCheckMenuItem("No", "",
				[=]() {return module->retrigGatesOnReset == RGOR_NONE;},
				[=]() {module->retrigGatesOnReset = RGOR_NONE;}
			));
			menu->addChild(createCheckMenuItem("Yes", "",
				[=]() {return module->retrigGatesOnReset == RGOR_YES;},
				[=]() {module->retrigGatesOnReset = RGOR_YES;}
			));
			menu->addChild(createCheckMenuItem("Only when Run cable is unconnected", "",
				[=]() {return module->retrigGatesOnReset == RGOR_NRUN;},
				[=]() {module->retrigGatesOnReset = RGOR_NRUN;}
			));
		}));

		menu->addChild(createBoolPtrMenuItem("Hold tied notes", "", &module->holdTiedNotes));

		menu->addChild(createBoolPtrMenuItem("Single shot song", "", &module->stopAtEndOfSong));

		InteropSeqItem *interopSeqItem = createMenuItem<InteropSeqItem>(portableSequenceID, RIGHT_ARROW);
		interopSeqItem->module = module;
		interopSeqItem->disabled = !module->isSeqMode();
		menu->addChild(interopSeqItem);
	}	
	
	
	struct PushButton80 : CKD6 {
		int* mode = NULL;
		TransformWidget *tw;
		PushButton80() {
			float ratio = 0.8f;
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

	struct DiceButton : widget::SvgWidget {
		TwinSlide* module = NULL;
		std::shared_ptr<window::Svg> svgNormal;
		std::shared_ptr<window::Svg> svgPressed;
		void onHover(const HoverEvent& e) override { e.consume(this); }
		void onButton(const ButtonEvent& e) override {
			if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
				if (e.action == GLFW_PRESS) {
					setSvg(svgPressed);
					if (module) module->randomizeTrack();
				} else if (e.action == GLFW_RELEASE) {
					setSvg(svgNormal);
				}
				e.consume(this);
			}
		}
		void onDragEnd(const DragEndEvent& e) override { setSvg(svgNormal); }
	};

	struct ConditionButton : PushButton80 {
		void appendContextMenu(Menu* menu) override {
			TwinSlide* module = static_cast<TwinSlide*>(getParamQuantity()->module);
			if (!module) return;
			menu->addChild(new MenuSeparator());
			static const std::string condNames[5] = {"1:1", "1:2", "1:4", "1st", "!1st"};
			int currentCond = module->getEditStepCondition();
			int track = module->getEditTrack();
			int step = module->getEditStep();
			for (int i = 0; i < 5; i++) {
				menu->addChild(createCheckMenuItem(condNames[i], "",
					[=]() { return currentCond == i; },
					[=]() { module->attributes[module->seqIndexEdit][track][step].setCond(i); }
				));
			}
		}
	};

	struct DataEncoder : IMBigKnobInf {
		DataEncoder() {
		}
		void onDoubleClick(const event::DoubleClick &e) override {
			ParamQuantity* paramQuantity = getParamQuantity();
			if (paramQuantity) {
				TwinSlide* module = static_cast<TwinSlide*>(paramQuantity->module);
				if (module->editingPpqn != 0) {
					module->pulsesPerStep = module->defaultPulsesPerStep;
					//editingPpqn = editGateLengthTimeTicks;
				}
				else if (module->displayState == TwinSlide::DISP_MODE) {
					if (module->isSeqMode()) {
						int track = module->getEditTrack();
						module->trackRunMode[module->seqIndexEdit][track] = MODE_FWD;
					}
					else {
						module->runModeSong = MODE_FWD;
					}
				}
				else if (module->displayState == TwinSlide::DISP_LENGTH) {
					if (module->isSeqMode()) {
						int track = module->getEditTrack();
						module->trackLength[module->seqIndexEdit][track] = 16;// reset to full length
					}
					else {
						module->phrases = 4;
					}
				}
				else if (module->displayState == TwinSlide::DISP_TRANSPOSE) {
					if (module->isSeqMode()) {
						int track = module->getEditTrack();
						int transVal = module->trackTranspose[module->seqIndexEdit][track];
						if (transVal != 0) {
							// Reverse the accumulated transpose
							float reverseCV = ((float)(-transVal)) / 12.0f;
							for (int s = 0; s < 16; s++)
								module->cv[module->seqIndexEdit][track][s] += reverseCV;
							module->trackTranspose[module->seqIndexEdit][track] = 0;
						}
					}
				}
				else if (module->displayState == TwinSlide::DISP_ROTATE) {
					if (module->isSeqMode()) {
						int track = module->getEditTrack();
						int rotVal = module->trackRotate[module->seqIndexEdit][track];
						int seqLen = module->trackLength[module->seqIndexEdit][track];
						// Rotate data back to original position
						while (rotVal > 0) {
							// Was rotated right, rotate left to undo
							module->rotateSeq(module->seqIndexEdit, false, seqLen, track);
							rotVal--;
						}
						while (rotVal < 0) {
							// Was rotated left, rotate right to undo
							module->rotateSeq(module->seqIndexEdit, true, seqLen, track);
							rotVal++;
						}
						// Reset cursor and counter
						module->stepIndexEdit = module->trackStartIndex(track);
						module->trackRotate[module->seqIndexEdit][track] = 0;
					}
					return;
				}
				else {// DISP_NORMAL
					if (module->isSeqMode()) {
						module->seqIndexEdit = 0;
					}
					else {
						module->phrase[module->phraseIndexEdit] = 0;
					}
				}
			}
			ParamWidget::onDoubleClick(e);
		}
	};

	// Eye widget for attach indicator (closed when detached, open when attached)
	struct AttachEyeWidget : TransparentWidget {
		TwinSlide* module = nullptr;

		AttachEyeWidget() {
			box.size = Vec(mm2px(4.0f), mm2px(3.0f));
		}

		void drawLayer(const DrawArgs& args, int layer) override {
			if (layer != 1) return;

			bool attached = module ? module->attached : false;
			float cx = box.size.x / 2.f;
			float cy = box.size.y / 2.f;
			float eyeW = mm2px(1.5f);
			float eyeH = mm2px(0.8f);
			float lashLen = mm2px(0.6f);

			nvgStrokeWidth(args.vg, mm2px(0.2f));
			nvgLineCap(args.vg, NVG_ROUND);
			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, 204));

			if (attached) {
				// Open eye: ellipse with pupil
				nvgBeginPath(args.vg);
				nvgEllipse(args.vg, cx, cy, eyeW, eyeH);
				nvgStroke(args.vg);

				// Pupil
				nvgBeginPath(args.vg);
				nvgCircle(args.vg, cx, cy, mm2px(0.35f));
				nvgFillColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, 204));
				nvgFill(args.vg);

				// Lashes up (3 lashes)
				for (int i = -1; i <= 1; i++) {
					float lx = cx + i * mm2px(0.8f);
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, lx, cy - eyeH);
					nvgLineTo(args.vg, lx + i * mm2px(0.15f), cy - eyeH - lashLen);
					nvgStroke(args.vg);
				}

				// Lashes down (3 lashes)
				for (int i = -1; i <= 1; i++) {
					float lx = cx + i * mm2px(0.8f);
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, lx, cy + eyeH);
					nvgLineTo(args.vg, lx + i * mm2px(0.15f), cy + eyeH + lashLen);
					nvgStroke(args.vg);
				}
			} else {
				// Closed eye: horizontal line with lashes down
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, cx - eyeW, cy);
				nvgLineTo(args.vg, cx + eyeW, cy);
				nvgStroke(args.vg);

				// Lashes down (3 lashes)
				for (int i = -1; i <= 1; i++) {
					float lx = cx + i * mm2px(0.8f);
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, lx, cy);
					nvgLineTo(args.vg, lx + i * mm2px(0.15f), cy + lashLen);
					nvgStroke(args.vg);
				}
			}

			TransparentWidget::drawLayer(args, layer);
		}
	};

	// Waveform symbols widget (square at top, saw at bottom)
	struct WaveformSymbols : TransparentWidget {
		WaveformSymbols() {
			box.size = Vec(mm2px(2.25f), mm2px(5.85f));
		}

		void drawLayer(const DrawArgs& args, int layer) override {
			if (layer != 1) return;

			float symbolH = mm2px(1.8f);
			float symbolW = mm2px(1.8f);
			float gap = mm2px(2.25f);

			nvgStrokeWidth(args.vg, mm2px(0.3f));
			nvgLineCap(args.vg, NVG_ROUND);
			nvgLineJoin(args.vg, NVG_ROUND);
			nvgStrokeColor(args.vg, nvgRGBA(0xFF, 0xC0, 0x50, 204));

			// Square symbol at top
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, 0, symbolH);
			nvgLineTo(args.vg, 0, 0);
			nvgLineTo(args.vg, symbolW * 0.5f, 0);
			nvgLineTo(args.vg, symbolW * 0.5f, symbolH);
			nvgLineTo(args.vg, symbolW, symbolH);
			nvgLineTo(args.vg, symbolW, 0);
			nvgStroke(args.vg);

			// Saw symbol at bottom
			nvgBeginPath(args.vg);
			float y0 = symbolH + gap;
			nvgMoveTo(args.vg, 0, y0 + symbolH);
			nvgLineTo(args.vg, symbolW * 0.5f, y0);
			nvgLineTo(args.vg, symbolW * 0.5f, y0 + symbolH);
			nvgLineTo(args.vg, symbolW, y0);
			nvgLineTo(args.vg, symbolW, y0 + symbolH);
			nvgStroke(args.vg);

			TransparentWidget::drawLayer(args, layer);
		}
	};


	TwinSlideWidget(TwinSlide *module) {
		setModule(module);
		int* mode = NULL;// theming removed

		// Set custom ParamQuantity for condition button tooltip
		if (module) {
			delete module->paramQuantities[TwinSlide::COND_PARAM];
			module->paramQuantities[TwinSlide::COND_PARAM] = new ConditionParamQuantity();
			module->paramQuantities[TwinSlide::COND_PARAM]->module = module;
			module->paramQuantities[TwinSlide::COND_PARAM]->paramId = TwinSlide::COND_PARAM;
			module->paramQuantities[TwinSlide::COND_PARAM]->name = "Condition";

			delete module->paramQuantities[TwinSlide::ENCODER_PARAM];
			module->paramQuantities[TwinSlide::ENCODER_PARAM] = new DataEncoderParamQuantity();
			module->paramQuantities[TwinSlide::ENCODER_PARAM]->module = module;
			module->paramQuantities[TwinSlide::ENCODER_PARAM]->paramId = TwinSlide::ENCODER_PARAM;
			module->paramQuantities[TwinSlide::ENCODER_PARAM]->minValue = -INFINITY;
			module->paramQuantities[TwinSlide::ENCODER_PARAM]->maxValue = INFINITY;

			// Custom ParamQuantity for step/phrase buttons (dynamic tooltip based on mode)
			for (int i = 0; i < 32; i++) {
				delete module->paramQuantities[TwinSlide::STEP_PHRASE_PARAMS + i];
				StepPhraseParamQuantity* pq = new StepPhraseParamQuantity();
				pq->module = module;
				pq->paramId = TwinSlide::STEP_PHRASE_PARAMS + i;
				pq->buttonIndex = i;
				pq->name = "";// getLabel() handles the name dynamically
				pq->minValue = 0.0f;
				pq->maxValue = 1.0f;
				module->paramQuantities[TwinSlide::STEP_PHRASE_PARAMS + i] = pq;
			}
		}

		// Main panel from Inkscape
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TwinSlide.svg")));
		SvgPanel* svgPanel = static_cast<SvgPanel*>(getPanel());

		// Screws (TC Design: dark screws)
		addChild(createWidget<ScrewBlack>(Vec(15, 0)));
		addChild(createWidget<ScrewBlack>(Vec(15, 365)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x-30, 365)));

		
		
		// ****** Top row - Step sequencer ******

		// Step/Phrase LED buttons (75% size)
		// Row A at Y=43, Row B at Y=63, starting X=36
		float posX = 36.0f;
		for (int x = 0; x < 16; x++) {
			addParam(createParamCentered<LEDButtonSmall>(VecPx(posX, 43), module, TwinSlide::STEP_PHRASE_PARAMS + x));
			addChild(createLightCentered<SmallLightTS<GreenRedWhiteLightTS>>(VecPx(posX, 43), module, TwinSlide::STEP_PHRASE_LIGHTS + (x * 3)));
			addParam(createParamCentered<LEDButtonSmall>(VecPx(posX, 63), module, TwinSlide::STEP_PHRASE_PARAMS + x + 16));
			addChild(createLightCentered<SmallLightTS<GreenRedWhiteLightTS>>(VecPx(posX, 63), module, TwinSlide::STEP_PHRASE_LIGHTS + ((x + 16) * 3)));
			posX += 15.0f;
			if ((x + 1) % 4 == 0)
				posX += 3.67f;
		}
		// Attach button and light
		addParam(createParamCentered<LEDButtonSmall>(VecPx(292, 53), module, TwinSlide::ATTACH_PARAM));
		addChild(createLightCentered<SmallLightTS<GreenRedLightTS>>(VecPx(292, 53), module, TwinSlide::ATTACH_LIGHT));
		// Attach eye indicator (centered between keynote and keygate buttons)
		AttachEyeWidget* attachEye = new AttachEyeWidget();
		attachEye->module = module;
		attachEye->box.pos = VecPx(272 - attachEye->box.size.x / 2.f, 117 - attachEye->box.size.y / 2.f);
		addChild(attachEye);
		// Config switch
		addParam(createDynamicSwitchCentered<TS2switchVInv>(VecPx(349.6, 55), module, TwinSlide::TRACKSELECT_PARAM, mode, svgPanel));



		// ****** Octave and keyboard area ******

		// Octave LED buttons (75% size) at X=36, Y starting at 87 with 15px spacing
		for (int i = 0; i < 5; i++) {
			addParam(createParamCentered<LEDButtonSmall>(VecPx(36, 87 + i * 15), module, TwinSlide::OCTAVE_PARAM + i));
			addChild(createLightCentered<SmallLightTS<GreenRedLightTS>>(VecPx(36, 87 + i * 15), module, TwinSlide::OCTAVE_LIGHTS + i * 2));
		}

		// Keys and Key lights
		Vec keyboardPos = mm2px(Vec(18.222f, 31.303f));
		svgPanel->fb->addChild(new KeyboardSmall(keyboardPos, mode));

		Vec offsetLeds = Vec(PianoKeySmall::sizeX * 0.5f, PianoKeySmall::sizeY * 0.55f);
		for (int k = 0; k < 12; k++) {
			Vec keyPos = keyboardPos + mm2px(smaKeysPos[k]);
			addChild(createPianoKey<PianoKeySmall>(keyPos, k, module ? &module->pkInfo : NULL));
			addChild(createLightCentered<SmallLight<GreenRedWhiteLightTS>>(keyPos + offsetLeds, module, TwinSlide::KEY_LIGHTS + k * 3));
		}

		// Key mode LED buttons at X=272
		addParam(createParamCentered<LEDButtonSmall>(VecPx(272, 102), module, TwinSlide::KEYNOTE_PARAM));
		addChild(createLightCentered<SmallLightTS<WhiteLightTS>>(VecPx(272, 102), module, TwinSlide::KEYNOTE_LIGHT));
		addParam(createParamCentered<LEDButtonSmall>(VecPx(272, 132), module, TwinSlide::KEYGATE_PARAM));
		addChild(createLightCentered<SmallLightTS<WhiteLightTS>>(VecPx(272, 132), module, TwinSlide::KEYGATE_LIGHT));

		// ****** Right side control area ******

		// Title display at top
		TitleDisplayWidget *titleDisplay = new TitleDisplayWidget();
		titleDisplay->box.pos = Vec(0, 0);
		addChild(titleDisplay);

		// Edit mode switch (seq/song)
		addParam(createDynamicSwitchCentered<TS2switchV>(VecPx(434.6, 55), module, TwinSlide::SEQSONG_PARAM, mode, svgPanel));
		// Sequence display
		SequenceDisplayWidget *displaySequence = new SequenceDisplayWidget();
		displaySequence->box.size = VecPx(55, 26);
		displaySequence->box.pos = VecPx(392.1, 55).minus(displaySequence->box.size.div(2));
		displaySequence->module = module;
		addChild(displaySequence);
		svgPanel->fb->addChild(new DisplayBackground(displaySequence->box.pos, displaySequence->box.size, mode));
		// Len/mode button
		addParam(createDynamicParamCentered<PushButton80>(VecPx(434.6, 94), module, TwinSlide::RUNMODE_PARAM, mode));
		// Data encoder
		addParam(createDynamicParamCentered<DataEncoder>(VecPx(392.1, 102), module, TwinSlide::ENCODER_PARAM, mode));
		// Transpose/rotate button
		addParam(createDynamicParamCentered<PushButton80>(VecPx(349.6, 94), module, TwinSlide::TRAN_ROT_PARAM, mode));
		// Reset LED bezel and light
		addParam(createParamCentered<LEDBezel>(VecPx(442, 190), module, TwinSlide::RESET_PARAM));
		addChild(createLightCentered<SmallLight<RedLightTS>>(VecPx(442, 190), module, TwinSlide::RESET_LIGHT));
		// Run LED bezel and light
		addParam(createParamCentered<LEDBezel>(VecPx(341, 190), module, TwinSlide::RUN_PARAM));
		addChild(createLightCentered<SmallLight<GreenRedLightTS>>(VecPx(341, 190), module, TwinSlide::RUN_LIGHT));
		// Copy/paste buttons
		addParam(createParamCentered<LEDButtonSmall>(VecPx(362, 147), module, TwinSlide::COPY_PARAM));
		addParam(createParamCentered<LEDButtonSmall>(VecPx(422, 147), module, TwinSlide::PASTE_PARAM));
		addParam(createParamCentered<LEDButtonSmall>(VecPx(447, 147), module, TwinSlide::CLR_PARAM));
		// Copy-paste mode switch (3 position)
		addParam(createDynamicSwitchCentered<IMSwitch3H>(VecPx(392, 147), module, TwinSlide::CPMODE_PARAM, mode, svgPanel));

		// Control label widget (white text with black outline)
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

		// Clickable label for randomizer exclusion
		struct RndExcludeLabel : OpaqueWidget {
			std::string text;
			TwinSlide* module = NULL;
			int exclIndex = -1;
			RndExcludeLabel(std::string t, int idx) : text(t), exclIndex(idx) {}
			void onButton(const ButtonEvent& e) override {
				if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS && module && exclIndex >= 0) {
					module->rndExclude[exclIndex] = !module->rndExclude[exclIndex];
					e.consume(this);
				}
			}
			void draw(const DrawArgs& args) override {
				// Dot matrix amber background
				nvgFillColor(args.vg, nvgRGBA(0xe6, 0xb0, 0x60, 0x80));
				float dotR = 0.5f;
				float spacing = 2.0f;
				for (float dy = spacing * 0.5f; dy < box.size.y; dy += spacing) {
					for (float dx = spacing * 1.5f; dx < box.size.x - spacing; dx += spacing) {
						nvgBeginPath(args.vg);
						nvgCircle(args.vg, dx, dy, dotR);
						nvgFill(args.vg);
					}
				}

				bool dimmed = module && exclIndex >= 0 && module->rndExclude[exclIndex];
				float cx = box.size.x * 0.5f;
				float cy = box.size.y * 0.5f;
				std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Sono/static/Sono_Proportional-Medium.ttf"));
				if (!font) return;
				nvgFontFaceId(args.vg, font->handle);
				nvgFontSize(args.vg, 10.0f);
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
				nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
				for (int dx = -1; dx <= 1; dx++) {
					for (int dy = -1; dy <= 1; dy++) {
						if (dx != 0 || dy != 0)
							nvgText(args.vg, cx + dx * 0.5f, cy + dy * 0.5f, text.c_str(), NULL);
					}
				}
				nvgFillColor(args.vg, dimmed ? nvgRGB(0xc0, 0xc0, 0xc0) : nvgRGB(0xff, 0xff, 0xff));
				nvgText(args.vg, cx, cy, text.c_str(), NULL);
			}
		};

		ControlLabel* rndLabel = new ControlLabel("RND");
		rndLabel->box.pos = VecPx(337, 160);
		rndLabel->box.size = VecPx(30, 10);
		addChild(rndLabel);

		ControlLabel* copyLabel = new ControlLabel("CPY");
		copyLabel->box.pos = VecPx(362, 160);
		copyLabel->box.size = VecPx(30, 10);
		addChild(copyLabel);

		ControlLabel* pasteLabel = new ControlLabel("PST");
		pasteLabel->box.pos = VecPx(422, 160);
		pasteLabel->box.size = VecPx(35, 10);
		addChild(pasteLabel);

		ControlLabel* clrLabel = new ControlLabel("CLR");
		clrLabel->box.pos = VecPx(447, 160);
		clrLabel->box.size = VecPx(30, 10);
		addChild(clrLabel);

		ControlLabel* cpMode4Label = new ControlLabel("4");
		cpMode4Label->box.pos = VecPx(382, 160);
		cpMode4Label->box.size = VecPx(10, 10);
		addChild(cpMode4Label);

		ControlLabel* cpMode8Label = new ControlLabel("8");
		cpMode8Label->box.pos = VecPx(392, 160);
		cpMode8Label->box.size = VecPx(10, 10);
		addChild(cpMode8Label);

		ControlLabel* cpModeALabel = new ControlLabel("T");
		cpModeALabel->box.pos = VecPx(402, 160);
		cpModeALabel->box.size = VecPx(10, 10);
		addChild(cpModeALabel);

		// Key mode labels (NOTE above keynote button, GATE below keygate button)
		RndExcludeLabel* keyNoteLabel = new RndExcludeLabel("NOTE", TwinSlide::RND_PITCH);
		keyNoteLabel->module = module;
		keyNoteLabel->box.pos = VecPx(257, 80);
		keyNoteLabel->box.size = VecPx(30, 10);
		addChild(keyNoteLabel);

		RndExcludeLabel* keyGateLabel = new RndExcludeLabel("GATE", TwinSlide::RND_GATESHAPE);
		keyGateLabel->module = module;
		keyGateLabel->box.pos = VecPx(257, 144);
		keyGateLabel->box.size = VecPx(30, 10);
		addChild(keyGateLabel);


		// ****** Gate and slide section (lights centered on buttons) ******

		// Track A label
		ControlLabel* trackALabel = new ControlLabel("A");
		trackALabel->box.pos = VecPx(20.5, 43);
		trackALabel->box.size = VecPx(10, 10);
		addChild(trackALabel);

		// Track B label
		ControlLabel* trackBLabel = new ControlLabel("B");
		trackBLabel->box.pos = VecPx(20.5, 63);
		trackBLabel->box.size = VecPx(10, 10);
		addChild(trackBLabel);

		// Track switch labels (A above, B below) - centered on switch X=349.6
		ControlLabel* trackSwitchA = new ControlLabel("A");
		trackSwitchA->box.pos = VecPx(349.6, 37);
		trackSwitchA->box.size = VecPx(10, 10);
		addChild(trackSwitchA);
		ControlLabel* trackSwitchB = new ControlLabel("B");
		trackSwitchB->box.pos = VecPx(349.6, 73);
		trackSwitchB->box.size = VecPx(10, 10);
		addChild(trackSwitchB);

		// Seq/Song switch labels (SEQ above, SONG below) - centered on switch X=434.6
		ControlLabel* seqLabel = new ControlLabel("SEQ");
		seqLabel->box.pos = VecPx(434.6, 37);
		seqLabel->box.size = VecPx(10, 10);
		addChild(seqLabel);
		ControlLabel* sonLabel = new ControlLabel("SONG");
		sonLabel->box.pos = VecPx(434.6, 73);
		sonLabel->box.size = VecPx(10, 10);
		addChild(sonLabel);

		// Transpose/rotate button labels (2 lines below button at Y=94)
		ControlLabel* transposeLabel = new ControlLabel("TRANS");
		transposeLabel->box.pos = VecPx(349.6, 110);
		transposeLabel->box.size = VecPx(10, 10);
		addChild(transposeLabel);
		ControlLabel* rotateLabel = new ControlLabel("ROTATE");
		rotateLabel->box.pos = VecPx(349.6, 120);
		rotateLabel->box.size = VecPx(10, 10);
		addChild(rotateLabel);

		// Length/mode button labels (2 lines below button at Y=94)
		ControlLabel* lengthLabel = new ControlLabel("LENGTH");
		lengthLabel->box.pos = VecPx(434.6, 110);
		lengthLabel->box.size = VecPx(10, 10);
		addChild(lengthLabel);
		ControlLabel* modeLabel = new ControlLabel("MODE");
		modeLabel->box.pos = VecPx(434.6, 120);
		modeLabel->box.size = VecPx(10, 10);
		addChild(modeLabel);

		// Octave labels (2, 1, 0, 1, 2) at Y=87, 102, 117, 132, 147
		ControlLabel* oct2Label = new ControlLabel("2");
		oct2Label->box.pos = VecPx(20.5, 87);
		oct2Label->box.size = VecPx(10, 10);
		addChild(oct2Label);

		ControlLabel* oct1Label = new ControlLabel("1");
		oct1Label->box.pos = VecPx(20.5, 102);
		oct1Label->box.size = VecPx(10, 10);
		addChild(oct1Label);

		ControlLabel* oct0Label = new ControlLabel("0");
		oct0Label->box.pos = VecPx(20.5, 117);
		oct0Label->box.size = VecPx(10, 10);
		addChild(oct0Label);

		ControlLabel* octM1Label = new ControlLabel("1");
		octM1Label->box.pos = VecPx(20.5, 132);
		octM1Label->box.size = VecPx(10, 10);
		addChild(octM1Label);

		ControlLabel* octM2Label = new ControlLabel("2");
		octM2Label->box.pos = VecPx(20.5, 147);
		octM2Label->box.size = VecPx(10, 10);
		addChild(octM2Label);

		// GATE label (aligned with PROB button below at X=30)
		RndExcludeLabel* gateLabel = new RndExcludeLabel("GATE", TwinSlide::RND_GATE);
		gateLabel->module = module;
		gateLabel->box.pos = VecPx(15, 165);
		gateLabel->box.size = VecPx(30, 10);
		addChild(gateLabel);

		// Gate light and button (aligned with PROB button below at X=30)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(30, 190), module, TwinSlide::GATE_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(30, 190), module, TwinSlide::GATE_LIGHT));
		// TIE label (centered above PROB trimpot at X=62)
		RndExcludeLabel* tieLabel = new RndExcludeLabel("TIE", TwinSlide::RND_TIE);
		tieLabel->module = module;
		tieLabel->box.pos = VecPx(47, 165);
		tieLabel->box.size = VecPx(30, 10);
		addChild(tieLabel);

		// Tie light and button (centered above PROB trimpot at X=62)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(62, 190), module, TwinSlide::TIE_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(62, 190), module, TwinSlide::TIE_LIGHT));
		// REL label
		RndExcludeLabel* relLabel = new RndExcludeLabel("REL", TwinSlide::RND_REL);
		relLabel->module = module;
		relLabel->box.pos = VecPx(79, 165);
		relLabel->box.size = VecPx(30, 10);
		addChild(relLabel);
		// REL button and light (per-step release)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(94, 190), module, TwinSlide::REL_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(94, 190), module, TwinSlide::REL_LIGHT));
		// REL knob (per-step release time)
		addParam(createDynamicParamCentered<IMSmallKnob>(VecPx(126, 190), module, TwinSlide::REL_KNOB_PARAM, mode));
		// ACC label (aligned with COND button below at X=158)
		RndExcludeLabel* accLabel = new RndExcludeLabel("ACC", TwinSlide::RND_ACC);
		accLabel->module = module;
		accLabel->box.pos = VecPx(143, 165);
		accLabel->box.size = VecPx(30, 10);
		addChild(accLabel);

		// Accent light and button (aligned with COND button below at X=158)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(158, 190), module, TwinSlide::ACCENT_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(158, 190), module, TwinSlide::ACCENT_LIGHT));

		// CON label (centered in container 3)
		RndExcludeLabel* conLabel = new RndExcludeLabel("CON", TwinSlide::RND_COND);
		conLabel->module = module;
		conLabel->box.pos = VecPx(143, 202);
		conLabel->box.size = VecPx(30, 10);
		addChild(conLabel);

		// Condition light and button (32px from SLIDE knob at X=126)
		addParam(createDynamicParamCentered<ConditionButton>(VecPx(158, 227), module, TwinSlide::COND_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(158, 227), module, TwinSlide::COND_LIGHT));

		// FINE A label
		ControlLabel* fineALabel = new ControlLabel("FINE");
		fineALabel->box.pos = VecPx(247, 170);
		fineALabel->box.size = VecPx(30, 10);
		addChild(fineALabel);

		// Level A label
		ControlLabel* levelALabel = new ControlLabel("LEVEL");
		levelALabel->box.pos = VecPx(279, 170);
		levelALabel->box.size = VecPx(30, 10);
		addChild(levelALabel);

		// Waveform A switch + Fine A knob + Level A (container 4: x=64.25-103mm SVG, centered at X=247)
		ControlLabel* waveALabel = new ControlLabel("A");
		waveALabel->box.pos = VecPx(185, 187);
		waveALabel->box.size = VecPx(15, 10);
		addChild(waveALabel);
		addParam(createDynamicSwitchCentered<TS2switchV>(VecPx(215, 187), module, TwinSlide::WAVEA_PARAM, mode, svgPanel));
		WaveformSymbols* waveSymbolsA = new WaveformSymbols();
		waveSymbolsA->box.pos = VecPx(195, 187 - waveSymbolsA->box.size.y / 2.f);
		addChild(waveSymbolsA);
		addParam(createParamCentered<C1Knob280>(VecPx(247, 190), module, TwinSlide::FINEA_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(279, 190), module, TwinSlide::LEVELA_PARAM));
		LedRingOverlay* fineARing = new LedRingOverlay(module, TwinSlide::FINEA_PARAM);
		fineARing->box.pos = VecPx(247 - 25, 190 - 25);
		addChild(fineARing);
		LedRingOverlay* levelARing = new LedRingOverlay(module, TwinSlide::LEVELA_PARAM);
		levelARing->box.pos = VecPx(279 - 25, 190 - 25);
		addChild(levelARing);

		// FINE B label
		ControlLabel* fineBLabel = new ControlLabel("FINE");
		fineBLabel->box.pos = VecPx(247, 207);
		fineBLabel->box.size = VecPx(30, 10);
		addChild(fineBLabel);

		// Level B label
		ControlLabel* levelBLabel = new ControlLabel("LEVEL");
		levelBLabel->box.pos = VecPx(279, 207);
		levelBLabel->box.size = VecPx(30, 10);
		addChild(levelBLabel);

		// Waveform B switch + Fine B knob + Level B (container 4, centered)
		ControlLabel* waveBLabel = new ControlLabel("B");
		waveBLabel->box.pos = VecPx(185, 224);
		waveBLabel->box.size = VecPx(15, 10);
		addChild(waveBLabel);
		addParam(createDynamicSwitchCentered<TS2switchV>(VecPx(215, 224), module, TwinSlide::WAVEB_PARAM, mode, svgPanel));
		WaveformSymbols* waveSymbolsB = new WaveformSymbols();
		waveSymbolsB->box.pos = VecPx(195, 224 - waveSymbolsB->box.size.y / 2.f);
		addChild(waveSymbolsB);
		addParam(createParamCentered<C1Knob280>(VecPx(247, 227), module, TwinSlide::FINEB_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(279, 227), module, TwinSlide::LEVELB_PARAM));
		LedRingOverlay* fineBRing = new LedRingOverlay(module, TwinSlide::FINEB_PARAM);
		fineBRing->box.pos = VecPx(247 - 25, 227 - 25);
		addChild(fineBRing);
		LedRingOverlay* levelBRing = new LedRingOverlay(module, TwinSlide::LEVELB_PARAM);
		levelBRing->box.pos = VecPx(279 - 25, 227 - 25);
		addChild(levelBRing);

		// ****** Bottom two rows ******

		// PROB label (button+knob pair centered in container 1: button X=30, knob X=62)
		RndExcludeLabel* probLabel = new RndExcludeLabel("PROB", TwinSlide::RND_PROB);
		probLabel->module = module;
		probLabel->box.pos = VecPx(15, 202);
		probLabel->box.size = VecPx(30, 10);
		addChild(probLabel);

		// Probability light and button (left of center to fit knob in container 1)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(30, 227), module, TwinSlide::PROB_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(30, 227), module, TwinSlide::PROB_LIGHT));
		// Probability knob (10px clearance from button outer edge)
		addParam(createDynamicParamCentered<IMSmallKnob>(VecPx(62, 227), module, TwinSlide::PROB_KNOB_PARAM, mode));
		// SLIDE label (button+knob pair centered in container 2: button X=94, knob X=126)
		RndExcludeLabel* slideLabel = new RndExcludeLabel("SLIDE", TwinSlide::RND_SLIDE);
		slideLabel->module = module;
		slideLabel->box.pos = VecPx(79, 202);
		slideLabel->box.size = VecPx(30, 10);
		addChild(slideLabel);

		// Slide light and button (left of center to fit knob in container 2)
		addParam(createDynamicParamCentered<PushButton80>(VecPx(94, 227), module, TwinSlide::SLIDE_BTN_PARAM, mode));
		addChild(createLightCentered<SmallLight<WhiteLightTS>>(VecPx(94, 227), module, TwinSlide::SLIDE_LIGHT));
		// Slide knob (10px clearance from button outer edge)
		addParam(createDynamicParamCentered<IMSmallKnob>(VecPx(126, 227), module, TwinSlide::SLIDE_KNOB_PARAM, mode));
		// Clock input
		addInput(createDynamicPortCentered<JackPort>(VecPx(392, 226), true, module, TwinSlide::CLOCK_INPUT, mode));
		// Dice icon above clock input
		DiceButton* diceIcon = new DiceButton;
		diceIcon->module = module;
		diceIcon->svgNormal = APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/fad-random-1dice.svg"));
		diceIcon->svgPressed = APP->window->loadSvg(asset::plugin(pluginInstance, "res/comp/fad-random-1dice-pressed.svg"));
		diceIcon->setSvg(diceIcon->svgNormal);
		diceIcon->box.pos = VecPx(337 - 8.5f, 147 - 8.5f);
		addChild(diceIcon);

		// ****** Synth control labels ******
		ControlLabel* drvLabel = new ControlLabel("DRV");
		drvLabel->box.pos = VecPx(30, 264);
		drvLabel->box.size = VecPx(30, 10);
		addChild(drvLabel);
		ControlLabel* vcfLabel = new ControlLabel("VCF");
		vcfLabel->box.pos = VecPx(62, 264);
		vcfLabel->box.size = VecPx(30, 10);
		addChild(vcfLabel);
		ControlLabel* resLabel = new ControlLabel("RES");
		resLabel->box.pos = VecPx(126, 264);
		resLabel->box.size = VecPx(30, 10);
		addChild(resLabel);
		ControlLabel* envLabel = new ControlLabel("ENV");
		envLabel->box.pos = VecPx(158, 264);
		envLabel->box.size = VecPx(30, 10);
		addChild(envLabel);
		ControlLabel* decLabel = new ControlLabel("DEC");
		decLabel->box.pos = VecPx(222, 264);
		decLabel->box.size = VecPx(30, 10);
		addChild(decLabel);
		ControlLabel* accKnobLabel = new ControlLabel("ACC");
		accKnobLabel->box.pos = VecPx(286, 264);
		accKnobLabel->box.size = VecPx(30, 10);
		addChild(accKnobLabel);

		// ****** Synth A controls (row at Y=284) ******
		// Order: Drive, VCF, VCF CV, Res, EnvMod, EnvMod CV, Decay, Decay CV, Accent, Level, Audio Out
		// (Wave switch and Fine tune moved to container 4)
		addParam(createParamCentered<C1Knob280>(VecPx(30, 284), module, TwinSlide::DRIVEA_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(62, 284), module, TwinSlide::CUTOFFA_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(94, 284), true, module, TwinSlide::CUTOFFA_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(126, 284), module, TwinSlide::RESA_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(158, 284), module, TwinSlide::ENVMODA_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(190, 284), true, module, TwinSlide::ENVMODA_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(222, 284), module, TwinSlide::DECAYA_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(254, 284), true, module, TwinSlide::DECAYA_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(286, 284), module, TwinSlide::ACCENTA_KNOB_PARAM));
		// LED ring overlays for Synth A knobs
		LedRingOverlay* driveARing = new LedRingOverlay(module, TwinSlide::DRIVEA_PARAM);
		driveARing->box.pos = VecPx(30 - 25, 284 - 25);
		addChild(driveARing);
		LedRingOverlay* vcfARing = new LedRingOverlay(module, TwinSlide::CUTOFFA_PARAM);
		vcfARing->box.pos = VecPx(62 - 25, 284 - 25);
		addChild(vcfARing);
		LedRingOverlay* resARing = new LedRingOverlay(module, TwinSlide::RESA_PARAM);
		resARing->box.pos = VecPx(126 - 25, 284 - 25);
		addChild(resARing);
		LedRingOverlay* envARing = new LedRingOverlay(module, TwinSlide::ENVMODA_PARAM);
		envARing->box.pos = VecPx(158 - 25, 284 - 25);
		addChild(envARing);
		LedRingOverlay* decARing = new LedRingOverlay(module, TwinSlide::DECAYA_PARAM);
		decARing->box.pos = VecPx(222 - 25, 284 - 25);
		addChild(decARing);
		LedRingOverlay* accARing = new LedRingOverlay(module, TwinSlide::ACCENTA_KNOB_PARAM);
		accARing->box.pos = VecPx(286 - 25, 284 - 25);
		addChild(accARing);
		// Row A identifier (above VCF CV jack)
		ControlLabel* rowALabel = new ControlLabel("A");
		rowALabel->box.pos = VecPx(94, 264.5);
		rowALabel->box.size = VecPx(15, 10);
		addChild(rowALabel);
		// Level A moved to container 4
		addOutput(createDynamicPortCentered<JackPort>(VecPx(442, 284), false, module, TwinSlide::AUDIOA_OUTPUT, mode));

		// ****** Synth B control labels ******
		ControlLabel* drvLabelB = new ControlLabel("DRV");
		drvLabelB->box.pos = VecPx(30, 303);
		drvLabelB->box.size = VecPx(30, 10);
		addChild(drvLabelB);
		ControlLabel* vcfLabelB = new ControlLabel("VCF");
		vcfLabelB->box.pos = VecPx(62, 303);
		vcfLabelB->box.size = VecPx(30, 10);
		addChild(vcfLabelB);
		ControlLabel* resLabelB = new ControlLabel("RES");
		resLabelB->box.pos = VecPx(126, 303);
		resLabelB->box.size = VecPx(30, 10);
		addChild(resLabelB);
		ControlLabel* envLabelB = new ControlLabel("ENV");
		envLabelB->box.pos = VecPx(158, 303);
		envLabelB->box.size = VecPx(30, 10);
		addChild(envLabelB);
		ControlLabel* decLabelB = new ControlLabel("DEC");
		decLabelB->box.pos = VecPx(222, 303);
		decLabelB->box.size = VecPx(30, 10);
		addChild(decLabelB);
		ControlLabel* accKnobLabelB = new ControlLabel("ACC");
		accKnobLabelB->box.pos = VecPx(286, 303);
		accKnobLabelB->box.size = VecPx(30, 10);
		addChild(accKnobLabelB);

		// ****** Synth B controls (row at Y=323) ******
		// Order: Drive, VCF, VCF CV, Res, EnvMod, EnvMod CV, Decay, Decay CV, Accent, Level, Audio Out
		// (Wave switch and Fine tune moved to container 4)
		addParam(createParamCentered<C1Knob280>(VecPx(30, 323), module, TwinSlide::DRIVEB_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(62, 323), module, TwinSlide::CUTOFFB_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(94, 323), true, module, TwinSlide::CUTOFFB_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(126, 323), module, TwinSlide::RESB_PARAM));
		addParam(createParamCentered<C1Knob280>(VecPx(158, 323), module, TwinSlide::ENVMODB_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(190, 323), true, module, TwinSlide::ENVMODB_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(222, 323), module, TwinSlide::DECAYB_PARAM));
		addInput(createDynamicPortCentered<JackPort>(VecPx(254, 323), true, module, TwinSlide::DECAYB_INPUT, mode));
		addParam(createParamCentered<C1Knob280>(VecPx(286, 323), module, TwinSlide::ACCENTB_KNOB_PARAM));
		// LED ring overlays for Synth B knobs
		LedRingOverlay* driveBRing = new LedRingOverlay(module, TwinSlide::DRIVEB_PARAM);
		driveBRing->box.pos = VecPx(30 - 25, 323 - 25);
		addChild(driveBRing);
		LedRingOverlay* vcfBRing = new LedRingOverlay(module, TwinSlide::CUTOFFB_PARAM);
		vcfBRing->box.pos = VecPx(62 - 25, 323 - 25);
		addChild(vcfBRing);
		LedRingOverlay* resBRing = new LedRingOverlay(module, TwinSlide::RESB_PARAM);
		resBRing->box.pos = VecPx(126 - 25, 323 - 25);
		addChild(resBRing);
		LedRingOverlay* envBRing = new LedRingOverlay(module, TwinSlide::ENVMODB_PARAM);
		envBRing->box.pos = VecPx(158 - 25, 323 - 25);
		addChild(envBRing);
		LedRingOverlay* decBRing = new LedRingOverlay(module, TwinSlide::DECAYB_PARAM);
		decBRing->box.pos = VecPx(222 - 25, 323 - 25);
		addChild(decBRing);
		LedRingOverlay* accBRing = new LedRingOverlay(module, TwinSlide::ACCENTB_KNOB_PARAM);
		accBRing->box.pos = VecPx(286 - 25, 323 - 25);
		addChild(accBRing);
		// Row B identifier (above VCF CV jack)
		ControlLabel* rowBLabel = new ControlLabel("B");
		rowBLabel->box.pos = VecPx(94, 303.5);
		rowBLabel->box.size = VecPx(15, 10);
		addChild(rowBLabel);
		// Level B moved to container 4
		addOutput(createDynamicPortCentered<JackPort>(VecPx(442, 323), false, module, TwinSlide::AUDIOB_OUTPUT, mode));

		// Audio output labels
		ControlLabel* outLabel = new ControlLabel("OUT");
		outLabel->box.pos = VecPx(442, 268);
		outLabel->box.size = VecPx(30, 10);
		addChild(outLabel);
		// A/B labels to the right of output jacks
		ControlLabel* outALabel = new ControlLabel("A");
		outALabel->box.pos = VecPx(458, 284);
		outALabel->box.size = VecPx(15, 10);
		addChild(outALabel);
		ControlLabel* outBLabel = new ControlLabel("B");
		outBLabel->box.pos = VecPx(458, 323);
		outBLabel->box.size = VecPx(15, 10);
		addChild(outBLabel);

		// CV control Inputs
		addInput(createDynamicPortCentered<JackPort>(VecPx(341, 226), true, module, TwinSlide::RUNCV_INPUT, mode));
		// Reset input
		addInput(createDynamicPortCentered<JackPort>(VecPx(442, 226), true, module, TwinSlide::RESET_INPUT, mode));

		// Transport jack labels
		ControlLabel* runJackLabel = new ControlLabel("RUN");
		runJackLabel->box.pos = VecPx(341, 207);
		runJackLabel->box.size = VecPx(10, 10);
		addChild(runJackLabel);
		ControlLabel* clockJackLabel = new ControlLabel("CLOCK");
		clockJackLabel->box.pos = VecPx(392, 207);
		clockJackLabel->box.size = VecPx(10, 10);
		addChild(clockJackLabel);
		ControlLabel* resetJackLabel = new ControlLabel("RESET");
		resetJackLabel->box.pos = VecPx(442, 207);
		resetJackLabel->box.size = VecPx(10, 10);
		addChild(resetJackLabel);

		// ****** Microtuning section (x=106.6mm, y=87mm, 35mm x 29mm) ******

		// MicroKeyboard widget
		MicroKeyboard* microKeyboard = new MicroKeyboard();
		microKeyboard->box.pos = mm2px(Vec(108.0f, 89.0f));
		if (module) {
			microKeyboard->selectedKey = &module->selectedMicroKey;
		}
		addChild(microKeyboard);

		// Microtuning knobs (Cents, Octave, Master)
		addParam(createDynamicParamCentered<IMSmallKnob>(mm2px(Vec(113.0f, 102.3f)), module, TwinSlide::MICRO_CENTS_PARAM, mode));
		addParam(createDynamicParamCentered<IMSmallKnob>(mm2px(Vec(124.0f, 102.3f)), module, TwinSlide::MICRO_OCTAVE_PARAM, mode));
		addParam(createDynamicParamCentered<IMSmallKnob>(mm2px(Vec(135.0f, 102.3f)), module, TwinSlide::MICRO_MASTER_PARAM, mode));

		// Microtuning labels
		ControlLabel* centsLabel = new ControlLabel("CENTS");
		centsLabel->box.pos = mm2px(Vec(113.0f, 107.3f));
		centsLabel->box.size = VecPx(30, 10);
		addChild(centsLabel);
		ControlLabel* octaveLabel = new ControlLabel("OCTAVE");
		octaveLabel->box.pos = mm2px(Vec(124.0f, 107.3f));
		octaveLabel->box.size = VecPx(30, 10);
		addChild(octaveLabel);
		ControlLabel* masterLabel = new ControlLabel("MASTER");
		masterLabel->box.pos = mm2px(Vec(135.0f, 107.3f));
		masterLabel->box.size = VecPx(40, 10);
		addChild(masterLabel);

		// Temperament selector
		TemperamentSelector* tempSelector = new TemperamentSelector();
		tempSelector->box.pos = mm2px(Vec(108.0f, 110.2f));
		if (module) {
			tempSelector->presetIndex = &module->temperamentPreset;
			tempSelector->microtuningCents = module->microtuningCents;
		}
		addChild(tempSelector);

		// TC Logo at bottom (centered)
		TCLogoWidget* tcLogo = new TCLogoWidget(TCLogoWidget::FULL, module);
		tcLogo->box.pos = Vec(box.size.x / 2.f - 3.f, 355);
		addChild(tcLogo);
	}

};

Model *modelTwinSlide = createModel<TwinSlide, TwinSlideWidget>("TwinSlide");

//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************

#pragma once

#include "TwinSlidePlugin.hpp"
#include "Clipboard.hpp"


// General constants

enum RunModeIds {MODE_FWD, MODE_REV, MODE_PPG, MODE_PEN, MODE_BRN, MODE_RND, MODE_RN2, MODE_FW2, MODE_FW3, MODE_FW4, NUM_MODES};
static const std::string modeLabels[NUM_MODES] = {"FWD","REV","PPG","PEN","BRN","RND","RN2","FW2","FW3","FW4"};
static const int NUM_SEQ_MODES = 7;// sequence modes: FWD through RN2
static const int songModes[] = {MODE_FWD, MODE_FW2, MODE_FW3, MODE_FW4, MODE_BRN, MODE_PEN, MODE_PPG, MODE_RND, MODE_RN2};
static const int NUM_SONG_MODES = 9;

static const int NUM_GATES = 12;// advanced gate types												


//*****************************************************************************


class StepAttributes {
	unsigned short attributes = 0;
	
	public:

	static const unsigned short ATT_MSK_GATE = 0x01;
	static const unsigned short ATT_MSK_GATEP = 0x02;
	static const unsigned short ATT_MSK_ACCENT = 0x04;
	static const unsigned short ATT_MSK_SLIDE = 0x08;
	static const unsigned short ATT_MSK_TIED = 0x10;
	static const unsigned short ATT_MSK_GATEMODE = 0x01E0, gateModeShift = 5;
	static const unsigned short ATT_MSK_COND = 0x0E00, condShift = 9;// 3 bits for condition (0-4)
	static const unsigned short ATT_MSK_RELEASE = 0x1000;// bit 12 - per-step release enable

	static const unsigned short ATT_MSK_INITSTATE = ATT_MSK_GATE;

	inline void clear() {attributes = 0u;}
	inline void init() {attributes = ATT_MSK_INITSTATE;}
	inline void randomize() {attributes = (random::u32() & (ATT_MSK_GATE | ATT_MSK_GATEP | ATT_MSK_ACCENT | ATT_MSK_SLIDE));}

	inline bool getGate() {return (attributes & ATT_MSK_GATE) != 0;}
	inline bool getGateP() {return (attributes & ATT_MSK_GATEP) != 0;}
	inline bool getAccent() {return (attributes & ATT_MSK_ACCENT) != 0;}
	inline bool getSlide() {return (attributes & ATT_MSK_SLIDE) != 0;}
	inline bool getTied() {return (attributes & ATT_MSK_TIED) != 0;}
	inline int getGateMode() {return (attributes & ATT_MSK_GATEMODE) >> gateModeShift;}
	inline int getCond() {return (attributes & ATT_MSK_COND) >> condShift;}
	inline bool getRelease() {return (attributes & ATT_MSK_RELEASE) != 0;}
	inline unsigned short getAttribute() {return attributes;}

	inline void setGate(bool gateState) {attributes &= ~ATT_MSK_GATE; if (gateState) attributes |= ATT_MSK_GATE;}
	inline void setGateP(bool gatePState) {attributes &= ~ATT_MSK_GATEP; if (gatePState) attributes |= ATT_MSK_GATEP;}
	inline void setAccent(bool accentState) {attributes &= ~ATT_MSK_ACCENT; if (accentState) attributes |= ATT_MSK_ACCENT;}
	inline void setSlide(bool slideState) {attributes &= ~ATT_MSK_SLIDE; if (slideState) attributes |= ATT_MSK_SLIDE;}
	inline void setTied(bool tiedState) {
		attributes &= ~ATT_MSK_TIED;
		if (tiedState) {
			attributes |= ATT_MSK_TIED;
			attributes &= ~(ATT_MSK_GATE | ATT_MSK_GATEP | ATT_MSK_SLIDE);// clear other attributes if tied
		}
	}
	inline void setGateMode(int gateMode) {attributes &= ~ATT_MSK_GATEMODE; attributes |= (gateMode << gateModeShift);}
	inline void setCond(int cond) {attributes &= ~ATT_MSK_COND; attributes |= (cond << condShift);}
	inline void setRelease(bool releaseState) {attributes &= ~ATT_MSK_RELEASE; if (releaseState) attributes |= ATT_MSK_RELEASE;}
	inline void cycleCond() {int c = getCond(); setCond((c + 1) % 5);}// cycles 0-4
	inline void setAttribute(unsigned short _attributes) {attributes = _attributes;}

	inline void toggleGate() {attributes ^= ATT_MSK_GATE;}
	inline void toggleGateP() {attributes ^= ATT_MSK_GATEP;}
	inline void toggleAccent() {attributes ^= ATT_MSK_ACCENT;}
	inline void toggleSlide() {attributes ^= ATT_MSK_SLIDE;}
	inline void toggleRelease() {attributes ^= ATT_MSK_RELEASE;}
};// class StepAttributes



//*****************************************************************************


class SeqAttributes {
	unsigned long attributes = 0ul;
	
	public:

	static const unsigned long SEQ_MSK_LENGTH  =   0x000000FF;// number of steps in each sequence, min value is 1
	static const unsigned long SEQ_MSK_RUNMODE =   0x0000FF00, runModeShift = 8;
	static const unsigned long SEQ_MSK_TRANSPOSE = 0x007F0000, transposeShift = 16;
	static const unsigned long SEQ_MSK_TRANSIGN =  0x00800000;// manually implement sign bit
	static const unsigned long SEQ_MSK_ROTATE =    0x7F000000, rotateShift = 24;
	static const unsigned long SEQ_MSK_ROTSIGN =   0x80000000;// manually implement sign bit (+ is right, - is left)
	
	inline void init(int length, int runMode) {attributes = ((length) | (((unsigned long)runMode) << runModeShift));}
	inline void randomize(int maxSteps, int numModes) {attributes = ( (2 + (random::u32() % (maxSteps - 1))) | (((unsigned long)(random::u32() % numModes) << runModeShift)) );}
	
	inline int getLength() {return (int)(attributes & SEQ_MSK_LENGTH);}
	inline int getRunMode() {return (int)((attributes & SEQ_MSK_RUNMODE) >> runModeShift);}
	inline int getTranspose() {
		int ret = (int)((attributes & SEQ_MSK_TRANSPOSE) >> transposeShift);
		if ( (attributes & SEQ_MSK_TRANSIGN) != 0)// if negative
			ret *= -1;
		return ret;
	}
	inline int getRotate() {
		int ret = (int)((attributes & SEQ_MSK_ROTATE) >> rotateShift);
		if ( (attributes & SEQ_MSK_ROTSIGN) != 0)// if negative
			ret *= -1;
		return ret;
	}
	inline unsigned long getSeqAttrib() {return attributes;}
	
	inline void setLength(int length) {attributes &= ~SEQ_MSK_LENGTH; attributes |= ((unsigned long)length);}
	inline void setRunMode(int runMode) {attributes &= ~SEQ_MSK_RUNMODE; attributes |= (((unsigned long)runMode) << runModeShift);}
	inline void setTranspose(int transp) {
		attributes &= ~ (SEQ_MSK_TRANSPOSE | SEQ_MSK_TRANSIGN); 
		attributes |= (((unsigned long)abs(transp)) << transposeShift);
		if (transp < 0) 
			attributes |= SEQ_MSK_TRANSIGN;
	}
	inline void setRotate(int rotn) {
		attributes &= ~ (SEQ_MSK_ROTATE | SEQ_MSK_ROTSIGN); 
		attributes |= (((unsigned long)abs(rotn)) << rotateShift);
		if (rotn < 0) 
			attributes |= SEQ_MSK_ROTSIGN;
	}
	inline void setSeqAttrib(unsigned long _attributes) {attributes = _attributes;}
};// class SeqAttributes


//*****************************************************************************
// RNDP-2 (Acid) Pattern Generation - ported from jjbbllkk/acid-generator
//*****************************************************************************

// Seeded RNG - exact port from src/utils.ts
class Sfc32 {
	uint32_t a, b, c, d;
public:
	Sfc32(uint32_t seed) { a = b = c = d = seed; }

	float next() {
		uint32_t t = (a + b) & 0xFFFFFFFF;
		a = b ^ (b >> 9);
		b = (c + (c << 3)) & 0xFFFFFFFF;
		c = ((c << 21) | (c >> 11)) & 0xFFFFFFFF;
		d = (d + 1) & 0xFFFFFFFF;
		t = (t + d) & 0xFFFFFFFF;
		c = (c + t) & 0xFFFFFFFF;
		return (float)(t) / 4294967296.0f;
	}

	int nextInt(int minVal, int maxVal) {
		return minVal + (int)(next() * (maxVal - minVal + 1));
	}
};

// All 24 scales from src/audio-engine/scales.ts
enum AcidScale {
	ACID_MAJOR, ACID_MINOR, ACID_DORIAN, ACID_MIXOLYDIAN, ACID_LYDIAN, ACID_PHRYGIAN, ACID_LOCRIAN,
	ACID_HARMONIC_MINOR, ACID_HARMONIC_MAJOR, ACID_DORIAN_NR_4, ACID_PHRYGIAN_DOMINANT,
	ACID_MELODIC_MINOR, ACID_LYDIAN_AUGMENTED, ACID_LYDIAN_DOMINANT, ACID_HUNGARIAN_MINOR,
	ACID_SUPER_LOCRIAN, ACID_SPANISH, ACID_BHAIRAV, ACID_PENTATONIC_MINOR, ACID_PENTATONIC_MAJOR,
	ACID_BLUES_MINOR, ACID_WHOLE_TONE, ACID_CHROMATIC, ACID_JAPANESE_IN_SEN,
	NUM_ACID_SCALES  // 24
};

static const std::vector<std::vector<int>> ACID_SCALES = {
	{0, 2, 4, 5, 7, 9, 11},      // MAJOR
	{0, 2, 3, 5, 7, 8, 10},      // MINOR
	{0, 2, 3, 5, 7, 9, 10},      // DORIAN
	{0, 2, 4, 5, 7, 9, 10},      // MIXOLYDIAN
	{0, 2, 4, 6, 7, 9, 11},      // LYDIAN
	{0, 1, 3, 5, 7, 8, 10},      // PHRYGIAN
	{0, 1, 3, 5, 6, 8, 10},      // LOCRIAN
	{0, 2, 3, 5, 7, 8, 11},      // HARMONIC_MINOR
	{0, 2, 4, 5, 7, 8, 11},      // HARMONIC_MAJOR
	{0, 2, 3, 6, 7, 9, 10},      // DORIAN_NR_4
	{0, 1, 4, 5, 7, 8, 10},      // PHRYGIAN_DOMINANT
	{0, 2, 3, 5, 7, 9, 11},      // MELODIC_MINOR
	{0, 2, 4, 6, 8, 9, 11},      // LYDIAN_AUGMENTED
	{0, 2, 4, 6, 7, 9, 10},      // LYDIAN_DOMINANT
	{0, 2, 3, 6, 7, 8, 11},      // HUNGARIAN_MINOR
	{0, 1, 3, 4, 6, 8, 10},      // SUPER_LOCRIAN
	{0, 1, 4, 5, 7, 9, 10},      // SPANISH
	{0, 1, 4, 5, 7, 8, 11},      // BHAIRAV
	{0, 3, 5, 7, 10},            // PENTATONIC_MINOR (5 notes)
	{0, 2, 4, 7, 9},             // PENTATONIC_MAJOR (5 notes)
	{0, 3, 5, 6, 7, 10},         // BLUES_MINOR (6 notes)
	{0, 2, 4, 6, 8, 10},         // WHOLE_TONE (6 notes)
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},  // CHROMATIC (12 notes)
	{0, 1, 5, 7, 10},            // JAPANESE_IN_SEN (5 notes)
};

struct AcidStep {
	float cv;        // V/oct CV value
	bool gate;       // Has note
	bool accent;     // Accent flag
	bool slide;      // Slide flag
};

struct AcidGenParams {
	int density;            // 0-100%, default 100
	int spread;             // 0-100%, default 100
	int accentDensity;      // 0-100%, default 50
	int slideDensity;       // 0-100%, default 50
	uint32_t seed;          // 0-999, default 500
	int transpose;          // -12 to +12 semitones, default 0
	bool startWithNote;     // default true (TwinSlide addition)
	bool startWithAccent;   // default false (TwinSlide addition)
	int forceScaleIndex;    // -1 = random, 0-23 = force specific scale
};

// Function declarations (implemented in SeqUtil.cpp)
int getNoteInScale(int noteIndex, int scaleIndex, int octave);
void generateAcidPattern(const AcidGenParams& params, AcidStep* steps, int* outScaleIndex);


//*****************************************************************************


inline int ppsToIndex(int pulsesPerStep) {// map 1,2,4,6,8,10,12...24, to 0,1,2,3,4,5,6...12
	if (pulsesPerStep == 1) return 0;
	return pulsesPerStep >> 1;
}
inline int indexToPps(int index) {// inverse map of ppsToIndex()
	index = clamp(index, 0, 12);
	if (index == 0) return 1;
	return index <<	1;
}

inline float applyNewOct(float cvVal, int newOct0) {
	float fdel = cvVal - std::floor(cvVal);
	if (fdel > 0.999f) {
		fdel = 0.0f;
	}
	return fdel + (float)newOct0;
}

inline bool calcGate(int gateCode, Trigger clockTrigger, unsigned long clockStep, float sampleRate) {
	if (gateCode < 2) 
		return gateCode == 1;
	if (gateCode == 2)
		return clockTrigger.isHigh();
	return clockStep < (unsigned long) (sampleRate * 0.01f);
}

inline int gateModeToKeyLightIndex(StepAttributes attribute) {
	return attribute.getGateMode();
}



// Other methods (code in SeqUtil.cpp)

int getAdvGate(int ppqnCount, int pulsesPerStep, int gateMode);
bool moveIndexRunMode(int* index, int numSteps, int runMode, unsigned long* history);
int keyIndexToGateMode(int keyIndex, int pulsesPerStep);

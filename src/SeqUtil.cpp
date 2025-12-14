//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************


#include "SeqUtil.hpp"
#include <algorithm>


static const uint32_t advGateHitMask[NUM_GATES] = 
{0x00003F, 0x0F0F0F, 0x000FFF, 0x0F0F00, 0x03FFFF, 0xFFFFFF, 0x00000F, 0x03F03F, 0x000F00, 0x03F000, 0x0F0000, 0};
//	  25%		TRI		  50%		T23		  75%		FUL		  TR1 		DUO		  TR2 	     D2		  TR3  TRIG		


int getAdvGate(int ppqnCount, int pulsesPerStep, int gateMode) { 
	if (gateMode == 11)
		return ppqnCount == 0 ? 3 : 0;
	uint32_t shiftAmt = ppqnCount * (24 / pulsesPerStep);
	return (int)((advGateHitMask[gateMode] >> shiftAmt) & (uint32_t)0x1);
}


bool moveIndexRunMode(int* index, int numSteps, int runMode, unsigned long* history) {// some of this code if from PS32EX)
	int reps = 1;
	// assert((reps * numSteps) <= 0xFFF); // for BRN and RND run modes, history is not a span count but a step count
	
	bool crossBoundary = false;
	
	switch (runMode) {
	
		// history 0x0000 is reserved for reset
		
		case MODE_REV :// reverse; history base is 0x2000
			if ((*history) < 0x2001 || (*history) > 0x2FFF)
				(*history) = 0x2000 + reps;
			(*index)--;
			if ((*index) < 0) {
				(*index) = numSteps - 1;
				(*history)--;
				if ((*history) <= 0x2000)
					crossBoundary = true;
			}
		break;
		
		case MODE_PPG :// forward-reverse; history base is 0x3000
			if ((*history) < 0x3001 || (*history) > 0x3FFF) // even means going forward, odd means going reverse
				(*history) = 0x3000 + reps * 2;
			if (((*history) & 0x1) == 0) {// even so forward phase
				(*index)++;
				if ((*index) >= numSteps) {
					(*index) = numSteps - 1 ;
					(*history)--;
				}
			}
			else {// odd so reverse phase
				(*index)--;
				if ((*index) < 0) {
					(*index) = 0;
					(*history)--;
					if ((*history) <= 0x3000)
						crossBoundary = true;
				}
			}
		break;

		case MODE_PEN :// forward-reverse; history base is 0x4000
			if ((*history) < 0x4001 || (*history) > 0x4FFF) // even means going forward, odd means going reverse
				(*history) = 0x4000 + reps * 2;
			if (((*history) & 0x1) == 0) {// even so forward phase
				(*index)++;
				if ((*index) >= numSteps) {
					(*index) = numSteps - 2;
					(*history)--;
					if ((*index) < 1) {// if back at 0 after turnaround, then no reverse phase needed
						(*index) = 0;
						(*history)--;
						if ((*history) <= 0x4000)
							crossBoundary = true;
					}
				}
			}
			else {// odd so reverse phase
				(*index)--;
				if ((*index) < 1) {
					(*index) = 0;
					(*history)--;
					if ((*history) <= 0x4000)
						crossBoundary = true;
				}
			}
		break;
		
		case MODE_BRN :// brownian random; history base is 0x5000
			if ((*history) < 0x5001 || (*history) > 0x5FFF) 
				(*history) = 0x5000 + numSteps * reps;
			(*index) += (random::u32() % 3) - 1;
			if ((*index) >= numSteps) {
				(*index) = 0;
			}
			if ((*index) < 0) {
				(*index) = numSteps - 1;
			}
			(*history)--;
			if ((*history) <= 0x5000) {
				crossBoundary = true;
			}
		break;
		
		case MODE_RND :// random; history base is 0x6000
		case MODE_RN2 :// random non-repeating (handled in fillStepIndexRunVector)
			if ((*history) < 0x6001 || (*history) > 0x6FFF)
				(*history) = 0x6000 + numSteps * reps;
			(*index) = (random::u32() % numSteps);
			(*history)--;
			if ((*history) <= 0x6000) {
				crossBoundary = true;
			}
		break;
		
		default :// MODE_FWD, MODE_FW2, MODE_FW3, MODE_FW4 forward; history base is 0x1000
			if (runMode == MODE_FW2) reps++;
			else if (runMode == MODE_FW3) reps += 2;
			else if (runMode == MODE_FW4) reps += 3;
			if ((*history) < 0x1001 || (*history) > 0x1FFF)
				(*history) = 0x1000 + reps;
			(*index)++;
			if ((*index) >= numSteps) {
				(*index) = 0;
				(*history)--;
				if ((*history) <= 0x1000)
					crossBoundary = true;
			}
	}

	return crossBoundary;
}


int keyIndexToGateMode(int keyIndex, int pulsesPerStep) {
	int ret = keyIndex;
	
	if (keyIndex == 1 || keyIndex == 3 || keyIndex == 6 || keyIndex == 8 || keyIndex == 10) {// black keys
		if ((pulsesPerStep % 6) != 0)
			ret = -1;
	}
	else if (keyIndex == 4 || keyIndex == 7 || keyIndex == 9) {// 75%, DUO, DU2 
		if ((pulsesPerStep % 4) != 0)
			ret = -1;
	}
	else if (keyIndex == 2) {// 50%
		if ((pulsesPerStep % 2) != 0)
			ret = -1;
	}
	else if (keyIndex == 0) {// 25%
		if (pulsesPerStep != 1 && (pulsesPerStep % 4) != 0)
			ret = -1;
	}
	//else always good: 5 (full) and 11 (trig)

	return ret;
}


//*****************************************************************************
// RNDP-2 (Acid) Pattern Generation - ported from jjbbllkk/acid-generator
//*****************************************************************************

// Maps abstract note index to actual semitone value using scale
// Ported from src/utils.ts getNoteInScale()
int getNoteInScale(int noteIndex, int scaleIndex, int octave) {
	const auto& scale = ACID_SCALES[scaleIndex];
	int len = (int)scale.size();
	int wrappedIndex = noteIndex % len;
	int octaveOffset = noteIndex / len;
	return scale[wrappedIndex] + 12 * (octave + octaveOffset);
}

// Generates 16-step acid pattern
// Ported from src/audio-engine/generator.ts generate()
void generateAcidPattern(const AcidGenParams& params, AcidStep* steps, int* outScaleIndex) {
	Sfc32 rng(params.seed);

	// Select scale (random or forced)
	int scaleIndex;
	if (params.forceScaleIndex >= 0 && params.forceScaleIndex < NUM_ACID_SCALES) {
		scaleIndex = params.forceScaleIndex;
	} else {
		scaleIndex = rng.nextInt(0, NUM_ACID_SCALES - 1);
	}
	if (outScaleIndex) {
		*outScaleIndex = scaleIndex;
	}

	// --- 1. MUSICAL SPREAD LOGIC ---
	// Use fixed 7-element abstract scale [0,1,2,3,4,5,6] for weighted selection
	const int ABSTRACT_SCALE_LEN = 7;
	struct Weighted { int index; float weight; };
	Weighted weightedScale[ABSTRACT_SCALE_LEN];

	for (int i = 0; i < ABSTRACT_SCALE_LEN; i++) {
		weightedScale[i].index = i;
		weightedScale[i].weight = rng.next();
		if (i == 0) weightedScale[i].weight += 999.0f;  // Root always first
		if (i == 4) weightedScale[i].weight += 0.5f;    // Fifth bias
	}

	std::sort(weightedScale, weightedScale + ABSTRACT_SCALE_LEN,
		[](const Weighted& a, const Weighted& b) { return a.weight > b.weight; });

	// Select top N notes based on spread
	int spreadCount = std::max(1, (int)std::round(ABSTRACT_SCALE_LEN * params.spread / 100.0f));
	int selectedNotes[ABSTRACT_SCALE_LEN];
	for (int i = 0; i < spreadCount; i++) {
		selectedNotes[i] = weightedScale[i].index;
	}

	// --- 2. DENSITY MASK (RHYTHM) ---
	const int BAR_LEN = 16;
	Weighted weightedSteps[BAR_LEN];

	for (int i = 0; i < BAR_LEN; i++) {
		weightedSteps[i].index = i;
		weightedSteps[i].weight = rng.next();
		if (i % 4 == 0) weightedSteps[i].weight += 0.5f;  // Downbeat boost
		if (i == 0) weightedSteps[i].weight += 0.5f;      // "One" boost
	}

	std::sort(weightedSteps, weightedSteps + BAR_LEN,
		[](const Weighted& a, const Weighted& b) { return a.weight > b.weight; });

	// Create active steps set
	int numActiveSteps = (int)std::round(BAR_LEN * params.density / 100.0f);
	bool activeSteps[BAR_LEN] = {false};
	for (int i = 0; i < numActiveSteps && i < BAR_LEN; i++) {
		activeSteps[weightedSteps[i].index] = true;
	}

	// --- 3. GENERATE STEP CONTENT ---
	struct StepData {
		int noteIndex;
		int octave;
		float accentProb;
		float slideProb;
	};
	StepData stepData[BAR_LEN];

	for (int i = 0; i < BAR_LEN; i++) {
		bool isDownbeat = (i % 4 == 0);

		// 70% chance of root note on downbeats
		if (isDownbeat && rng.next() > 0.3f) {
			stepData[i].noteIndex = 0;
		} else {
			stepData[i].noteIndex = rng.nextInt(0, spreadCount - 1);
		}

		stepData[i].octave = rng.nextInt(-1, 0);
		stepData[i].accentProb = rng.next();
		stepData[i].slideProb = rng.next();
	}

	// --- 4. APPLY MASKS AND BUILD OUTPUT ---
	for (int i = 0; i < BAR_LEN; i++) {
		if (!activeSteps[i]) {
			steps[i].gate = false;
			steps[i].cv = 0.0f;
			steps[i].accent = false;
			steps[i].slide = false;
		} else {
			steps[i].gate = true;

			// Get the abstract note index and map to actual semitone
			int abstractIndex = selectedNotes[stepData[i].noteIndex];
			int semitone = getNoteInScale(abstractIndex, scaleIndex, stepData[i].octave);

			// Apply transpose and convert to V/oct
			semitone += params.transpose;
			steps[i].cv = semitone / 12.0f;

			steps[i].accent = (i != 0 && stepData[i].accentProb < params.accentDensity / 100.0f);  // Step 0 accent only from startWithAccent
			steps[i].slide = (stepData[i].slideProb < params.slideDensity / 100.0f);
		}
	}

	// --- 5. TWINSLIDE ADDITIONS ---
	// startWithNote: Force step 0 ("the one") to have a note (always root)
	if (params.startWithNote) {
		steps[0].gate = true;
		int semitone = getNoteInScale(0, scaleIndex, 0);  // Root, octave 0
		semitone += params.transpose;
		steps[0].cv = semitone / 12.0f;
		steps[0].accent = false;  // Controlled by startWithAccent
		steps[0].slide = false;
	}

	// startWithAccent: Force step 0 to have accent (only if it has a note)
	if (params.startWithAccent && steps[0].gate) {
		steps[0].accent = true;
	}
}

//***********************************************************************************************
//TwinSlide a plugin by Twisted Cable
//
//303 DSP code ported from Open303 by Robin Schmidt
//Sequencer built on adapted code from "Impromptu Modular PhraseSeq32" by Marc Boulé
//
//See ./LICENSE.md for all licenses
//***********************************************************************************************


#include "TwinSlidePlugin.hpp"


Plugin *pluginInstance;


void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(modelTwinSlide);
	p->addModel(modelTwinSlideExpander);
	p->addModel(modelMonoSlide);
}



// General objects

ClockMaster clockMaster;  




// General functions

static const char noteLettersSharp[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
static const char noteLettersFlat [12] = {'C', 'D', 'D', 'E', 'E', 'F', 'G', 'G', 'A', 'A', 'B', 'B'};
static const char isBlackKey      [12] = { 0,   1,   0,   1,   0,   0,   1,   0,   1,   0,   1,   0 };

void printNoteNoOct(int note, char* text, bool sharp) {// text must be at least 3 chars long (three displayed chars plus end of string)
	// given note is a pitch CV multiplied by 12 and rounded to nearest integer
	note = eucMod(note, 12);
	text[0] = sharp ? noteLettersSharp[note] : noteLettersFlat[note];// note letter
	text[1] = (isBlackKey[note] == 1) ? (sharp ? '\"' : 'b' ) : ' ';// sharp/flat
	text[2] = 0;
}


int printNoteOrig(float cvVal, char* text, bool sharp) {// text must be at least 4 chars long (three displayed chars plus end of string)
	// return cursor position of eos
	
	int indexNote;
	int octave;
	calcNoteAndOct(cvVal, &indexNote, &octave);
	
	// note letter
	text[0] = sharp ? noteLettersSharp[indexNote] : noteLettersFlat[indexNote];
	int cursor = 1;
	
	// octave number
	octave += 4;
	if (octave >= 0 && octave <= 9) {
		text[cursor] = (char) ( 0x30 + octave);
		cursor++;
	}
	
	// sharp/flat
	if (isBlackKey[indexNote] == 1) {
		text[cursor] = (sharp ? '\"' : 'b' );
		cursor++;
	}
	
	text[cursor] = 0;
	return cursor;
}


int printNote(float cvVal, char* text, bool sharp) {// text must be at least 4 chars long (three displayed chars plus end of string)
	// return cursor position of eos
	
	int indexNote;
	int octave;
	calcNoteAndOct(cvVal, &indexNote, &octave);
	
	// note letter
	text[0] = sharp ? noteLettersSharp[indexNote] : noteLettersFlat[indexNote];
	int cursor = 1;
	
	// sharp/flat
	if (isBlackKey[indexNote] == 1) {
		text[cursor] = (sharp ? '\"' : 'b' );
		cursor++;
	}
	
	// octave number
	octave += 4;
	if (octave >= 0 && octave <= 9) {
		text[cursor] = (char) ( 0x30 + octave);
		cursor++;
	}
	
	text[cursor] = 0;
	return cursor;
}


int moveIndex(int index, int indexNext, int numSteps) {
	if (indexNext < 0)
		index = numSteps - 1;
	else
	{
		if (indexNext - index >= 0) { // if moving right or same place
			if (indexNext >= numSteps)
				index = 0;
			else
				index = indexNext;
		}
		else { // moving left 
			if (indexNext >= numSteps)
				index = numSteps - 1;
			else
				index = indexNext;
		}
	}
	return index;
}


void NormalizedFloat12Copy(float* float12) {
	json_t* normFloats12J = json_object();
	json_t *normFloats12ArrayJ = json_array();
	for (int i = 0; i < 12; i++) {
		json_array_insert_new(normFloats12ArrayJ, i, json_real(float12[i]));
	}
	json_object_set_new(normFloats12J, "normalizedFloats", normFloats12ArrayJ);

	json_t* clipboardJ = json_object();		
	json_object_set_new(clipboardJ, "Impromptu normalized float12", normFloats12J);
	char* float12Clip = json_dumps(clipboardJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	json_decref(clipboardJ);
	glfwSetClipboardString(APP->window->win, float12Clip);
	free(float12Clip);
}

void NormalizedFloat12Paste(float* float12) { 
	const char* float12Clip = glfwGetClipboardString(APP->window->win);
	if (!float12Clip) {
		WARN("Normalized float12 error getting clipboard string");
	}
	else {
		json_error_t error;
		json_t* clipboardJ = json_loads(float12Clip, 0, &error);
		if (!clipboardJ) {
			WARN("Normalized float12 error json parsing clipboard");
		}
		else {
			DEFER({json_decref(clipboardJ);});
			json_t* normFloats12J = json_object_get(clipboardJ, "Impromptu normalized float12");
			if (!normFloats12J) {
				WARN("Error no Impromptu normalized float12 values present in clipboard");
			}
			else {
				json_t *normFloats12ArrayJ = json_object_get(normFloats12J, "normalizedFloats");
				if (normFloats12ArrayJ && json_is_array(normFloats12ArrayJ)) {
					for (int i = 0; i < 12; i++) {
						json_t *normFloatsItemJ = json_array_get(normFloats12ArrayJ, i);
						if (normFloatsItemJ) {
							float12[i] = json_number_value(normFloatsItemJ);
						}
					}
				}
			}
		}	
	}	
}
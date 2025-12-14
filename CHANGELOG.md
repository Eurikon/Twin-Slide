# TwinSlide Changelog

## [2.1.0] - 26-12-2024

### Release Candidate 1

First public release preparation with GPL-3.0-or-later license.

**Plugin Metadata:**
- Version: 2.1.0
- License: GPL-3.0-or-later
- Author: Latif Karoumi
- Brand: Twisted Cable

### New Song Modes

Added repeat pattern modes for song playback:
- **FW2**: Forward with 2x repeat per phrase
- **FW3**: Forward with 3x repeat per phrase
- **FW4**: Forward with 4x repeat per phrase
- Song mode now has 9 modes total (FWD, FW2, FW3, FW4, BRN, PEN, PPG, RND, RN2)
- Sequence mode limited to 7 modes (FWD through RN2)
- RN2 in song mode ensures non-repeating random (different phrase each time)

### Expert Mode

Per-synth expert settings accessible via context menu:
- **VCF Range**: Shift filter frequency range toward bass (100-760Hz) or keep default (314-2394Hz)
- **Decay Range**: Adjust decay time multiplier (2.0x to 6.0x, default 4.0x)
- **Sustain**: Hold envelope at target level (0-100%)
- **Release**: Release time from 21ms (original 303) to 500ms

### Encoder Improvements

- Renamed SEQUENCE_PARAM to ENCODER_PARAM
- Custom DataEncoderParamQuantity with dynamic tooltip labels
- Fixed encoder range (was limited to 8 positions due to missing -INFINITY/INFINITY on ParamQuantity)
- Transpose clamped to ±24 semitones (2 octaves)
- Rotate clamped to ±16 steps
- actualDelta check prevents CV/rotation changes when at limits

### Menu State Memory

- lastMenuState remembers last menu position (LENGTH, MODE, or CLOCKDIV)
- Returning to menu restores previous position instead of always starting at LENGTH
- CLOCKDIV not available in song mode (auto-redirects to LENGTH)

### Visual Updates

- Orange button lights for step attributes (GATE, SLIDE, ACCENT, TIE, PROB, KEYGATE)
- Song length now displayed in normal mode (dimmed orange 0.1f on phrase slots)
- Edit cursor timeout reduced to 3 seconds
- Menu display timeout extended to 5 seconds

### Expander Updates

- Extended message format (18 floats)
- Added fine CV inputs per track
- Updated bidirectional communication protocol

### Microtuning

- Complete overhaul of MicroKeyboard and temperament presets system
- Master tuning (420-460Hz)
- Per-key cent offsets

### Cleanup

- Removed SEQCV_INPUT (external sequence selection)
- Removed seqCVmethod setting
- Deleted unused res/C1Knob.svg
- Moved DESIGN.md to docs/_archived/

---

## [0.1.0] - 14-12-2024

### Genesis

TwinSlide is a dual acid bass synthesizer module for VCV Rack, born from the Slide project.

**Lineage:**
- Sequencer architecture adapted from Phrase-Seq-32 (ImpromptuModular by Marc Boulé)
- Synth DSP from Open303 (Robin Schmidt / RS-MET / ROSIC)
- Filter coefficients from mystran (Teemu Voipio) and kunn

**Design Decision:**
Rather than two fully independent sequencer hierarchies, TwinSlide embeds dual tracks INSIDE each sequence. This preserves the proven Phrase-Seq-32 song/phrase structure while adding parallel synth voices.

```
Song → Phrases → Sequences → [Track A + Track B] → Steps
```

### Project Scaffold

- Created plugin structure following Slide's dual-Makefile pattern
- Version 2.6.4 (VCV Rack ABI compatibility)
- Brand: Twisted Cable

### Sequencer Utilities

Adapted from ImpromptuModular (self-contained, no external dependencies):

- `TwinSlideUtil.hpp` - Core utilities (VecPx, RefreshCounter, Trigger, TriggerRiseFall, HoldDetect)
- `PhraseSeqUtil.hpp/.cpp` - Sequencer classes (StepAttributes, SeqAttributes, run modes)
- `comp/PianoKey.hpp/.cpp` - Piano keyboard widget

**Preserved from Phrase-Seq-32:**
- Full StepAttributes (GATE1, GATE1P, GATE2, SLIDE, TIED, gate modes)
- Full SeqAttributes (length, run mode, transpose, rotate)

### Files

```
TwinSlide/
├── src/
│   ├── plugin.cpp
│   ├── plugin.hpp
│   ├── TwinSlide.cpp          # Module shell
│   ├── TwinSlideUtil.hpp      # Core utilities
│   ├── PhraseSeqUtil.hpp      # Sequencer classes
│   ├── PhraseSeqUtil.cpp      # Sequencer functions
│   └── comp/
│       ├── PianoKey.hpp
│       └── PianoKey.cpp
├── res/panels/
│   └── TwinSlide.svg          # Placeholder panel
├── Makefile
├── Makefile.local
├── plugin.json
├── docs/
│   └── DESIGN.md
└── CHANGELOG.md
```

### Panel Debranding

Removed Impromptu Modular branding from TwinSlide.svg panel:
- Removed first musical note symbol (♫)
- Removed "IMPROMPT" text
- Removed "PHRASE-SEQ-32" title
- Removed "SEQ STEP / SONG PHRASE" label
- Kept second musical note for visual balance

### Project Structure

- Moved DESIGN.md to docs/ folder

### Build Status

First successful compile against Rack source (not SDK).

---

## [0.1.1] - 14-12-2024

### Panel Cleanup

- Removed "OCT" (OCTAVE) label from panel
- Removed octave numbers (1, 4, 7) from panel
- Repositioned GATE 1 label to align with moved button (transform translation)

### UI Adjustments

- Moved GATE 1 button to center above GATE 1 probability button (x=34)
- Changed octave selector from 7 octaves to 5 octaves with 0 at center

---

## [0.1.2] - 15-12-2024

### Panel Cleanup

Removed text labels and UI elements from TwinSlide.svg:
- Removed WRITE, HOLD: CLK RES, SEQ, SEQ # labels
- Removed GATE 2 label
- Removed ALL, 8 text elements
- Removed arrow symbols (play/reverse icons)
- Removed decorative rectangles and line elements
- Kept: GATE 1, ♫, *, A, B, CV labels

### UI Repositioning

- Moved Slide button group (button + light + knob) to align with Probability group on X-axis, and SEQ# input jack on Y-axis
- Moved TIE button group to align with Probability knob on X-axis

### Code Cleanup - Gate Naming

Systematic cleanup of Gate1/Gate2 naming convention. Architecture change: single "Gate" per step assigned per track, no Gate2.

**TwinSlide.cpp:**
- Renamed enums: GATE1_PARAM → GATE_PARAM, GATE1_LIGHT → GATE_LIGHT
- Renamed outputs: GATE1A_OUTPUT → GATEA_OUTPUT, GATE1B_OUTPUT → GATEB_OUTPUT
- Renamed probability: GATE1_PROB_* → PROB_*
- Renamed variables: gate1Code → gateCode, gate1Trigger → gateTrigger, etc.
- Renamed function: calcGate1Code → calcGateCode
- Removed all GATE2_* enums, variables, outputs, widgets, and dead code

**PhraseSeqUtil.hpp:**
- Renamed masks: ATT_MSK_GATE1 → ATT_MSK_GATE, ATT_MSK_GATE1P → ATT_MSK_GATEP
- Renamed methods: getGate1() → getGate(), setGate1() → setGate(), etc.
- Removed all Gate2 masks and methods
- Simplified gateModeToKeyLightIndex() function

**PhraseSeqUtil.cpp:**
- Removed calcGate2Code() function

---

## [0.1.2.5] - 15-12-2024

### Phase 2: Dual Track Implementation

Complete implementation of dual-track architecture within each sequence.

**Data Structure Changes:**
- `cv[32][2][16]` - Dual track CV storage (32 sequences × 2 tracks × 16 steps)
- `attributes[32][2][16]` - Dual track step attributes
- `trackLength[32][2]` - Independent length per track per sequence
- `trackRunMode[32][2]` - Independent run mode per track
- `trackTranspose[32][2]` - Independent transpose per track
- `trackRotate[32][2]` - Independent rotate per track
- `stepIndexRun[2]` - Separate run position per track

**Track Selector:**
- Replaced CONFIG_PARAM with TRACKSELECT_PARAM
- 3-position switch: Track A / Both / Track B
- Controls which track(s) are edited and displayed

**Copy/Paste Fix:**
- `startCP` now follows `stepIndexEdit` directly
- Track derived from step position (0-15 = Track A, 16-31 = Track B)
- Fixed cross-track copy behavior

**Step Index Encoding:**
- `stepIndexEdit` range: 0-31 (0-15 = Track A, 16-31 = Track B)
- Track extraction: `stepIndexEdit / 16`
- Step extraction: `stepIndexEdit % 16`

**Per-Track Independence:**
- Each track runs independently with its own length, direction, and position
- Tracks can have different lengths within same sequence
- Tracks can run in different directions (FWD/REV/PPG/etc.)
- Tracks advance independently based on their own run modes

**JSON Serialization:**
- Updated to save/load all dual-track data
- Backward compatibility maintained for single-track presets

---

## [0.1.3] - 16-12-2024

### GUI Styling

**Title:**
- Added "TWIN SLIDE" title at top using Sono-Bold font
- Black outline with white fill text effect (PhysModDrums style)

**Jacks:**
- Switched all jack ports from light to dark version (PJ301M-dark.svg default)

**Components:**
- Added IMSwitch3H horizontal 3-position switch for copy-paste mode
- Added TC Logo at bottom center (shared component from ChopChop)

**Fonts:**
- Added Sono font family (SIL Open Font License)

### Code Cleanup

**Theming Removed:**
- Deleted PanelTheme.cpp and PanelTheme.hpp
- Removed dynamic theming infrastructure
- Simplified Components.hpp (removed DynamicSVGScrew, IMScrew)

**Panel:**
- Simplified TwinSlide.svg panel

---

## [0.1.4] - 16-12-2024

### Edit Cursor Timeout

- Edit cursor (white) now has 5-second visibility timeout
- Cursor disappears after 5 seconds of no step clicks
- Step reverts to showing its actual state (gate indicator) when cursor times out

### Song Mode Visual Feedback

- Current phrase slot now pulses white to 4/4 measure (4 pulses per 16 steps)
- Full brightness on beats (steps 0, 4, 8, 12), dim between beats
- Phrase pulse uses white channel, step position indicator uses orange

### Run/Reset Button Lights

- Changed from large LEDBezelLight to smaller SmallLight
- Run light: Orange (OrangeLightIM)
- Reset light: Red (RedLightIM)

---

## [0.1.5] - 16-12-2024

### TwinSlide Expander Module

Added 8HP CV/Gate I/O expander for TwinSlide.

**Features:**
- Track A: CV, Gate, Slide inputs and outputs
- Track B: CV, Gate, Slide inputs and outputs
- Link light indicator (amber when connected to mother)
- Bidirectional communication with TwinSlide via rightExpander

**Panel Design:**
- Dark theme matching TwinSlide
- "TS ·X·" title with connection-aware coloring
- TC Logo at bottom (FULL style)
- Two equal sections: INPUT and OUTPUT

**Files Added:**
- `src/TwinSlideExpander.cpp` - Expander module
- `res/panels/TwinSlideExpander.svg` - Panel graphic

**Code Changes:**
- Registered `modelTwinSlideExpander` in plugin.cpp
- Added expander to plugin.json modules array
- Removed legacy `modelPhraseSeqExpander` reference
- Removed `InstantiateExpanderItem` (not needed)

---

## [0.2.0] - 16-12-2024

### Phase 4: Dual Synth Voices

Complete implementation of dual 303-style acid synth voices.

**DSP Components** (from Slide/Open303, copied verbatim):
- `OnePoleFilter303` - Feedback HPF helper
- `TeeBeeFilter303` - Diode ladder filter (main filter)
- `SlideOscillator` - PolyBLEP saw/square oscillator
- `AnalogEnvelope303` - RC-style VCA envelope
- `LeakyIntegrator303` - Pitch slew for slide/portamento
- `SimpleHighpass303` - Pre/post filter highpass
- `EnvModScaler303` - Envelope to cutoff mapping
- `DecayEnvelope303` - Accent decay envelope
- `SynthVoice` - Wrapper struct for complete voice

**Per-Voice Synth Parameters:**
| Parameter | Range | Description |
|-----------|-------|-------------|
| Cutoff | 0-1 | Filter cutoff frequency |
| Resonance | 0-1 | Filter resonance |
| Env Mod | 0-1 | Envelope modulation depth |
| Decay | 0-1 | Envelope decay time |
| Accent | 0-1 | Accent intensity |
| Drive | 0-1 | Filter drive/saturation |
| Fine Tune | ±700 cents | Pitch fine tuning |
| Waveform | Saw/Square | Oscillator waveform |
| Cutoff CV | Input | External cutoff modulation |

**Outputs Added:**
- `AUDIOA_OUTPUT` - Synth A audio out
- `AUDIOB_OUTPUT` - Synth B audio out

**Sequencer Integration:**
- Added `ATT_MSK_ACCENT` step attribute (bit 0x04)
- `getAccent()`, `setAccent()`, `toggleAccent()` methods
- Accent button triggers accent toggle on current step
- Accent/Slide flags from sequencer drive respective synth voice

**Files Added:**
- `src/SynthDSP.hpp` - All DSP structs + SynthVoice wrapper

**Code Changes:**
- Synth voice instances in TwinSlide struct
- `onSampleRateChange()` updates voice sample rates
- Synth processing in `process()` after sequencer logic
- Accent attribute wired to ACCENT_PARAM button

**Build Status:**
- Compiles and runs successfully
- Both voices produce audio
- All synth parameters functional

---

## [0.2.1] - 18-12-2024

### File Reorganization

Renamed files to better reflect TwinSlide identity (removing Impromptu heritage names):

| Old Name | New Name |
|----------|----------|
| `Interop.cpp/hpp` | `Clipboard.cpp/hpp` |
| `PhraseSeqUtil.cpp/hpp` | `SeqUtil.cpp/hpp` |
| `ImpromptuModular.cpp/hpp` | `TwinSlidePlugin.cpp/hpp` |

### New Synth Parameters

- **Level A/B** (`LEVELA_PARAM`, `LEVELB_PARAM`): Output level control per voice (0-100%)
- **Env Mod CV** (`ENVMODA_INPUT`, `ENVMODB_INPUT`): External CV modulation for envelope mod depth
- **Decay CV** (`DECAYA_INPUT`, `DECAYB_INPUT`): External CV modulation for decay time

CV inputs add 0.1V per unit to knob values, clamped to 0-1 range.

### Dead Code Removal

Removed vestigial `SeqAttributes sequences[32]` system (migration from Phrase-Seq-32 to per-track arrays):

- `SeqAttributes sequences[32]` declaration
- `SeqAttributes seqAttribCPbuffer` copy/paste buffer
- `SeqAttributes seqAttribBuffer[32]` JSON load buffer
- All related init, randomize, JSON save/load, and process sync code (~100+ lines)
- Legacy JSON migration code for old preset formats

### Cross-Paste Fix

Fixed crossed paste operations to use per-track arrays:
- Now correctly resets `trackTranspose[seqIndexEdit][track]` instead of dead `sequences[].setTranspose()`
- Now correctly resets `trackRotate[seqIndexEdit][track]` instead of dead `sequences[].setRotate()`

### UI Overhaul

**Dynamic Labels** - Added `ControlLabel` widget system for all UI labels:
- Track labels (A/B) for step rows and track switch
- Octave labels (+2, +1, 0, -1, -2)
- Button labels (GATE, TIE, ACC, CON, PROB, SLIDE)
- Synth control labels (DRV, VCF, RES, ENV, DEC, ACC, FINE, LEVEL, OUT)
- Transport labels (RUN, CLOCK, RESET)
- Copy/paste labels (COPY, PASTE, 4, 8, A)
- Seq/Song switch labels

**Display Modes** - Track-aware display:
- DISP_LENGTH: Shows length only for selected track
- DISP_MODE: Selected track row pulses at ~2Hz (using new `flashCounter`)
- DISP_TRANSPOSE: Shows only selected track (display simplified to `+3` instead of `A+3`)
- DISP_ROTATE: Shows only selected track

**Layout Adjustments:**
- Step rows moved up (Y=43, Y=63 instead of Y=46, Y=66)
- Octave buttons moved up (starting Y=87)
- Keyboard moved up 2mm
- Buttons/controls repositioned throughout
- Title display moved to Y=0
- Sequence display height reduced (26px from 30px)
- New `SequenceKnob` component replacing `Davies1900hBlackKnob`

### Behavior Changes

- **Default running state**: Changed from `true` to `false` (sequencer starts stopped)
- **Right-click DISP_TRANSPOSE**: Now reverses accumulated transpose and resets to 0
- **Right-click DISP_ROTATE**: Now rotates data back to original position and resets cursor/counter
- **CPMODE_PARAM label**: Changed from "Copy-paste mode" to "COPY" with labels "4", "8", "All"
- **SEQSONG_PARAM**: Renamed from `EDIT_PARAM`

### Component Updates

**Switch styling:**
- `IMSwitch2VInv`: Now inherits from `app::SvgSwitch` (was `CKSS`), adds explicit frame SVGs
- `IMSwitch2V`: No longer hides shadow
- Switch outline gradient: Changed from grey to TC amber at 70% opacity
  - Top: `nvgRGBA(138, 96, 32, 179)` (dark amber)
  - Bottom: `nvgRGBA(230, 176, 96, 179)` (light amber)

**DisplayBackground:**
- Changed from dark fill with double border to amber gradient frame matching switches
- Inner cutout changed from stroked outline to filled dark rectangle

### Build Status

- Compiles successfully
- All existing functionality preserved
- Per-track system now fully standalone (no vestigial sequence array dependencies)

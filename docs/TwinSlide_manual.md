# Twin Slide - User Manual

**Twisted Cable - Twin Slide**
Dual Acid Bass Synthesizer for VCV Rack

Version 2.1.0 | February 2026

---

## Table of Contents

1. [Overview](#overview)
2. [Module Layout](#module-layout)
3. [Sequencer Section](#sequencer-section)
4. [Synth Voice Section](#synth-voice-section)
5. [Expert Mode](#expert-mode)
6. [Acid Pattern Generator](#acid-pattern-generator)
7. [Microtuning](#microtuning)
8. [Twin Slide Expander](#twin-slide-expander)
9. [MonoSlide](#monoslide)
10. [Context Menu Options](#context-menu-options)
11. [Tips and Techniques](#tips-and-techniques)

---

## Overview

Twin Slide is a dual acid bass synthesizer module featuring twin 16-step sequencers driving twin synth voices based on circuit-accurate modelling.
The design combines the proven Phrase-Seq-32 architecture from ImpromptuModular with the Open303 DSP engine.

### Architecture

```
Song → Phrases → Sequences → [Track A + Track B] → Steps (16 each)
```

- **32 sequences**, each containing 2 tracks × 16 steps
- **Independent track settings**: length, run mode, transpose, rotate, clock division
- **Dual synth voices** with complete 303-style synthesis chain
- **60 HP total**: 40 HP main module + 8 HP expander + 12 HP MonoSlide

### Signal Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                        Twin Slide (40 HP)                        │
│  ┌────────────┐    ┌────────────┐    ┌────────────────────────┐ │
│  │ Sequencer  │───▶│  Track A   │───▶│  Synth Voice A ──▶ OUT │ │
│  │  32 seq    │    │  16 steps  │    │  (303 DSP)             │ │
│  │  x2 tracks │    ├────────────┤    ├────────────────────────┤ │
│  │            │───▶│  Track B   │───▶│  Synth Voice B ──▶ OUT │ │
│  └────────────┘    │  16 steps  │    │  (303 DSP)             │ │
│                    └────────────┘    └────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

---

## Module Layout

### Panel Sections (Left to Right)

| Section | Description |
|---------|-------------|
| Step LEDs | Two rows of 16 step buttons (Track A top, Track B bottom) |
| Octave + Keyboard | 5 octave buttons (-2 to +2) and piano keyboard |
| Synth A Controls | Left synth voice parameters |
| Sequencer Controls | Central encoder and transport |
| Synth B Controls | Right synth voice parameters |
| I/O | Clock, Reset, Run inputs; Audio outputs |

### Step/Phrase LEDs

Two rows of 16 LED buttons display sequence steps or song phrases:

- **Track A** (top row): Steps 1-16 of Track A
- **Track B** (bottom row): Steps 1-16 of Track B

LED Colors:
- **Orange**: Run cursor (active step during playback)
- **White**: Edit cursor position
- **Dimmed Orange**: Gate indicator (sequence mode), length indicator
- **Amber**: Transpose/Rotate display modes

### Track Select Switch

Located at top right, selects which track you're editing:
- **Up**: Track A
- **Down**: Track B

The selected track affects Length, Run Mode, Transpose, Rotate, and Clock Division settings.

---

## Sequencer Section

### Seq/Song Switch

Toggles between two editing modes:

| Mode | Display | Function |
|------|---------|----------|
| **Sequence** (up) | Sequence number (1-32) | Edit individual sequences |
| **Song** (down) | Phrase slot content | Chain sequences into a song |

### Data Encoder

The large central encoder controls different values depending on the current display mode:

| Display Mode | Encoder Function | Double-click |
|--------------|-----------------|--------------|
| Normal | Select sequence/phrase (1-32) | Reset to 1 |
| Length | Set track length (1-16) | Reset to 16 |
| Run Mode | Select run mode | Reset to FWD |
| Clock Div | Set clock division (/1-/5) | Reset to /1 |
| Transpose | Shift pitch (±24 semitones) | Undo all transpose |
| Rotate | Rotate steps (±16 positions) | Undo all rotation |

### Len/Mode Button (Right of Encoder)

Short press cycles through display modes:
1. **Length** - Set track length (1-16 steps) or song length (1-32 phrases)
2. **Run Mode** - Select playback pattern
3. **Clock Div** - Set clock division (sequence mode only)

Long press (hold): Access PPQN (pulses per quarter note) setting.

### Transpose/Rotate Button (Left of Encoder)

Short press cycles through:
1. **Transpose** - Shift all CV values in current track (±24 semitones)
2. **Rotate** - Move step data left/right (±16 positions)

### Run Modes

**Sequence Run Modes (7 modes):**

| Mode | Name | Description |
|------|------|-------------|
| FWD | Forward | Play steps 1→16 |
| REV | Reverse | Play steps 16→1 |
| PPG | Ping-Pong | Forward then reverse, repeating endpoints |
| PEN | Pendulum | Forward then reverse, no endpoint repeat |
| BRN | Brownian | Random walk (forward or backward each step) |
| RND | Random | Random step selection |
| RN2 | Random Non-Repeat | Random, never same step twice in a row |

**Song Run Modes (9 modes):**

All sequence modes plus:

| Mode | Name | Description |
|------|------|-------------|
| FW2 | Forward x2 | Play each phrase twice |
| FW3 | Forward x3 | Play each phrase three times |
| FW4 | Forward x4 | Play each phrase four times |

### Clock Division

Each track can divide the incoming clock independently:

| Setting | Effect |
|---------|--------|
| /1 | Every clock pulse (default) |
| /2 | Every 2nd clock pulse |
| /3 | Every 3rd clock pulse |
| /4 | Every 4th clock pulse |
| /5 | Every 5th clock pulse |

This enables polyrhythmic patterns between tracks.

### Copy/Paste System

Three-position switch selects copy mode:

| Position | Sequence Mode | Song Cross-Paste |
|----------|--------------|------------------|
| **4** | Copy/paste 4 steps | RCV - Randomize CVs |
| **8** | Copy/paste 8 steps | RG1 - Randomize gates |
| **All** | Copy/paste full track | TG1 - Toggle gates |

**Song Mode Cross-Paste:**

| Position | Effect |
|----------|--------|
| **4** | INC - Increment phrase numbers |
| **8** | RPH - Randomize phrases |
| **All** | CLR - Clear (initialize) |

### Step Attributes

Each step has the following attributes toggled by dedicated buttons:

| Button | Attribute | Function |
|--------|-----------|----------|
| **GATE** | Gate On/Off | Step produces a gate signal |
| **SLIDE** | Slide/Portamento | Glide pitch from previous step |
| **ACCENT** | Accent | Boost filter envelope and VCA |
| **TIE** | Tied Note | Extend gate into next step (no retrigger) |
| **PROB** | Probability | Gate fires probabilistically (use knob) |
| **COND** | Condition | Conditional step execution |

### Step Conditions

The COND button cycles through 5 conditions:

| Condition | Symbol | Description |
|-----------|--------|-------------|
| 0 | -- | Always play |
| 1 | 1- | Play on 1st loop only |
| 2 | -2 | Play on 2nd loop only |
| 3 | 12 | Play on 1st and 2nd loops |
| 4 | ?? | 50% probability |

### Gate Types

Hold a step button and turn the encoder to select from 12 gate shapes that modify how the gate signal is generated during that step.

### Slide Knob

The Slide knob controls the portamento rate for slide steps. Higher values produce slower, more pronounced glides between notes.

### Attach Mode

The ATTACH button (light above) links the edit cursor to playback:
- **Off**: Edit position is independent
- **On**: Edit position follows playback

---

## Synth Voice Section

Each synth voice (A and B) has identical controls:

### Main Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Cutoff** | 0-100% | 50% | Filter cutoff frequency (314-2394 Hz) |
| **Resonance** | 0-100% | 0% | Filter resonance/Q |
| **Env Mod** | 0-100% | 50% | Envelope modulation depth |
| **Decay** | 0-100% | 100% | Envelope decay time (200-2500 ms) |
| **Accent** | 0-100% | 50% | Accent intensity |
| **Drive** | 0-100% | 25% | Filter drive/saturation |
| **Fine Tune** | ±700 cents | 0 | Pitch fine tuning |
| **Waveform** | Saw/Square | Saw | Oscillator waveform |
| **Level** | 0-100% | 75% | Output level |

### CV Inputs (per voice)

| Input | Function |
|-------|----------|
| Cutoff CV | Modulate filter cutoff |
| Env Mod CV | Modulate envelope depth |
| Decay CV | Modulate decay time |

### DSP Architecture

Each voice contains a complete synthesis chain:

1. **SlideOscillator** - MipMapped wavetable saw/square with tanh soft saturation
2. **TeeBeeFilter303** - 4-pole diode ladder filter (mystran/kunn coefficients)
3. **AnalogEnvelope303** - RC-filter exponential envelope
4. **LeakyIntegrator303** - Pitch slew for slide/portamento
5. **DecayEnvelope303** - Dedicated accent decay envelope
6. **EnvModScaler303** - Measured envelope-to-cutoff mapping
7. **4x oversampling** in filter stage for aliasing reduction

---

## Expert Mode

Access via context menu → **Expert Mode A** or **Expert Mode B**

Each synth has 5 expert parameters:

### VCF Range

Shifts the filter frequency range:

| Value | Frequency Range | Character |
|-------|-----------------|-----------|
| -1.0 | 100-760 Hz | Deep bass |
| 0.0 | 314-2394 Hz | Original 303 (default) |

Uses piecewise exponential interpolation for smooth transitions.

### Decay Speed

Adjusts the decay envelope multiplier:

| Value | Multiplier | Effect |
|-------|------------|--------|
| -1.0 | 6.0x | Slower decay |
| 0.0 | 4.0x | Original 303 (default) |
| +1.0 | 2.0x | Faster decay |

### Sustain

Sets the envelope sustain level:

| Value | Level | Effect |
|-------|-------|--------|
| 0% | 0% | No sustain (default, 303 behavior) |
| 50% | 50% | Partial sustain |
| 100% | 100% | Full sustain (organ-like) |

### Release

Sets the envelope release time:

| Value | Time | Effect |
|-------|------|--------|
| 0% | 21 ms | Nearly instant (default, 303 behavior) |
| 50% | 260 ms | Medium release |
| 100% | 500 ms | Long release |

### Blend

Crossfades between saw and square waveforms:

| Value | Effect |
|-------|--------|
| 0% | Pure sawtooth |
| 50% | Mixed waveform |
| 100% | Pure square |

This allows smooth waveform transitions beyond the discrete switch positions.

### Indicator

When all expert parameters are at default (0, 0, 0%, 0%, 0%), the submenu shows "303" indicating original behavior.

---

## Acid Pattern Generator

Access via context menu → **Randomize Track**

The Acid Pattern Generator creates musically coherent acid bass patterns with configurable parameters.

### Generate

Click "Generate" to create a new pattern for the currently selected track. Each generation uses the current parameter settings.

### Scale Selection

24 scales organized in submenus:

**Top Level:**
- Random (selects randomly from all scales)
- Chromatic
- Whole Tone

**Modes:**
- Major
- Minor
- Dorian
- Mixolydian
- Lydian
- Phrygian
- Locrian

**Harmonic:**
- Harmonic Minor
- Harmonic Major
- Phrygian Dominant

**Melodic:**
- Melodic Minor
- Dorian #4
- Lydian Augmented
- Lydian Dominant
- Super Locrian

**Pentatonic:**
- Pentatonic Major
- Pentatonic Minor
- Blues Minor
- Japanese In-Sen

**World:**
- Hungarian Minor
- Spanish
- Bhairav

### Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Density** | 0-100% | 75% | Probability of notes occurring |
| **Spread** | 0-100% | 50% | Pitch range spread across octaves |
| **Accent Density** | 0-100% | 25% | Frequency of accent placement |
| **Slide Density** | 0-100% | 25% | Frequency of slide placement |
| **Transpose** | ±12 st | 0 | Pattern transposition in semitones |
| **Seed** | 0-999 | 303 | Random seed for reproducible patterns |

### Options

| Option | Default | Description |
|--------|---------|-------------|
| **Pin Seed** | Off | Prevents seed auto-increment after generation |
| **Start with Note** | On | Guarantees first step has a note |
| **Start with Accent** | Off | First note gets an accent |

### Seed Behavior

By default, the seed auto-increments after each generation (wraps at 999). This ensures variety while allowing you to recall specific patterns by noting the seed value. Enable "Pin Seed" to generate multiple variations at the same seed.

---

## Microtuning

Twin Slide includes a complete microtuning system for alternative tunings and temperaments.

### Microtuning Controls

Located on the MicroKeyboard widget:

| Control | Range | Function |
|---------|-------|----------|
| **Cents Knob** | ±100 cents | Adjust selected note's tuning |
| **Octave Switch** | -1, 0, +1 | Shift tuning by octaves |
| **Master Tune** | 420-460 Hz | Master reference frequency (default 440 Hz) |

### Editing Notes

1. Click a key on the MicroKeyboard to select it
2. Turn the Cents knob to adjust that note's tuning
3. Repeat for each note requiring adjustment

### Temperament Presets

Access via context menu → Microtuning

**47 temperament presets** organized by category:

**Western Historical:**

| Preset | Description |
|--------|-------------|
| INITIALIZE | Reset to 12-TET (all zeros) |
| JUST | Just intonation - pure intervals |
| PYTH | Pythagorean - based on perfect fifths |
| MEAN1/4 | Quarter-comma meantone |
| MEAN1/6 | Sixth-comma meantone |
| WERCK3 | Werckmeister III |
| WERCK4 | Werckmeister IV |
| KIRN3 | Kirnberger III |
| VALLOT | Vallotti |
| YOUNG | Young |
| KELLNR | Kellner |
| BARNES | Barnes-Bach |

**Extended Equal Divisions:**

| Preset | Description |
|--------|-------------|
| 17-TET | 17 equal divisions of octave |
| 19-TET | 19 equal divisions of octave |
| 22-TET | 22 equal divisions of octave |
| 24-TET | Quarter-tone (24 equal divisions) |
| 31-TET | 31 equal divisions of octave |
| 53-TET | 53 equal divisions of octave |

**Arabic Maqam:**

| Preset | Description |
|--------|-------------|
| RAST-E | Rast (Egyptian) |
| RAST-S | Rast (Syrian) |
| BAYATI | Bayati maqam |
| SABA | Saba maqam |
| HIJAZ | Hijaz maqam |
| SEGAH | Segah maqam |
| NAHWND | Nahawand maqam |
| KURD | Kurd maqam |

**Persian Radif:**

| Preset | Description |
|--------|-------------|
| SHUR | Shur dastgah |
| SEGAH-P | Segah (Persian) |
| CHAHAR | Chahargah |
| MAHUR | Mahur dastgah |
| HOMYUN | Homayun dastgah |
| NAVA | Nava dastgah |

**Sundanese Gamelan:**

| Preset | Description |
|--------|-------------|
| SLNDRO | Slendro |
| PELOG | Pelog |
| DEGUNG | Degung |
| MADNDA | Madenda |

**Thai/Asian:**

| Preset | Description |
|--------|-------------|
| THAI | Thai 7-tone |
| SHRUTI | Indian shruti |

**Harmonic Series:**

| Preset | Description |
|--------|-------------|
| HARM7 | 7-limit just intonation |
| HARM11 | 11-limit just intonation |
| HARM13 | 13-limit just intonation |

**Wendy Carlos:**

| Preset | Description |
|--------|-------------|
| ALPHA | Carlos Alpha |
| BETA | Carlos Beta |
| GAMMA | Carlos Gamma |

**Experimental/Modern:**

| Preset | Description |
|--------|-------------|
| LUCY | LucyTuning |
| GOLDEN | Golden ratio tuning |
| BOHLEN | Bohlen-Pierce (tritave) |

---

## Twin Slide Expander

The 8HP expander provides CV/Gate I/O for external sequencer integration.

### Inputs (Override Internal Sequencer)

| Input | Track | Function |
|-------|-------|----------|
| CV In | A/B | Pitch CV (1V/oct) |
| Gate In | A/B | Gate signal |
| Slide In | A/B | Slide trigger |
| Fine In | A/B | Fine tune CV |

When an input is connected, it overrides the internal sequencer for that signal.

### Outputs (From Internal Sequencer)

| Output | Track | Function |
|--------|-------|----------|
| CV Out | A/B | Pitch CV from sequencer |
| Gate Out | A/B | Gate from sequencer |
| Slide Out | A/B | Slide signal from sequencer |

### Connection Indicator

The "LINK" indicator fades from white to amber when connected to the main module.

### Placement

The expander must be placed immediately to the **right** of the main Twin Slide module.

---

## MonoSlide

MonoSlide is a standalone 12HP 303 synth voice designed for external sequencing.

### Overview

MonoSlide provides the complete Twin Slide synthesis engine in a compact format, accepting external CV/Gate/Slide signals. It includes all synthesis parameters with full CV control.

### Panel Layout

**Top Section (Large Knobs):**
- **VCF** - Filter cutoff frequency
- **ENV** - Envelope modulation depth
- **DEC** - Envelope decay time

**Display Area:**
- Waveform switch (Saw/Square)
- Waveform symbol indicator
- Parameter display (shows last touched knob)
- Connection status indicators

**Middle Section (Small Knobs):**
- **DRV** - Filter drive/saturation
- **ACC** - Accent intensity
- **RES** - Filter resonance
- **FINE** - Fine tune (±700 cents)
- **LEVEL** - Output level

### Inputs

**Parameter CV Inputs:**

| Input | Function |
|-------|----------|
| VCF | Cutoff CV modulation |
| ENV | Envelope mod CV |
| DEC | Decay CV modulation |
| DRV | Drive CV modulation |
| ACC | Accent CV modulation |
| FINE | Fine tune CV (±1V = ±700 cents) |
| LEVEL | Level CV modulation |

**Main Inputs:**

| Input | Function |
|-------|----------|
| CV | Pitch CV (1V/oct) |
| GATE | Gate signal (triggers envelope) |
| SLIDE | Slide trigger (enables portamento) |

### Output

| Output | Function |
|--------|----------|
| OUT | Audio output |

### Expert Mode

Access via context menu → **Expert Mode**

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| VCF Range | -1 to 0 | 0 | Filter frequency range shift |
| Decay Speed | -1 to +1 | 0 | Decay multiplier (6x to 2x) |
| Sustain | 0-100% | 0% | Envelope sustain level |
| Release | 0-100% | 0% | Release time (21-500 ms) |
| Blend | 0-100% | 0% | Saw/square crossfade |
| Slide Time | 0-100 ms | 37.5 ms | Portamento duration |

### Visual Feedback

**Parameter Display:**<br />
Shows the name and value of the last touched knob.

**Waveform Symbol:**<br />
Animated indicator showing current oscillator waveform with smooth crossfade.

**Connection Indicators:**<br />
Shows "√ CV" and "√ GATE" when respective inputs are connected (hidden when Slide is connected to reduce visual clutter).

---

## Context Menu Options

Right-click on the module panel to access:

### Randomize Track

See [Acid Pattern Generator](#acid-pattern-generator) section.

### Synth Settings

| Option | Description |
|--------|-------------|
| Expert Mode A | VCF Range, Decay Speed, Sustain, Release, Blend for Synth A |
| Expert Mode B | VCF Range, Decay Speed, Sustain, Release, Blend for Synth B |

### Sequencer Settings

| Option | Values | Description |
|--------|--------|-------------|
| Reset on run | On/Off | Reset sequence when Run is triggered |
| Retrigger gates on reset | No/Yes/Only when Run unconnected | Gate behavior on reset |
| Hold tied notes | On/Off | Sustain notes through tied steps |
| Single shot song | On/Off | Stop at end of song (no loop) |

### Microtuning

See [Microtuning](#microtuning) section for temperament presets.

### Portable Sequence

| Option | Description |
|--------|-------------|
| Copy sequence | Copy to clipboard (portable format) |
| Paste sequence | Paste from clipboard |

---

## Tips and Techniques

### Classic Acid Patterns

1. Use **short sequences** (4-8 steps) with high step density
2. Apply **Slide** on 1-2 steps per sequence for characteristic glide
3. Use **Accent** sparingly for dynamic variation
4. Set **Cutoff** low with high **Env Mod** for squelchy tones

### Dual Voice Techniques

1. **Octave Doubling**: Same pattern on both tracks, one octave apart
2. **Call and Response**: Alternate phrases between tracks
3. **Polyrhythm**: Use different clock divisions per track
4. **Harmony**: Program complementary patterns

### Using Clock Division for Polyrhythmic Patterns

- Track A at /1 + Track B at /2 = half-time bass layer
- Track A at /3 + Track B at /4 = complex polyrhythm
- Both tracks at /1 = tight unison

### Using Track Length for Polymetric Patterns

Each track can have an independent length (1-16 steps),
enabling polymetric compositions where patterns of different lengths phase against each other:

- **3 vs 4**: Track A = 3 steps, Track B = 4 steps → patterns realign every 12 steps
- **5 vs 7**: Track A = 5 steps, Track B = 7 steps → patterns realign every 35 steps
- **7 vs 16**: Track A = 7 steps, Track B = 16 steps → patterns realign every 112 steps
- **3 vs 16**: Short melodic cell against full sequence creates evolving variation

Combine with clock division for even more complex rhythmic interplay:
- Track A: 5 steps at /1 + Track B: 4 steps at /2 = interlocking polymetric/polyrhythmic texture

### Pattern Generator Tips

1. Use **Pin Seed** to create variations on a pattern you like
2. Lower **Density** for sparse, spacious patterns
3. Higher **Slide Density** with low **Accent Density** for smooth, flowing lines
4. The **Seed** value is reproducible - note seeds you like for later recall

### External Sequencing with MonoSlide

1. Connect external sequencer CV to MonoSlide CV input
2. Connect external gate to Gate input
3. Use Slide input for portamento triggers
4. Use CV inputs for parameter modulation from LFOs or envelopes

### Microtuning Tips

- **Just Intonation** sounds best in one key; avoid modulation
- **Werkmeister III** allows key changes with historical character
- Small **Master Tune** adjustments (±5 Hz) can help blend with other instruments
- Arabic and Persian presets work well with appropriate scales in the Pattern Generator

---

## Technical Specifications

| Specification | Value |
|---------------|-------|
| Module Widths | 40 HP (TwinSlide), 8 HP (Expander), 12 HP (MonoSlide) |
| Audio Voltage | ±5V |
| Pitch Standard | 1V/oct |
| Gate/Trigger | 10V |
| Oversampling | 4x in filter stage |
| Voices | 2 monophonic (TwinSlide), 1 monophonic (MonoSlide) |

---

## Keyboard Shortcuts

When hovering over the sequence display:

| Key | Function |
|-----|----------|
| 0-9 | Direct sequence/phrase number entry |
| Space | Advance to next phrase slot |

---

## Version History

See [CHANGELOG.md](../CHANGELOG.md) for complete version history.

---

**Twin Slide** - Copyright 2025 Twisted Cable<br />
Licensed under GPL-3.0-or-later

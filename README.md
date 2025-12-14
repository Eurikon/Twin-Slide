# Twin Slide

Dual acid bass synthesizer for VCV Rack with integrated step sequencer.

---

## Overview

**Twisted Cable - Twin Slide**
is a dual acid bass synthesizer module for VCV Rack, featuring twin sequencers driving twin synth voices.

The design builds upon the proven Phrase Seq 32 from ImpromptuModular.
Modified to two parallel tracks inside each sequence,
enabling simultaneous dual-voice (and/or duophonic) playback.
And the Open303 source code for the bass synth, based on circuit-accurate modelling.

```
Song → Phrases → Sequences → [Track A + Track B] → Steps (16 each)
```

## Modules

### Twin Slide - 40HP

Main module containing dual 16-step sequencers and dual synth voices.

**Sequencer:**
- 32 sequences, each containing 2 tracks x 16 steps
- Independent track length (1-16 steps per track)
- Independent run modes per track (FWD, REV, PPG, PEN, BRN, RND, RN2)
- Song mode with 9 run modes (adds FW2, FW3, FW4 repeat patterns)
- Per-step attributes: Gate, Probability, Slide, Accent, Tie, Condition
- 12 gate types with advanced gate modes
- Transpose (±24 semitones) and Rotate (±16 steps) per track
- Copy/paste with 4-step, 8-step, and full track modes
- Slide rate knob for portamento speed control

**Synth Voice (x2):**
- PolyBLEP oscillator (saw/square with tanh shaping)
- Diode ladder filter (circuit-accurate 303 emulation)
- RC-filter exponential envelope
- Pitch slew for slide/portamento
- Accent with dedicated decay envelope

**Per-Voice Controls:**

| Parameter | Range | Description |
|-----------|-------|-------------|
| Cutoff | 0-1 | Filter cutoff (314-2394Hz default) |
| Resonance | 0-1 | Filter resonance |
| Env Mod | 0-1 | Envelope modulation depth |
| Decay | 0-1 | Envelope decay (200-2500ms) |
| Accent | 0-1 | Accent intensity |
| Drive | 0-1 | Filter drive/saturation |
| Fine Tune | ±700 cents | Pitch fine tuning |
| Waveform | Saw/Square | Oscillator waveform |
| Level | 0-100% | Output level |

**Expert Mode (per synth, via context menu):**
- VCF Range: Shift filter toward deeper bass range (100-760Hz) or default (314-2394Hz)
- Decay Speed: Adjust decay multiplier (2.0x to 6.0x)
- Sustain: Hold envelope at target level (0-100%)
- Release: Release time (21ms original to 500ms)
- Blend: Saw/square waveform crossfade (0-100%)

**Acid Pattern Generator (via context menu):**<br />
Randomize Track submenu with configurable pattern generation:
- 24 scales: Major, Minor, Dorian, Mixolydian, Lydian, Phrygian, Locrian, Harmonic Minor, Harmonic Major, Dorian #4, Phrygian Dominant, Melodic Minor, Lydian Augmented, Lydian Dominant, Hungarian Minor, Super Locrian, Spanish, Bhairav, Pentatonic Minor, Pentatonic Major, Blues Minor, Whole Tone, Chromatic, Japanese In-Sen
- Density (0-100%): Note density in pattern
- Spread (0-100%): Pitch range spread
- Accent Density (0-100%): Frequency of accents
- Slide Density (0-100%): Frequency of slides
- Transpose (±12 semitones): Pattern transposition
- Seed (0-999): Reproducible pattern generation
- Pin Seed: Prevent auto-increment
- Start with Note: Guarantee first step has note
- Start with Accent: First note accented

**Microtuning:**
- Master tuning (420-460Hz, default 440Hz)
- Per-key cent offsets (±100 cents)
- 47 temperament presets including Western historical, extended equal divisions, Arabic maqam, Persian radif, Sundanese gamelan, Thai/Asian, harmonic series, Wendy Carlos scales, and experimental tunings

### Twin Slide Expander - 8HP

CV/Gate I/O expander for external sequencer integration.

**Per Track (A and B):**
- CV Input/Output
- Gate Input/Output
- Slide Input/Output
- Fine CV Input

**Features:**
- Bidirectional communication with main module
- Link indicator (amber when connected)
- Override internal sequencer with external CV/Gate

### MonoSlide - 12HP

Standalone 303 synth voice for external sequencing.

**Controls:**
- VCF (Cutoff), ENV (Envelope Mod), DEC (Decay) - large knobs
- DRV (Drive), ACC (Accent), RES (Resonance), FINE (Fine Tune) - small knobs
- LEVEL output level
- Waveform switch (Saw/Square)

**CV Inputs (all parameters):**
- Cutoff, Env Mod, Decay, Drive, Accent, Fine, Level

**Main Inputs:**
- CV (V/Oct pitch)
- Gate
- Slide (portamento trigger)

**Output:**
- Audio

**Expert Mode (via context menu):**
- VCF Range: Filter frequency range shift
- Decay Speed: Decay multiplier
- Sustain: Envelope sustain level
- Release: Envelope release time
- Blend: Saw/square crossfade
- Slide Time: Portamento duration

**Visual Feedback:**
- Parameter display showing last touched knob value
- Waveform symbol indicating current oscillator type
- Connection indicators for CV/Gate status

## Signal Flow

```
  Twin Slide (40 HP) + Expander (8 HP)

  ┌─────────────────────────────────────────────────────────────────┐
  │                        Twin Slide                                 │
  │  ┌────────────┐    ┌────────────┐    ┌────────────────────────┐ │
  │  │ Sequencer  │───▶│  Track A   │───▶│  Synth Voice A ──▶ OUT │ │
  │  │  32 seq    │    │  16 steps  │    │  (303 DSP)             │ │
  │  │  x2 tracks │    ├────────────┤    ├────────────────────────┤ │
  │  │            │───▶│  Track B   │───▶│  Synth Voice B ──▶ OUT │ │
  │  └────────────┘    │  16 steps  │    │  (303 DSP)             │ │
  │                    └────────────┘    └────────────────────────┘ │
  └─────────────────────────────────────────────────────────────────┘
                               ▲ ▼
                   ┌───────────────────────┐
                   │   TwinSlide Expander  │
                   │  CV/Gate I/O per track│
                   └───────────────────────┘


  MonoSlide (12 HP) - Standalone Voice

  ┌────────────────────────────────────────────┐
  │             ┌────────────────────┐         │
  │  CV    ────▶│                    │         │
  │  Gate  ────▶│ Synth Voice (303)  │───▶ OUT │
  │  Slide ────▶│                    │         │
  │             └────────────────────┘         │
  └────────────────────────────────────────────┘
```

### Sequencer Modes

**Sequence Mode:**
- Edit individual sequences (1-32)
- Set track length, run mode, transpose, rotate per track
- Clock division per track (÷1 to ÷5)

**Song Mode:**
- Chain sequences into phrases (1-32 phrases)
- 9 song run modes including repeat patterns (FW2, FW3, FW4)
- Non-repeating random (RN2) ensures variety

## Building from Source

### Requirements

- **VCV Rack**: VCV Rack source/SDK
- **C++17 Compiler**: GCC, Clang, or MSVC with C++17 support


## Technical Specifications

- **Voltage Range**: ±5V audio, 1V/oct pitch, 10V triggers/gates
- **Sample Rate**: Supports all VCV Rack sample rates
- **Oversampling**: 4x in filter stage
- **Filter**: Circuit-accurate 303 diode ladder (mystran/kunn coefficients)
- **Polyphony**: Dual monophonic voices (TwinSlide), single monophonic (MonoSlide)
- **Module Widths**: 40 HP (TwinSlide), 8 HP (Expander), 12 HP (MonoSlide)

## Attribution

This project builds upon GPL-3.0-licensed work from:
- **ImpromptuModular**: Phrase-Seq-32 sequencer architecture (Marc Boulé)
- **Open303**: Synth DSP and filter design (Robin Schmidt / RS-MET / ROSIC)
- **Filter Coefficients**: mystran (Teemu Voipio) and kunn (KVR forum)

See [ATTRIBUTION.md](ATTRIBUTION.md) for complete source code credits.

## License

This project is licensed under GPL-3.0-or-later.

See [LICENSE.md](LICENSE.md) for full license text.

---

**Version**: 2.1.0<br />
**Date**: 03-02-2026<br />
**Author**: Latif Karoumi

**Copyright 2025 Twisted Cable. Licensed under GPL-3.0-or-later.**

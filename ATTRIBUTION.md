# Twin Slide - Attribution

This project builds upon the work of several open-source projects. 
All incorporated code is GPL-3.0 compatible.

---

## Sequencer Architecture

**ImpromptuModular - Phrase-Seq-32**
- **Author**: Marc Boulé
- **License**: GPL-3.0-or-later
- **Repository**: https://github.com/MarcBoule/ImpromptuModular
- **Used For**: Sequencer core architecture, step attributes, run modes, song/phrase structure

Adapted components:
- `SeqUtil.hpp/cpp` - Step and sequence attribute classes
- `TwinSlidePlugin.hpp` - Utility classes (RefreshCounter, Trigger, HoldDetect)
- `comp/PianoKey.hpp/cpp` - Piano keyboard widget
- Sequencer logic in `TwinSlide.cpp`

---

## Synth DSP

**Open303**
- **Author**: Robin Schmidt (RS-MET / ROSIC)
- **License**: MIT
- **Repository**: https://github.com/RobinSchmidt/Open303
- **Used For**: Complete 303-style synth voice DSP

Ported components in `SynthDSP.hpp`:
- `OnePoleFilter303` - Feedback highpass filter
- `TeeBeeFilter303` - Diode ladder filter (circuit-accurate)
- `SlideOscillator` - PolyBLEP saw/square oscillator
- `AnalogEnvelope303` - RC-style VCA envelope
- `LeakyIntegrator303` - Pitch slew for slide/portamento
- `SimpleHighpass303` - Pre/post filter highpass
- `EnvModScaler303` - Envelope to cutoff mapping
- `DecayEnvelope303` - Accent decay envelope
- `SynthVoice` - Complete voice wrapper

---

## Filter Coefficients

**mystran (Teemu Voipio) and kunn**
- **Source**: KVR Audio Forum (page 40 of 303 filter thread)
- **Used For**: Polynomial-approximated coefficients matching real 303 response

Technical details:
- 1+3 pole configuration (characteristic of 303 diode ladder)
- Highpass filter in feedback path reducing bass resonance
- Nonlinear waveshaping between stages

---

## VCV Rack Components

**VCV Rack Component Library**
- **Author**: Andrew Belt / VCV
- **License**: CC BY-NC 4.0
- **Repository**: https://github.com/VCVRack/Rack
- **Used For**: Adapted component graphics in `res/comp/complib/`

---

## Fonts

**Sono**
- **License**: SIL Open Font License
- **Used For**: UI text rendering

**Segment14**
- **Used For**: 14-segment display rendering

---

## Acknowledgments

Special thanks to:
- Marc Boulé for the elegant Phrase-Seq-32 architecture
- Robin Schmidt for the meticulous Open303 DSP implementation
- mystran and kunn for the circuit analysis that made accurate 303 filter emulation possible
- The VCV Rack community for continuous feedback and support

---

**Twin Slide** - Copyright © 2025 Latif Karoumi / Twisted Cable
Licensed under GPL-3.0-or-later

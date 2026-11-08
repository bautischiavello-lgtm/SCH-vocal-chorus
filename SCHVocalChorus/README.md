# SCH VOCAL CHORUS (VST3)

Retro pixel-art vocal choir plugin. C++17, JUCE 8 (7+ compatible API), CMake.

## Build (Windows 64-bit, Visual Studio 2022)

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

JUCE 8.0.4 is fetched automatically. To use a local checkout instead:
`cmake -B build -DSCH_JUCE_DIR=C:/path/to/JUCE ...`

The VST3 is produced under
`build/SCHVocalChorus_artefacts/Release/VST3/SCH Vocal Chorus.vst3`.
Copy it to `C:\Program Files\Common Files\VST3`.

## Source layout

| File | Role |
|---|---|
| `Source/ChoirEngine.h` | DSP voice generator (no JUCE dependency) |
| `Source/PluginProcessor.*` | AudioProcessor, parameters (APVTS), state |
| `Source/PluginEditor.*` | Editor / paint routines |
| `Source/PixelLookAndFeel.*` | Custom LookAndFeel (knobs, slider, combo, toggle) |
| `Source/PixelArt.h` | 5x7 bitmap font, avatar sprite, sparkles, palette |

## DSP notes

- Per voice: two-tap crossfaded delay-line pitch shifter, slew-limited random
  micro-delay (0-50 ms x mode factor), drift LFO, two peaking-biquad formant
  filters, optional tanh drive (Shout), fixed random pan scaled by WIDTH.
- DETUNE: |value| = max random pitch spread per voice; sign skews the choir
  flat (-) or sharp (+).
- All buffers/filters are allocated in `prepareToPlay()`; `processBlock()` makes
  no allocations (verified with an allocation counter in an offline test).
- Measured: ~4% of one core for 32 voices at 48 kHz (offline test, -O2).
- Latency reported: 0. The wet choir is inherently delayed by the micro-delays
  (about 2-50 ms), which is part of the effect.

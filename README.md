# DDIEGO Y2K FX

JUCE 8 sample-based VST3 effect with a real-time granularizer and MIDI-triggered
slice chopper. Audio samples are decoded through JUCE's audio format readers;
the plugin accepts WAV, AIFF, FLAC, and MP3 when the bundled JUCE MP3 decoder is
available.

## Build

Requirements: CMake 3.22+, a C++17 toolchain supported by JUCE 8, and Git when
using the default dependency setup. JUCE is open source and is fetched from the
JUCE 8.0.12 release by CMake; no paid JUCE distribution or third-party DSP
library is required. To use an existing JUCE 8 checkout instead, pass
`-DJUCE_SOURCE_DIR=C:\path\to\JUCE`.

The project enables JUCE's optional MP3 decoder to support MP3 loading. JUCE
includes a specific legal disclaimer for that decoder; review the disclaimer
in `juce_MP3AudioFormat.h` and determine whether enabling it is appropriate for
your distribution. WAV, AIFF, and FLAC do not depend on the MP3 decoder.

On Windows with Visual Studio Build Tools:

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target DDIEGO_Y2K_FX_VST3 --parallel
```

The VST3 bundle is created under
`build\DDIEGO_Y2K_FX_artefacts\Release\VST3\DDIEGO Y2K FX.vst3`.

## Use

- **Granularizer** leaves the DAW input unchanged at Granular Amount 0%; raising
  it blends in windowed, pitch-shifted grains from the loaded sample. Grain
  Size, Density, Position / Scan,
  Randomness, Pitch, Texture, and Dry / Wet are host-automatable.
- **Chopper** responds to MIDI notes C1–B2 (MIDI notes 24–47), with note-to-slice
  mapping adapting to 1/4, 1/8, or randomized divisions. Use the on-screen
  keyboard or a DAW MIDI track routed to the plugin. Start Offset, Slice Length,
  Reverse, Randomize Map, Pitch, Attack, Release, and Dry / Wet control playback.
- Load from the button or drop an audio file onto the waveform. The loaded file
  path and all parameter values are saved in plugin state; the sample itself
  stays on disk, so move/copy it with the session when transferring projects.
- The plugin supports mono and stereo input/output. State reload reports an
  error in the editor if the saved sample path is no longer available.
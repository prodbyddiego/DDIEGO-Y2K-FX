#include "ChopperEngine.h"

void ChopperEngine::prepare(double sampleRate, int, int)
{
    sr = sampleRate;
}

void ChopperEngine::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(buffer, midi, sr);
    // MIDI slice-trigger architecture placeholder.
    // Full audio slicing/sample playback will be added in the next version.
}

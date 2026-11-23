#pragma once
#include <JuceHeader.h>

class ChopperEngine
{
public:
    void prepare(double sampleRate, int samplesPerBlock, int channels);
    void process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);
private:
    double sr = 44100.0;
};

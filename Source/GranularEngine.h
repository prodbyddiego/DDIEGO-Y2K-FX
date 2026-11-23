#pragma once
#include <JuceHeader.h>

class GranularEngine
{
public:
    void prepare(double sampleRate, int samplesPerBlock, int channels);
    void process(juce::AudioBuffer<float>& buffer);
private:
    double sr = 44100.0;
    float phase = 0.0f;
};

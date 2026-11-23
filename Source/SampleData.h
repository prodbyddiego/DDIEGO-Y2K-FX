#pragma once

#include <JuceHeader.h>

#include <utility>
#include <vector>

struct SampleData
{
    juce::AudioBuffer<float> audio;
    std::vector<std::pair<float, float>> waveform;
    double sampleRate = 44100.0;
    juce::String filePath;
    juce::String fileName;
};

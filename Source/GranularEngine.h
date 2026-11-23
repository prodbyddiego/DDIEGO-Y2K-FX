#pragma once

#include <JuceHeader.h>
#include "SampleData.h"

#include <array>
#include <random>

class GranularEngine
{
public:
    struct Parameters
    {
        float amount = 0.0f;
        float grainSizeMs = 120.0f;
        float density = 12.0f;
        float position = 0.5f;
        float randomness = 0.15f;
        float pitchSemitones = 0.0f;
        float texture = 0.35f;
        float dryWet = 1.0f;
    };

    void prepare(double sampleRate);
    void process(juce::AudioBuffer<float>& buffer,
                 const SampleData* sample,
                 const Parameters& parameters);

private:
    struct Grain
    {
        double position = 0.0;
        double start = 0.0;
        double length = 0.0;
        double age = 0.0;
        double increment = 1.0;
        bool active = false;
    };

    static constexpr int maxGrains = 32;
    static float readSample(const SampleData& sample, int channel, double position);

    std::array<Grain, maxGrains> grains {};
    std::minstd_rand random { 0x594b };
    const SampleData* currentSample = nullptr;
    double outputSampleRate = 44100.0;
    double samplesUntilNextGrain = 0.0;
    double scanHead = 0.0;
    juce::SmoothedValue<float> amountSmoothed;
    juce::SmoothedValue<float> dryWetSmoothed;
};

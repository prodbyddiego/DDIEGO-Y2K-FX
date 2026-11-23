#pragma once

#include <JuceHeader.h>
#include "SampleData.h"

#include <array>
#include <atomic>
#include <random>

class ChopperEngine
{
public:
    struct Parameters
    {
        int division = 0;
        float startOffset = 0.0f;
        float sliceLength = 1.0f;
        bool reverse = false;
        float pitchSemitones = 0.0f;
        float attackMs = 5.0f;
        float releaseMs = 200.0f;
        float dryWet = 1.0f;
    };

    void prepare(double sampleRate);
    void process(juce::AudioBuffer<float>& buffer,
                 juce::MidiBuffer& midi,
                 const SampleData* sample,
                 const Parameters& parameters);
    void requestRandomize() noexcept { randomizePending.store(true, std::memory_order_relaxed); }
    int getSliceCount() const noexcept { return displayedSliceCount.load(std::memory_order_relaxed); }

private:
    struct Voice
    {
        int note = -1;
        float velocity = 1.0f;
        double position = 0.0;
        double start = 0.0;
        double end = 0.0;
        double increment = 1.0;
        double age = 0.0;
        double attackSamples = 0.0;
        double releaseSamples = 1.0;
        double releaseElapsed = 0.0;
        float releaseStartLevel = 0.0f;
        bool releasing = false;
        bool active = false;
    };

    static constexpr int firstMidiNote = 24;
    static constexpr int mappedNotes = 24;
    static constexpr int maxVoices = 24;

    static float readSample(const SampleData& sample, int channel, double position);
    void updateMapping(int sliceCount, bool randomize = false);
    void triggerNote(int note, float velocity, const SampleData& sample, const Parameters& parameters);
    void releaseNote(int note);
    float getEnvelopeLevel(const Voice& voice) const;

    std::array<Voice, maxVoices> voices {};
    std::array<int, mappedNotes> noteToSlice {};
    std::minstd_rand random { 0x594b };
    std::atomic<bool> randomizePending { false };
    std::atomic<int> displayedSliceCount { 8 };
    juce::SmoothedValue<float> dryWetSmoothed;
    const SampleData* currentSample = nullptr;
    double outputSampleRate = 44100.0;
    int selectedDivision = -1;
    int sliceCount = 8;
};

#include "GranularEngine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

void GranularEngine::prepare(double sampleRate)
{
    outputSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    amountSmoothed.reset(outputSampleRate, 0.02);
    dryWetSmoothed.reset(outputSampleRate, 0.02);
    amountSmoothed.setCurrentAndTargetValue(0.0f);
    dryWetSmoothed.setCurrentAndTargetValue(1.0f);
    currentSample = nullptr;
    samplesUntilNextGrain = 0.0;
    scanHead = 0.0;
    for (auto& grain : grains)
        grain.active = false;
}

float GranularEngine::readSample(const SampleData& sample, int channel, double position)
{
    const auto numSamples = sample.audio.getNumSamples();
    if (numSamples <= 0)
        return 0.0f;

    auto wrapped = std::fmod(position, static_cast<double>(numSamples));
    if (wrapped < 0.0)
        wrapped += numSamples;
    const auto first = static_cast<int>(wrapped);
    const auto second = (first + 1) % numSamples;
    const auto fraction = static_cast<float>(wrapped - first);
    const auto sourceChannel = std::min(channel, sample.audio.getNumChannels() - 1);
    const auto* data = sample.audio.getReadPointer(sourceChannel);
    return data[first] + (data[second] - data[first]) * fraction;
}

void GranularEngine::process(juce::AudioBuffer<float>& buffer,
                             const SampleData* sample,
                             const Parameters& parameters)
{
    if (sample == nullptr || sample->audio.getNumSamples() == 0
        || sample->audio.getNumChannels() == 0)
        return;

    if (sample != currentSample)
    {
        currentSample = sample;
        scanHead = 0.0;
        samplesUntilNextGrain = 0.0;
        for (auto& grain : grains)
            grain.active = false;
    }

    amountSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, parameters.amount));
    dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, parameters.dryWet));

    const auto sourceLength = static_cast<double>(sample->audio.getNumSamples());
    const auto sourceRate = sample->sampleRate > 0.0 ? sample->sampleRate : outputSampleRate;
    const auto grainLength = juce::jlimit(10.0, 500.0,
                                          static_cast<double>(parameters.grainSizeMs))
                            * outputSampleRate / 1000.0;
    const auto density = juce::jlimit(1.0, 60.0, static_cast<double>(parameters.density));
    const auto spawnInterval = outputSampleRate / density;
    const auto availableLength = std::max(1.0, sourceLength - 1.0);
    const auto pitchRatio = std::pow(2.0, static_cast<double>(parameters.pitchSemitones) / 12.0)
                            * sourceRate / outputSampleRate;
    const auto textureGain = 1.0f + juce::jlimit(0.0f, 1.0f, parameters.texture) * 5.0f;
    const auto textureNormaliser = std::tanh(textureGain);

    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
    {
        if (samplesUntilNextGrain <= 0.0)
        {
            auto* next = &grains.front();
            for (auto& grain : grains)
            {
                if (!grain.active)
                {
                    next = &grain;
                    break;
                }

                if (grain.age / std::max(1.0, grain.length)
                    > next->age / std::max(1.0, next->length))
                    next = &grain;
            }

            std::uniform_real_distribution<double> offsetDistribution(
                -0.5 * parameters.randomness * availableLength,
                 0.5 * parameters.randomness * availableLength);
            const auto nominalStart = parameters.position * availableLength + scanHead;
            auto start = std::fmod(nominalStart + offsetDistribution(random), availableLength);
            if (start < 0.0)
                start += availableLength;

            next->start = start;
            next->position = start;
            next->length = grainLength;
            next->age = 0.0;
            next->increment = pitchRatio;
            next->active = true;
            scanHead = std::fmod(scanHead + grainLength * 0.18, availableLength);
            samplesUntilNextGrain += spawnInterval;
        }

        std::array<float, 2> granularFrame {};
        double windowSum = 0.0;
        for (auto& grain : grains)
        {
            if (!grain.active)
                continue;

            const auto phase = grain.age / std::max(1.0, grain.length - 1.0);
            const auto window = 0.5 - 0.5 * std::cos(juce::MathConstants<double>::twoPi * phase);
            windowSum += window;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                granularFrame[static_cast<size_t>(channel)] +=
                    readSample(*sample, channel, grain.position) * static_cast<float>(window);

            grain.position += grain.increment;
            grain.age += 1.0;
            if (grain.age >= grain.length)
                grain.active = false;
        }

        if (windowSum > 0.00001)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                granularFrame[static_cast<size_t>(channel)] /= static_cast<float>(windowSum);

        const auto amount = amountSmoothed.getNextValue();
        const auto dryWet = dryWetSmoothed.getNextValue();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* output = buffer.getWritePointer(channel);
            const auto dry = output[sampleIndex];
            const auto grain = textureGain > 1.001f
                ? std::tanh(granularFrame[static_cast<size_t>(channel)] * textureGain)
                    / textureNormaliser
                : granularFrame[static_cast<size_t>(channel)];
            const auto effected = dry + (grain - dry) * amount;
            output[sampleIndex] = dry + (effected - dry) * dryWet;
        }

        samplesUntilNextGrain -= 1.0;
    }
}

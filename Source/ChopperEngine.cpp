#include "ChopperEngine.h"

#include <algorithm>
#include <cmath>
#include <numeric>

void ChopperEngine::prepare(double sampleRate)
{
    outputSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    dryWetSmoothed.reset(outputSampleRate, 0.02);
    dryWetSmoothed.setCurrentAndTargetValue(1.0f);
    currentSample = nullptr;
    selectedDivision = -1;
    sliceCount = 8;
    displayedSliceCount.store(sliceCount, std::memory_order_relaxed);
    randomizePending.store(false, std::memory_order_relaxed);
    updateMapping(sliceCount);
    for (auto& voice : voices)
        voice.active = false;
}

float ChopperEngine::readSample(const SampleData& sample, int channel, double position)
{
    const auto numSamples = sample.audio.getNumSamples();
    if (numSamples <= 0)
        return 0.0f;

    const auto clamped = juce::jlimit(0.0, static_cast<double>(numSamples - 1), position);
    const auto first = static_cast<int>(clamped);
    const auto second = std::min(first + 1, numSamples - 1);
    const auto fraction = static_cast<float>(clamped - first);
    const auto sourceChannel = std::min(channel, sample.audio.getNumChannels() - 1);
    const auto* data = sample.audio.getReadPointer(sourceChannel);
    return data[first] + (data[second] - data[first]) * fraction;
}

void ChopperEngine::updateMapping(int count, bool randomize)
{
    sliceCount = juce::jlimit(4, 16, count);
    std::array<int, 16> shuffled {};
    std::iota(shuffled.begin(), shuffled.end(), 0);
    if (randomize)
        std::shuffle(shuffled.begin(), shuffled.begin() + sliceCount, random);

    for (int noteIndex = 0; noteIndex < mappedNotes; ++noteIndex)
        noteToSlice[static_cast<size_t>(noteIndex)] =
            shuffled[static_cast<size_t>(noteIndex % sliceCount)];

    displayedSliceCount.store(sliceCount, std::memory_order_relaxed);
}

float ChopperEngine::getEnvelopeLevel(const Voice& voice) const
{
    if (voice.releasing)
    {
        if (voice.releaseSamples <= 0.0)
            return 0.0f;
        return voice.releaseStartLevel * static_cast<float>(
            juce::jlimit(0.0, 1.0, 1.0 - voice.releaseElapsed / voice.releaseSamples));
    }

    if (voice.attackSamples <= 0.0)
        return voice.velocity;

    return voice.velocity * static_cast<float>(
        juce::jlimit(0.0, 1.0, voice.age / voice.attackSamples));
}

void ChopperEngine::triggerNote(int note,
                                float velocity,
                                const SampleData& sample,
                                const Parameters& parameters)
{
    auto* voice = &voices.front();
    for (auto& candidate : voices)
    {
        if (!candidate.active)
        {
            voice = &candidate;
            break;
        }

        if (candidate.age > voice->age)
            voice = &candidate;
    }

    auto noteIndex = (note - firstMidiNote) % mappedNotes;
    if (noteIndex < 0)
        noteIndex += mappedNotes;
    const auto sourceLength = static_cast<double>(sample.audio.getNumSamples());
    const auto sourceRate = sample.sampleRate > 0.0 ? sample.sampleRate : outputSampleRate;
    const auto offset = juce::jlimit(0.0, 1.0, static_cast<double>(parameters.startOffset));
    const auto usableStart = std::min(sourceLength - 1.0, sourceLength * offset);
    const auto usableLength = std::max(1.0, sourceLength - usableStart);
    const auto playableSliceCount = juce::jmax(1, juce::jmin(
        sliceCount, static_cast<int>(usableLength)));
    const auto selectedSlice = noteToSlice[static_cast<size_t>(noteIndex)] % playableSliceCount;
    const auto nominalSliceLength = usableLength / playableSliceCount;
    const auto start = usableStart + nominalSliceLength * selectedSlice;
    const auto length = std::max(1.0, nominalSliceLength
                                      * juce::jlimit(0.1, 1.0,
                                          static_cast<double>(parameters.sliceLength)));
    const auto end = std::min(sourceLength, start + length);

    voice->note = note;
    voice->velocity = juce::jlimit(0.0f, 1.0f, velocity);
    voice->start = start;
    voice->end = end;
    voice->position = parameters.reverse ? std::max(start, end - 1.0) : start;
    voice->increment = std::pow(2.0, static_cast<double>(parameters.pitchSemitones) / 12.0)
                       * sourceRate / outputSampleRate
                       * (parameters.reverse ? -1.0 : 1.0);
    voice->age = 0.0;
    voice->attackSamples = juce::jlimit(0.0, 500.0, static_cast<double>(parameters.attackMs))
                           * outputSampleRate / 1000.0;
    voice->releaseSamples = juce::jlimit(1.0, 2000.0, static_cast<double>(parameters.releaseMs))
                            * outputSampleRate / 1000.0;
    voice->releaseElapsed = 0.0;
    voice->releaseStartLevel = 0.0f;
    voice->releasing = false;
    voice->active = true;
}

void ChopperEngine::releaseNote(int note)
{
    for (auto& voice : voices)
    {
        if (voice.active && voice.note == note && !voice.releasing)
        {
            voice.releaseStartLevel = getEnvelopeLevel(voice);
            voice.releaseElapsed = 0.0;
            voice.releasing = true;
        }
    }
}

void ChopperEngine::process(juce::AudioBuffer<float>& buffer,
                            juce::MidiBuffer& midi,
                            const SampleData* sample,
                            const Parameters& parameters)
{
    if (sample == nullptr || sample->audio.getNumSamples() == 0
        || sample->audio.getNumChannels() == 0)
        return;

    if (sample != currentSample)
    {
        currentSample = sample;
        for (auto& voice : voices)
            voice.active = false;
    }

    if (parameters.division != selectedDivision)
    {
        selectedDivision = parameters.division;
        if (selectedDivision == 2)
        {
            std::uniform_int_distribution<int> countDistribution(4, 16);
            updateMapping(countDistribution(random));
        }
        else
        {
            updateMapping(selectedDivision == 1 ? 8 : 4);
        }
    }

    if (randomizePending.exchange(false, std::memory_order_relaxed))
    {
        if (selectedDivision == 2)
        {
            std::uniform_int_distribution<int> countDistribution(4, 16);
            updateMapping(countDistribution(random), true);
        }
        else
        {
            updateMapping(sliceCount, true);
        }
    }

    dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, parameters.dryWet));

    juce::MidiBuffer::Iterator midiIterator(midi);
    juce::MidiMessage message;
    int midiSamplePosition = 0;
    auto hasMidiEvent = midiIterator.getNextEvent(message, midiSamplePosition);

    for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
    {
        while (hasMidiEvent && midiSamplePosition <= sampleIndex)
        {
            if (message.isNoteOn())
                triggerNote(message.getNoteNumber(), message.getFloatVelocity(), *sample, parameters);
            else if (message.isNoteOff())
                releaseNote(message.getNoteNumber());

            hasMidiEvent = midiIterator.getNextEvent(message, midiSamplePosition);
        }

        std::array<float, 2> sampleFrame {};
        int activeVoices = 0;
        for (auto& voice : voices)
        {
            if (!voice.active)
                continue;

            if (!voice.releasing
                && (voice.position < voice.start || voice.position >= voice.end))
            {
                voice.active = false;
                continue;
            }

            const auto envelope = getEnvelopeLevel(voice);
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                sampleFrame[static_cast<size_t>(channel)] +=
                    readSample(*sample, channel, voice.position) * envelope;
            ++activeVoices;

            voice.position += voice.increment;
            voice.age += 1.0;
            if (voice.releasing)
            {
                voice.releaseElapsed += 1.0;
                if (voice.releaseElapsed >= voice.releaseSamples)
                    voice.active = false;
            }
        }

        const auto normaliser = activeVoices > 1
            ? 1.0f / std::sqrt(static_cast<float>(activeVoices))
            : 1.0f;
        const auto dryWet = dryWetSmoothed.getNextValue();
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* output = buffer.getWritePointer(channel);
            const auto dry = output[sampleIndex];
            const auto wet = sampleFrame[static_cast<size_t>(channel)] * normaliser;
            output[sampleIndex] = dry + (wet - dry) * dryWet;
        }
    }
}

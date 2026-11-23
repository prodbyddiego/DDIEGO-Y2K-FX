#include "GranularEngine.h"

void GranularEngine::prepare(double sampleRate, int, int)
{
    sr = sampleRate;
    phase = 0.0f;
}

void GranularEngine::process(juce::AudioBuffer<float>& buffer)
{
    juce::ignoreUnused(sr);

    // Lightweight placeholder effect so the plugin is audio-processing capable.
    // The full sample-based granular engine will replace this in the next version.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float wobble = 1.0f + 0.0025f * std::sin(phase);
            data[i] *= wobble;
            phase += 0.08f;
            if (phase > juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;
        }
    }
}

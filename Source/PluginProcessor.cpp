#include "PluginProcessor.h"
#include "PluginEditor.h"

DDIEGO_Y2K_FXAudioProcessor::DDIEGO_Y2K_FXAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void DDIEGO_Y2K_FXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    granular.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    chopper.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void DDIEGO_Y2K_FXAudioProcessor::releaseResources()
{
}

bool DDIEGO_Y2K_FXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto main = layouts.getMainOutputChannelSet();
    return main == juce::AudioChannelSet::mono()
        || main == juce::AudioChannelSet::stereo();
}

void DDIEGO_Y2K_FXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    granular.process(buffer);
    chopper.process(buffer, midiMessages);
}

juce::AudioProcessorEditor* DDIEGO_Y2K_FXAudioProcessor::createEditor()
{
    return new DDIEGO_Y2K_FXAudioProcessorEditor(*this);
}

void DDIEGO_Y2K_FXAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    stream.writeFloat(0.5f);
}

void DDIEGO_Y2K_FXAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

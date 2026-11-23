#pragma once
#include <JuceHeader.h>
#include "GranularEngine.h"
#include "ChopperEngine.h"

class DDIEGO_Y2K_FXAudioProcessor : public juce::AudioProcessor
{
public:
    DDIEGO_Y2K_FXAudioProcessor();
    ~DDIEGO_Y2K_FXAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

private:
    GranularEngine granular;
    ChopperEngine chopper;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DDIEGO_Y2K_FXAudioProcessor)
};

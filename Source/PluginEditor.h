#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DDIEGO_Y2K_FXAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DDIEGO_Y2K_FXAudioProcessorEditor(DDIEGO_Y2K_FXAudioProcessor&);
    ~DDIEGO_Y2K_FXAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DDIEGO_Y2K_FXAudioProcessor& processor;

    juce::TextButton granularButton { "GRANULAR" };
    juce::TextButton chopperButton { "CHOPPER" };
    juce::Slider amountSlider;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DDIEGO_Y2K_FXAudioProcessorEditor)
};

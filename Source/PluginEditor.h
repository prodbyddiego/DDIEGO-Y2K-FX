#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <array>
#include <functional>

class DDIEGO_Y2K_FXAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit DDIEGO_Y2K_FXAudioProcessorEditor(DDIEGO_Y2K_FXAudioProcessor&);
    ~DDIEGO_Y2K_FXAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class NeonLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float,
                              float, float, juce::Slider&) override;
        juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                                  bool, bool) override;
    };

    class WaveformDisplay final : public juce::Component,
                                  public juce::FileDragAndDropTarget
    {
    public:
        explicit WaveformDisplay(DDIEGO_Y2K_FXAudioProcessor& owner) : processor(owner) {}

        void paint(juce::Graphics&) override;
        bool isInterestedInFileDrag(const juce::StringArray& files) override;
        void fileDragEnter(const juce::StringArray&, int, int) override;
        void fileDragExit(const juce::StringArray&) override;
        void filesDropped(const juce::StringArray&, int, int) override;

        std::function<void(const juce::File&)> onFileDropped;

    private:
        DDIEGO_Y2K_FXAudioProcessor& processor;
        bool isDragTarget = false;
    };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void timerCallback() override;
    void openSampleChooser();
    void loadSample(const juce::File& file);
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);
    void configureTextButton(juce::TextButton& button);
    void layoutSlider(juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area);

    DDIEGO_Y2K_FXAudioProcessor& processor;
    NeonLookAndFeel neonLookAndFeel;
    WaveformDisplay waveform;

    juce::TextButton granularButton { "GRANULARIZER" };
    juce::TextButton chopperButton { "CHOPPER" };
    juce::TextButton loadButton { "LOAD SAMPLE" };
    juce::Label statusLabel;
    juce::MidiKeyboardComponent keyboard;
    std::unique_ptr<juce::FileChooser> fileChooser;

    std::array<juce::Slider, 8> granularSliders;
    std::array<juce::Label, 8> granularLabels;
    std::array<std::unique_ptr<SliderAttachment>, 8> granularAttachments;
    std::array<juce::Slider, 6> chopperSliders;
    std::array<juce::Label, 6> chopperLabels;
    std::array<std::unique_ptr<SliderAttachment>, 6> chopperAttachments;
    juce::ComboBox divisionBox;
    juce::Label divisionLabel;
    juce::ToggleButton reverseButton { "REVERSE" };
    juce::TextButton randomizeButton { "RANDOMIZE MAP" };
    std::unique_ptr<ComboBoxAttachment> divisionAttachment;
    std::unique_ptr<ButtonAttachment> reverseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DDIEGO_Y2K_FXAudioProcessorEditor)
};

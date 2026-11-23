#include "PluginEditor.h"

DDIEGO_Y2K_FXAudioProcessorEditor::DDIEGO_Y2K_FXAudioProcessorEditor(
    DDIEGO_Y2K_FXAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 620);

    titleLabel.setText("DDIEGO // Y2K FX", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(granularButton);
    addAndMakeVisible(chopperButton);

    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 24);
    amountSlider.setRange(0.0, 1.0, 0.001);
    amountSlider.setValue(0.5);
    amountSlider.setTextValueSuffix(" amount");
    addAndMakeVisible(amountSlider);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101018));

    g.setColour(juce::Colour(0xfff2f2f2));
    g.drawRect(getLocalBounds().reduced(12), 2);

    g.setColour(juce::Colour(0xff1d1d2b));
    g.fillRoundedRectangle(28.0f, 85.0f, 844.0f, 330.0f, 18.0f);

    g.setColour(juce::Colour(0xfff2f2f2));
    for (int i = 0; i < 64; ++i)
    {
        float x = 45.0f + i * 12.5f;
        float h = 15.0f + 100.0f * std::abs(std::sin(i * 0.77f));
        g.drawLine(x, 250.0f - h, x, 250.0f + h, 2.0f);
    }

    g.setColour(juce::Colour(0xff7777ff));
    g.drawRoundedRectangle(45.0f, 120.0f, 810.0f, 260.0f, 12.0f, 2.0f);

    g.setColour(juce::Colour(0xffa0a0aa));
    g.setFont(14.0f);
    g.drawText("DROP AUDIO FILE HERE  //  WAVEFORM AREA", 45, 395, 810, 24,
               juce::Justification::centred);

    g.drawText("Y2K MODE", 45, 55, 150, 24, juce::Justification::left);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::resized()
{
    titleLabel.setBounds(45, 20, 400, 40);
    granularButton.setBounds(600, 22, 120, 36);
    chopperButton.setBounds(730, 22, 120, 36);
    amountSlider.setBounds(45, 445, 180, 140);
}

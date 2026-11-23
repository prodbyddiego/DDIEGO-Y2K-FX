/*
THESIS: A playable sample is the center of the instrument, not a decorative waveform.
OWN-WORLD: Ink-black panels, cyan signal traces, magenta slice marks, and polished metal controls.
STORY: Load one sound, shape its grains or slices, then play it from the MIDI keyboard or DAW.
FIRST VIEWPORT: Mode and load controls sit above a large waveform; the active engine fills the lower deck.
FORM: A resizable, DAW-native instrument panel; no placeholder meters or simulated sample content.
*/

#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour backgroundTop { 0xff080d18 };
const juce::Colour backgroundBottom { 0xff101529 };
const juce::Colour panelFill { 0xff111a29 };
const juce::Colour panelEdge { 0xff24384a };
const juce::Colour cyan { 0xff53f2e5 };
const juce::Colour magenta { 0xffff4fc8 };
const juce::Colour primaryText { 0xffe6f8fb };
const juce::Colour secondaryText { 0xff8daab7 };
}

void DDIEGO_Y2K_FXAudioProcessorEditor::NeonLookAndFeel::drawRotarySlider(
    juce::Graphics& graphics, int x, int y, int width, int height, float sliderPosition,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    const auto diameter = static_cast<float>(std::min(width, height)) * 0.62f;
    const auto bounds = juce::Rectangle<float>(
        x + (width - diameter) * 0.5f, y + 3.0f, diameter, diameter);
    const auto centre = bounds.getCentre();
    const auto radius = diameter * 0.5f;
    const auto angle = rotaryStartAngle + sliderPosition * (rotaryEndAngle - rotaryStartAngle);

    graphics.setColour(juce::Colour(0xff09121d));
    graphics.fillEllipse(bounds.expanded(2.0f));
    graphics.setColour(panelEdge);
    graphics.drawEllipse(bounds.expanded(2.0f), 1.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius - 5.0f, radius - 5.0f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    graphics.setColour(juce::Colour(0xff314958));
    graphics.strokePath(track, juce::PathStrokeType(3.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path activeArc;
    activeArc.addCentredArc(centre.x, centre.y, radius - 5.0f, radius - 5.0f, 0.0f,
                            rotaryStartAngle, angle, true);
    graphics.setColour(cyan);
    graphics.strokePath(activeArc, juce::PathStrokeType(3.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.startNewSubPath(centre.x, centre.y);
    pointer.lineTo(centre.x + std::sin(angle) * radius * 0.65f,
                   centre.y - std::cos(angle) * radius * 0.65f);
    graphics.setColour(primaryText);
    graphics.strokePath(pointer, juce::PathStrokeType(2.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    graphics.setColour(magenta);
    graphics.fillEllipse(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::NeonLookAndFeel::drawButtonBackground(
    juce::Graphics& graphics, juce::Button& button, const juce::Colour& background,
    bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto active = button.getToggleState();
    auto fill = active ? juce::Colour(0xff14363e) : background;
    if (down)
        fill = fill.brighter(0.18f);
    else if (highlighted)
        fill = fill.brighter(0.08f);

    graphics.setColour(fill);
    graphics.fillRoundedRectangle(bounds, 9.0f);
    graphics.setColour(active ? cyan : panelEdge);
    graphics.drawRoundedRectangle(bounds, 9.0f, active ? 1.5f : 1.0f);
}

juce::Font DDIEGO_Y2K_FXAudioProcessorEditor::NeonLookAndFeel::getTextButtonFont(
    juce::TextButton&, int)
{
    return juce::Font(11.0f, juce::Font::bold);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::WaveformDisplay::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour(0xe9091521));
    graphics.fillRoundedRectangle(bounds, 15.0f);
    graphics.setColour(isDragTarget ? magenta : panelEdge);
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 15.0f, isDragTarget ? 2.0f : 1.0f);

    auto plot = bounds.reduced(18.0f, 30.0f);
    for (int line = 1; line < 4; ++line)
    {
        const auto y = plot.getY() + plot.getHeight() * static_cast<float>(line) / 4.0f;
        graphics.setColour(juce::Colour(0xff263744));
        graphics.drawHorizontalLine(juce::roundToInt(y), plot.getX(), plot.getRight());
    }

    for (int line = 1; line < 8; ++line)
    {
        const auto x = plot.getX() + plot.getWidth() * static_cast<float>(line) / 8.0f;
        graphics.setColour(juce::Colour(0xff1a2b38));
        graphics.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
    }

    const auto* sample = processor.getLoadedSample();
    if (sample == nullptr || sample->waveform.empty())
    {
        graphics.setColour(isDragTarget ? magenta : secondaryText);
        graphics.setFont(juce::Font(15.0f, juce::Font::bold));
        graphics.drawText(isDragTarget ? "DROP TO LOAD AUDIO" : "LOAD OR DROP A SAMPLE",
                          bounds.toNearestInt().reduced(20, 0), juce::Justification::centred);
        graphics.setColour(secondaryText);
        graphics.setFont(juce::Font(11.0f));
        graphics.drawText("WAV  /  AIFF  /  FLAC  /  MP3*",
                          bounds.toNearestInt().withTrimmedTop(38), juce::Justification::centred);
        return;
    }

    const auto waveformCentre = plot.getCentreY();
    const auto amplitude = plot.getHeight() * 0.44f;
    juce::Path shape;
    const auto binCount = sample->waveform.size();
    const auto binDenominator = static_cast<float>(std::max<size_t>(1, binCount - 1));
    shape.startNewSubPath(plot.getX(), waveformCentre
        - sample->waveform.front().second * amplitude);
    for (size_t bin = 1; bin < binCount; ++bin)
    {
        const auto x = plot.getX() + plot.getWidth()
            * static_cast<float>(bin) / binDenominator;
        shape.lineTo(x, waveformCentre - sample->waveform[bin].second * amplitude);
    }
    for (size_t bin = binCount; bin-- > 0;)
    {
        const auto x = plot.getX() + plot.getWidth()
            * static_cast<float>(bin) / binDenominator;
        shape.lineTo(x, waveformCentre - sample->waveform[bin].first * amplitude);
    }
    shape.closeSubPath();

    graphics.setColour(cyan.withAlpha(0.20f));
    graphics.fillPath(shape);
    graphics.setColour(cyan);
    graphics.strokePath(shape, juce::PathStrokeType(1.0f));

    if (static_cast<int>(processor.parameters.getRawParameterValue(
            DDIEGO_Y2K_FXAudioProcessor::modeParameter)->load()) == 1)
    {
        const auto slices = juce::jmax(1, processor.getCurrentSliceCount());
        const auto startOffset = juce::jlimit(0.0f, 1.0f,
            processor.parameters.getRawParameterValue(
                DDIEGO_Y2K_FXAudioProcessor::chopOffsetParameter)->load() / 100.0f);
        const auto sliceLength = juce::jlimit(0.1f, 1.0f,
            processor.parameters.getRawParameterValue(
                DDIEGO_Y2K_FXAudioProcessor::chopLengthParameter)->load() / 100.0f);
        const auto sliceStart = plot.getX() + plot.getWidth() * startOffset;
        const auto sliceRegionWidth = plot.getWidth() * (1.0f - startOffset);
        graphics.setColour(magenta.withAlpha(0.65f));
        for (int slice = 0; slice <= slices; ++slice)
        {
            const auto x = sliceStart + sliceRegionWidth
                * static_cast<float>(slice) / static_cast<float>(slices);
            graphics.drawVerticalLine(juce::roundToInt(x), plot.getY(), plot.getBottom());
        }
        graphics.setColour(cyan.withAlpha(0.50f));
        for (int slice = 0; slice < slices; ++slice)
        {
            const auto x = sliceStart + sliceRegionWidth
                * (static_cast<float>(slice) + sliceLength) / static_cast<float>(slices);
            graphics.drawVerticalLine(juce::roundToInt(x), plot.getCentreY() - 6.0f,
                                      plot.getCentreY() + 6.0f);
        }
    }

    graphics.setColour(primaryText);
    graphics.setFont(juce::Font(12.0f, juce::Font::bold));
    graphics.drawText(sample->fileName, bounds.toNearestInt().reduced(16, 0)
        .withHeight(24), juce::Justification::centredLeft, true);
    graphics.setColour(secondaryText);
    graphics.setFont(juce::Font(10.0f));
    graphics.drawText(juce::String(sample->audio.getNumSamples()) + " SAMPLES",
                      bounds.toNearestInt().reduced(16, 0).withTrimmedTop(
                          juce::roundToInt(bounds.getHeight()) - 25),
                      juce::Justification::centredRight);
}

bool DDIEGO_Y2K_FXAudioProcessorEditor::WaveformDisplay::isInterestedInFileDrag(
    const juce::StringArray& files)
{
    return !files.isEmpty();
}

void DDIEGO_Y2K_FXAudioProcessorEditor::WaveformDisplay::fileDragEnter(
    const juce::StringArray&, int, int)
{
    isDragTarget = true;
    repaint();
}

void DDIEGO_Y2K_FXAudioProcessorEditor::WaveformDisplay::fileDragExit(
    const juce::StringArray&)
{
    isDragTarget = false;
    repaint();
}

void DDIEGO_Y2K_FXAudioProcessorEditor::WaveformDisplay::filesDropped(
    const juce::StringArray& files, int, int)
{
    isDragTarget = false;
    repaint();
    if (files.size() > 0 && onFileDropped != nullptr)
        onFileDropped(juce::File(files[0]));
}

DDIEGO_Y2K_FXAudioProcessorEditor::DDIEGO_Y2K_FXAudioProcessorEditor(
    DDIEGO_Y2K_FXAudioProcessor& owner)
    : AudioProcessorEditor(&owner),
      processor(owner),
      waveform(owner),
      keyboard(owner.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&neonLookAndFeel);
    setSize(1120, 820);
    setResizable(true, true);
    setResizeLimits(920, 740, 1600, 1200);

    configureTextButton(granularButton);
    configureTextButton(chopperButton);
    configureTextButton(loadButton);
    configureTextButton(randomizeButton);
    granularButton.setClickingTogglesState(true);
    chopperButton.setClickingTogglesState(true);
    addAndMakeVisible(granularButton);
    addAndMakeVisible(chopperButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(waveform);
    waveform.onFileDropped = [this](const juce::File& file) { loadSample(file); };

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    statusLabel.setColour(juce::Label::textColourId, secondaryText);

    const std::array<const char*, 8> granularIds {
        DDIEGO_Y2K_FXAudioProcessor::granularAmountParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainSizeParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainDensityParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainPositionParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainRandomnessParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainPitchParameter,
        DDIEGO_Y2K_FXAudioProcessor::grainTextureParameter,
        DDIEGO_Y2K_FXAudioProcessor::granularDryWetParameter
    };
    const std::array<juce::String, 8> granularNames {
        "GRANULAR AMOUNT", "GRAIN SIZE", "GRAIN DENSITY", "POSITION / SCAN",
        "RANDOMNESS", "PITCH", "TEXTURE", "DRY / WET"
    };
    for (size_t index = 0; index < granularSliders.size(); ++index)
    {
        configureSlider(granularSliders[index], granularLabels[index], granularNames[index]);
        addAndMakeVisible(granularSliders[index]);
        addAndMakeVisible(granularLabels[index]);
        granularAttachments[index] = std::make_unique<SliderAttachment>(
            processor.parameters, granularIds[index], granularSliders[index]);
    }

    const std::array<const char*, 6> chopperIds {
        DDIEGO_Y2K_FXAudioProcessor::chopOffsetParameter,
        DDIEGO_Y2K_FXAudioProcessor::chopLengthParameter,
        DDIEGO_Y2K_FXAudioProcessor::chopPitchParameter,
        DDIEGO_Y2K_FXAudioProcessor::chopAttackParameter,
        DDIEGO_Y2K_FXAudioProcessor::chopReleaseParameter,
        DDIEGO_Y2K_FXAudioProcessor::chopDryWetParameter
    };
    const std::array<juce::String, 6> chopperNames {
        "START OFFSET", "SLICE LENGTH", "PITCH", "ATTACK", "RELEASE", "DRY / WET"
    };
    for (size_t index = 0; index < chopperSliders.size(); ++index)
    {
        configureSlider(chopperSliders[index], chopperLabels[index], chopperNames[index]);
        addAndMakeVisible(chopperSliders[index]);
        addAndMakeVisible(chopperLabels[index]);
        chopperAttachments[index] = std::make_unique<SliderAttachment>(
            processor.parameters, chopperIds[index], chopperSliders[index]);
    }

    divisionLabel.setText("SLICE DIVISION", juce::dontSendNotification);
    divisionLabel.setJustificationType(juce::Justification::centred);
    divisionLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    divisionLabel.setColour(juce::Label::textColourId, secondaryText);
    addAndMakeVisible(divisionLabel);
    divisionBox.addItem("1/4", 1);
    divisionBox.addItem("1/8", 2);
    divisionBox.addItem("RANDOM", 3);
    divisionBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0a131e));
    divisionBox.setColour(juce::ComboBox::outlineColourId, panelEdge);
    divisionBox.setColour(juce::ComboBox::textColourId, primaryText);
    divisionBox.setColour(juce::ComboBox::arrowColourId, cyan);
    addAndMakeVisible(divisionBox);
    divisionAttachment = std::make_unique<ComboBoxAttachment>(
        processor.parameters, DDIEGO_Y2K_FXAudioProcessor::chopDivisionParameter, divisionBox);

    reverseButton.setColour(juce::ToggleButton::textColourId, primaryText);
    reverseButton.setColour(juce::ToggleButton::tickColourId, magenta);
    reverseButton.setColour(juce::ToggleButton::tickDisabledColourId, panelEdge);
    addAndMakeVisible(reverseButton);
    reverseAttachment = std::make_unique<ButtonAttachment>(
        processor.parameters, DDIEGO_Y2K_FXAudioProcessor::chopReverseParameter, reverseButton);
    addAndMakeVisible(randomizeButton);
    randomizeButton.onClick = [this] { processor.requestMappingRandomization(); };

    keyboard.setAvailableRange(24, 47);
    keyboard.setScrollButtonsVisible(false);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffd8e9ed));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff101824));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, panelEdge);
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, cyan.withAlpha(0.25f));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, magenta.withAlpha(0.38f));
    keyboard.setColour(juce::MidiKeyboardComponent::textLabelColourId, secondaryText);
    addAndMakeVisible(keyboard);

    granularButton.onClick = [this]
    {
        if (auto* mode = processor.parameters.getParameter(
                DDIEGO_Y2K_FXAudioProcessor::modeParameter))
        {
            mode->beginChangeGesture();
            mode->setValueNotifyingHost(0.0f);
            mode->endChangeGesture();
        }
    };
    chopperButton.onClick = [this]
    {
        if (auto* mode = processor.parameters.getParameter(
                DDIEGO_Y2K_FXAudioProcessor::modeParameter))
        {
            mode->beginChangeGesture();
            mode->setValueNotifyingHost(1.0f);
            mode->endChangeGesture();
        }
    };
    loadButton.onClick = [this] { openSampleChooser(); };

    startTimerHz(12);
    timerCallback();
}

DDIEGO_Y2K_FXAudioProcessorEditor::~DDIEGO_Y2K_FXAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::configureTextButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff101c2a));
    button.setColour(juce::TextButton::textColourOffId, primaryText);
    button.setColour(juce::TextButton::textColourOnId, cyan);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff14363e));
}

void DDIEGO_Y2K_FXAudioProcessorEditor::configureSlider(
    juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
    slider.setColour(juce::Slider::textBoxTextColourId, primaryText);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff09131e));
    slider.setColour(juce::Slider::textBoxOutlineColourId, panelEdge);
    slider.setName(name);
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, secondaryText);
}

void DDIEGO_Y2K_FXAudioProcessorEditor::layoutSlider(
    juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
{
    label.setBounds(area.removeFromTop(18));
    slider.setBounds(area.reduced(4, 0));
}

void DDIEGO_Y2K_FXAudioProcessorEditor::openSampleChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Load an audio sample", juce::File {}, "*.wav;*.aif;*.aiff;*.flac;*.mp3");
    juce::Component::SafePointer<DDIEGO_Y2K_FXAudioProcessorEditor> safeThis(this);
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
        [safeThis](const juce::FileChooser& chooser)
        {
            if (safeThis != nullptr)
            {
                const auto file = chooser.getResult();
                if (file.existsAsFile())
                    safeThis->loadSample(file);
            }
        });
}

void DDIEGO_Y2K_FXAudioProcessorEditor::loadSample(const juce::File& file)
{
    if (processor.loadSampleFromFile(file))
        statusLabel.setColour(juce::Label::textColourId, cyan);
    else
        statusLabel.setColour(juce::Label::textColourId, magenta);

    statusLabel.setText(processor.getSampleLoadError().isNotEmpty()
                            ? processor.getSampleLoadError()
                            : "SAMPLE READY  //  " + file.getFileName().toUpperCase(),
                        juce::dontSendNotification);
    waveform.repaint();
}

void DDIEGO_Y2K_FXAudioProcessorEditor::timerCallback()
{
    processor.collectUnusedSamples();
    const auto isGranular = processor.parameters.getRawParameterValue(
        DDIEGO_Y2K_FXAudioProcessor::modeParameter)->load() < 0.5f;
    granularButton.setToggleState(isGranular, juce::dontSendNotification);
    chopperButton.setToggleState(!isGranular, juce::dontSendNotification);

    for (auto& slider : granularSliders)
        slider.setVisible(isGranular);
    for (auto& label : granularLabels)
        label.setVisible(isGranular);
    for (auto& slider : chopperSliders)
        slider.setVisible(!isGranular);
    for (auto& label : chopperLabels)
        label.setVisible(!isGranular);
    divisionBox.setVisible(!isGranular);
    divisionLabel.setVisible(!isGranular);
    reverseButton.setVisible(!isGranular);
    randomizeButton.setVisible(!isGranular);
    keyboard.setVisible(!isGranular);

    const auto error = processor.getSampleLoadError();
    if (error.isNotEmpty())
    {
        statusLabel.setText(error, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, magenta);
    }
    else if (const auto* sample = processor.getLoadedSample())
    {
        statusLabel.setText(sample->fileName.toUpperCase(), juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, cyan);
    }
    else
    {
        statusLabel.setText("NO SAMPLE LOADED", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, secondaryText);
    }

    waveform.repaint();
    repaint();
}

void DDIEGO_Y2K_FXAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    juce::ColourGradient background(backgroundTop, 0.0f, 0.0f,
                                    backgroundBottom, 0.0f,
                                    static_cast<float>(getHeight()), false);
    graphics.setGradientFill(background);
    graphics.fillAll();

    graphics.setColour(juce::Colour(0x1625bfd0));
    for (int x = 14; x < getWidth(); x += 48)
        graphics.drawVerticalLine(x, 82.0f, static_cast<float>(getHeight() - 12));
    for (int y = 94; y < getHeight(); y += 48)
        graphics.drawHorizontalLine(y, 14.0f, static_cast<float>(getWidth() - 14));

    graphics.setColour(cyan);
    graphics.setFont(juce::Font(23.0f, juce::Font::bold));
    graphics.drawText("DDIEGO", 25, 16, 152, 26, juce::Justification::left);
    graphics.setColour(primaryText);
    graphics.setFont(juce::Font(13.0f, juce::Font::bold));
    graphics.drawText("Y2K FX", 27, 44, 120, 19, juce::Justification::left);
    graphics.setColour(magenta);
    graphics.fillRoundedRectangle(151.0f, 22.0f, 2.0f, 37.0f, 1.0f);

    graphics.setColour(panelFill);
    const auto granular = processor.parameters.getRawParameterValue(
        DDIEGO_Y2K_FXAudioProcessor::modeParameter)->load() < 0.5f;
    const auto controlsY = 374;
    const auto controlsHeight = granular ? getHeight() - controlsY - 22
                                         : getHeight() - controlsY - 116;
    graphics.fillRoundedRectangle(juce::Rectangle<float>(
        22.0f, static_cast<float>(controlsY), static_cast<float>(getWidth() - 44),
        static_cast<float>(controlsHeight)), 15.0f);
    graphics.setColour(panelEdge);
    graphics.drawRoundedRectangle(juce::Rectangle<float>(
        22.0f, static_cast<float>(controlsY), static_cast<float>(getWidth() - 44),
        static_cast<float>(controlsHeight)), 15.0f, 1.0f);

    graphics.setColour(cyan.withAlpha(0.72f));
    graphics.setFont(juce::Font(10.0f, juce::Font::bold));
    graphics.drawText(granular ? "GRANULAR ENGINE" : "CHOPPER ENGINE",
                      39, controlsY + 9, 250, 18, juce::Justification::left);

    if (!granular)
    {
        const auto keyboardY = getHeight() - 99;
        graphics.setColour(panelFill);
        graphics.fillRoundedRectangle(22.0f, static_cast<float>(keyboardY),
                                      static_cast<float>(getWidth() - 44), 82.0f, 12.0f);
        graphics.setColour(panelEdge);
        graphics.drawRoundedRectangle(22.0f, static_cast<float>(keyboardY),
                                      static_cast<float>(getWidth() - 44), 82.0f, 12.0f, 1.0f);
        graphics.setColour(secondaryText);
        graphics.setFont(juce::Font(9.0f, juce::Font::bold));
        graphics.drawText("MIDI  //  C1-B2", getWidth() - 168, keyboardY + 4, 126, 14,
                          juce::Justification::centredRight);
    }
}

void DDIEGO_Y2K_FXAudioProcessorEditor::resized()
{
    const auto width = getWidth();
    const auto height = getHeight();
    granularButton.setBounds(width - 296, 22, 126, 38);
    chopperButton.setBounds(width - 158, 22, 132, 38);
    loadButton.setBounds(width - 423, 22, 112, 38);
    statusLabel.setBounds(324, 28, width - 762, 28);
    waveform.setBounds(22, 88, width - 44, 270);

    const auto controlsY = 374;
    const auto controlsBottom = height - 23;
    const auto granularArea = juce::Rectangle<int>(
        38, controlsY + 34, width - 76, controlsBottom - controlsY - 45);
    const auto columnWidth = granularArea.getWidth() / 4;
    const auto rowHeight = granularArea.getHeight() / 2;
    for (size_t index = 0; index < granularSliders.size(); ++index)
    {
        const auto row = static_cast<int>(index / 4);
        const auto column = static_cast<int>(index % 4);
        const juce::Rectangle<int> cell(
            granularArea.getX() + column * columnWidth,
            granularArea.getY() + row * rowHeight,
            columnWidth, rowHeight);
        layoutSlider(granularSliders[index], granularLabels[index], cell);
    }

    const auto chopperBottom = height - 121;
    const auto chopperArea = juce::Rectangle<int>(
        38, controlsY + 34, width - 76, chopperBottom - controlsY - 42);
    const auto chopperCellWidth = chopperArea.getWidth() / 5;
    const auto chopperRowHeight = chopperArea.getHeight() / 2;
    const auto cell = [&](int row, int column)
    {
        return juce::Rectangle<int>(chopperArea.getX() + column * chopperCellWidth,
            chopperArea.getY() + row * chopperRowHeight,
            chopperCellWidth, chopperRowHeight);
    };

    layoutSlider(chopperSliders[0], chopperLabels[0], cell(0, 1));
    layoutSlider(chopperSliders[1], chopperLabels[1], cell(0, 2));
    layoutSlider(chopperSliders[2], chopperLabels[2], cell(1, 0));
    layoutSlider(chopperSliders[3], chopperLabels[3], cell(1, 1));
    layoutSlider(chopperSliders[4], chopperLabels[4], cell(1, 2));
    layoutSlider(chopperSliders[5], chopperLabels[5], cell(1, 3));

    divisionLabel.setBounds(cell(0, 0).withHeight(18));
    divisionBox.setBounds(cell(0, 0).withTrimmedTop(26).reduced(8, 8));
    reverseButton.setBounds(cell(0, 3).reduced(8, 14));
    randomizeButton.setBounds(cell(0, 4).reduced(5, 14));
    keyboard.setKeyWidth(static_cast<float>(width - 68) / 24.0f);
    keyboard.setBounds(34, height - 84, width - 68, 61);
}

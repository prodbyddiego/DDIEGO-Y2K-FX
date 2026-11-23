#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <limits>
#include <new>

namespace
{
juce::AudioParameterFloatAttributes percentAttributes()
{
    return juce::AudioParameterFloatAttributes().withLabel("%");
}
}

DDIEGO_Y2K_FXAudioProcessor::DDIEGO_Y2K_FXAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    audioFormats.registerBasicFormats();
}

DDIEGO_Y2K_FXAudioProcessor::ParameterLayout
DDIEGO_Y2K_FXAudioProcessor::createParameterLayout()
{
    ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { modeParameter, 1 }, "Mode",
        juce::StringArray { "GRANULARIZER", "CHOPPER" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { granularAmountParameter, 1 }, "Granular Amount",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainSizeParameter, 1 }, "Grain Size",
        juce::NormalisableRange<float> { 10.0f, 500.0f, 1.0f, 0.35f }, 120.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainDensityParameter, 1 }, "Grain Density",
        juce::NormalisableRange<float> { 1.0f, 60.0f, 0.1f }, 12.0f,
        juce::AudioParameterFloatAttributes().withLabel("gr/s")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainPositionParameter, 1 }, "Position / Scan",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 50.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainRandomnessParameter, 1 }, "Randomness",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 15.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainPitchParameter, 1 }, "Pitch",
        juce::NormalisableRange<float> { -24.0f, 24.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { grainTextureParameter, 1 }, "Texture",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 35.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { granularDryWetParameter, 1 }, "Granular Dry / Wet",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 100.0f, percentAttributes()));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { chopDivisionParameter, 1 }, "Slice Division",
        juce::StringArray { "1/4", "1/8", "RANDOM" }, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopOffsetParameter, 1 }, "Start Offset",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopLengthParameter, 1 }, "Slice Length",
        juce::NormalisableRange<float> { 10.0f, 100.0f, 0.1f }, 100.0f, percentAttributes()));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { chopReverseParameter, 1 }, "Reverse", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopPitchParameter, 1 }, "Pitch",
        juce::NormalisableRange<float> { -24.0f, 24.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopAttackParameter, 1 }, "Attack",
        juce::NormalisableRange<float> { 0.0f, 500.0f, 0.1f }, 5.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopReleaseParameter, 1 }, "Release",
        juce::NormalisableRange<float> { 1.0f, 2000.0f, 1.0f, 0.35f }, 200.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { chopDryWetParameter, 1 }, "Chopper Dry / Wet",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 100.0f, percentAttributes()));

    return layout;
}

void DDIEGO_Y2K_FXAudioProcessor::prepareToPlay(double sampleRate, int)
{
    granular.prepare(sampleRate);
    chopper.prepare(sampleRate);
}

void DDIEGO_Y2K_FXAudioProcessor::releaseResources()
{
}

bool DDIEGO_Y2K_FXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo())
        && input == output;
}

void DDIEGO_Y2K_FXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    activeAudioCallbacks.fetch_add(1, std::memory_order_seq_cst);
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    const auto* sample = activeSample.load(std::memory_order_seq_cst);
    const auto read = [this](const char* id)
    {
        return parameters.getRawParameterValue(id)->load(std::memory_order_relaxed);
    };

    if (static_cast<int>(read(modeParameter)) == 0)
    {
        const GranularEngine::Parameters settings {
            read(granularAmountParameter) / 100.0f,
            read(grainSizeParameter),
            read(grainDensityParameter),
            read(grainPositionParameter) / 100.0f,
            read(grainRandomnessParameter) / 100.0f,
            read(grainPitchParameter),
            read(grainTextureParameter) / 100.0f,
            read(granularDryWetParameter) / 100.0f
        };
        granular.process(buffer, sample, settings);
    }
    else
    {
        const ChopperEngine::Parameters settings {
            static_cast<int>(read(chopDivisionParameter)),
            read(chopOffsetParameter) / 100.0f,
            read(chopLengthParameter) / 100.0f,
            read(chopReverseParameter) >= 0.5f,
            read(chopPitchParameter),
            read(chopAttackParameter),
            read(chopReleaseParameter),
            read(chopDryWetParameter) / 100.0f
        };
        chopper.process(buffer, midiMessages, sample, settings);
    }

    activeAudioCallbacks.fetch_sub(1, std::memory_order_seq_cst);
}

juce::AudioProcessorEditor* DDIEGO_Y2K_FXAudioProcessor::createEditor()
{
    return new DDIEGO_Y2K_FXAudioProcessorEditor(*this);
}

bool DDIEGO_Y2K_FXAudioProcessor::loadSampleFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "The selected audio file no longer exists.";
        return false;
    }

    std::unique_ptr<juce::AudioFormatReader> reader;
    try
    {
        reader.reset(audioFormats.createReaderFor(file));
    }
    catch (const std::bad_alloc&)
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "There is not enough memory to open this audio file.";
        return false;
    }
    if (reader == nullptr)
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "JUCE could not decode this audio file. Try WAV, AIFF, FLAC, or MP3.";
        return false;
    }

    const auto destinationChannels = static_cast<int>(juce::jlimit(
        1u, 2u, reader->numChannels));
    const auto frameCount = reader->lengthInSamples;
    constexpr juce::int64 maxDecodedFloats = 128LL * 1024LL * 1024LL;
    if (frameCount <= 0 || frameCount > std::numeric_limits<int>::max()
        || frameCount > maxDecodedFloats / destinationChannels)
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "This sample is empty or too large to load (maximum decoded size: 512 MB).";
        return false;
    }

    std::unique_ptr<SampleData> asset;
    try
    {
        asset = std::make_unique<SampleData>();
        asset->audio.setSize(destinationChannels, static_cast<int>(frameCount), false, true, false);
        if (!reader->read(&asset->audio, 0, static_cast<int>(frameCount), 0, true, true))
        {
            const juce::ScopedLock lock(sampleErrorLock);
            sampleLoadError = "The audio file could not be decoded completely.";
            return false;
        }

        asset->sampleRate = reader->sampleRate;
        asset->filePath = file.getFullPathName();
        asset->fileName = file.getFileName();

        constexpr size_t waveformBins = 4096;
        const auto numFrames = asset->audio.getNumSamples();
        const auto numBins = static_cast<size_t>(std::min(numFrames, static_cast<int>(waveformBins)));
        asset->waveform.resize(numBins);
        for (size_t bin = 0; bin < numBins; ++bin)
        {
            const auto start = static_cast<int>(
                static_cast<double>(bin) * numFrames / static_cast<double>(numBins));
            const auto end = std::max(start + 1, static_cast<int>(
                static_cast<double>(bin + 1) * numFrames / static_cast<double>(numBins)));
            auto minimum = 1.0f;
            auto maximum = -1.0f;
            for (int channel = 0; channel < destinationChannels; ++channel)
            {
                const auto* samples = asset->audio.getReadPointer(channel);
                for (auto frame = start; frame < end; ++frame)
                {
                    minimum = std::min(minimum, samples[frame]);
                    maximum = std::max(maximum, samples[frame]);
                }
            }
            asset->waveform[bin] = { minimum, maximum };
        }
    }
    catch (const std::bad_alloc&)
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "There is not enough memory to load this audio file.";
        return false;
    }

    const auto* publishedAsset = asset.get();
    try
    {
        const juce::ScopedLock lock(sampleAssetsLock);
        sampleAssets.push_back(std::move(asset));
        activeSample.store(publishedAsset, std::memory_order_seq_cst);
    }
    catch (const std::bad_alloc&)
    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError = "There is not enough memory to keep this sample loaded.";
        return false;
    }

    {
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError.clear();
        savedSamplePath = publishedAsset->filePath;
    }
    return true;
}

juce::String DDIEGO_Y2K_FXAudioProcessor::getSampleLoadError() const
{
    const juce::ScopedLock lock(sampleErrorLock);
    return sampleLoadError;
}

void DDIEGO_Y2K_FXAudioProcessor::collectUnusedSamples()
{
    auto* messageManager = juce::MessageManager::getInstance();
    if (messageManager == nullptr || !messageManager->isThisTheMessageThread()
        || activeAudioCallbacks.load(std::memory_order_seq_cst) != 0)
        return;

    const juce::ScopedLock lock(sampleAssetsLock);
    if (activeAudioCallbacks.load(std::memory_order_seq_cst) != 0)
        return;

    const auto* current = activeSample.load(std::memory_order_seq_cst);
    sampleAssets.erase(std::remove_if(sampleAssets.begin(), sampleAssets.end(),
        [current](const auto& asset) { return asset.get() != current; }), sampleAssets.end());
}

void DDIEGO_Y2K_FXAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    juce::String samplePath;
    {
        const juce::ScopedLock lock(sampleErrorLock);
        samplePath = savedSamplePath;
    }

    if (samplePath.isNotEmpty())
        state.setProperty("samplePath", samplePath, nullptr);
    else
        state.removeProperty("samplePath", nullptr);

    if (const auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void DDIEGO_Y2K_FXAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    const auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || !xml->hasTagName(parameters.state.getType().toString()))
        return;

    auto restoredState = juce::ValueTree::fromXml(*xml);
    const auto samplePath = restoredState.getProperty("samplePath").toString();
    parameters.replaceState(restoredState);

    if (samplePath.isNotEmpty())
    {
        activeSample.store(nullptr, std::memory_order_seq_cst);
        {
            const juce::ScopedLock lock(sampleErrorLock);
            savedSamplePath = samplePath;
        }
        loadSampleFromFile(juce::File(samplePath));
    }
    else
    {
        activeSample.store(nullptr, std::memory_order_seq_cst);
        const juce::ScopedLock lock(sampleErrorLock);
        sampleLoadError.clear();
        savedSamplePath.clear();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DDIEGO_Y2K_FXAudioProcessor();
}

#pragma once

#include <JuceHeader.h>
#include "ChopperEngine.h"
#include "GranularEngine.h"
#include "SampleData.h"

#include <atomic>
#include <memory>
#include <vector>

class DDIEGO_Y2K_FXAudioProcessor : public juce::AudioProcessor
{
public:
    using ParameterLayout = juce::AudioProcessorValueTreeState::ParameterLayout;

    static constexpr const char* modeParameter = "mode";
    static constexpr const char* granularAmountParameter = "granAmount";
    static constexpr const char* grainSizeParameter = "grainSize";
    static constexpr const char* grainDensityParameter = "grainDensity";
    static constexpr const char* grainPositionParameter = "grainPosition";
    static constexpr const char* grainRandomnessParameter = "grainRandomness";
    static constexpr const char* grainPitchParameter = "grainPitch";
    static constexpr const char* grainTextureParameter = "grainTexture";
    static constexpr const char* granularDryWetParameter = "granDryWet";
    static constexpr const char* chopDivisionParameter = "chopDivision";
    static constexpr const char* chopOffsetParameter = "chopOffset";
    static constexpr const char* chopLengthParameter = "chopLength";
    static constexpr const char* chopReverseParameter = "chopReverse";
    static constexpr const char* chopPitchParameter = "chopPitch";
    static constexpr const char* chopAttackParameter = "chopAttack";
    static constexpr const char* chopReleaseParameter = "chopRelease";
    static constexpr const char* chopDryWetParameter = "chopDryWet";

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
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool loadSampleFromFile(const juce::File& file);
    juce::String getSampleLoadError() const;
    void collectUnusedSamples();
    const SampleData* getLoadedSample() const noexcept
    {
        return activeSample.load(std::memory_order_acquire);
    }

    void requestMappingRandomization() noexcept { chopper.requestRandomize(); }
    int getCurrentSliceCount() const noexcept { return chopper.getSliceCount(); }
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    juce::AudioProcessorValueTreeState parameters;

private:
    static ParameterLayout createParameterLayout();

    GranularEngine granular;
    ChopperEngine chopper;
    juce::AudioFormatManager audioFormats;
    juce::MidiKeyboardState keyboardState;
    std::atomic<const SampleData*> activeSample { nullptr };
    std::atomic<int> activeAudioCallbacks { 0 };
    std::vector<std::unique_ptr<SampleData>> sampleAssets;
    mutable juce::CriticalSection sampleAssetsLock;
    mutable juce::CriticalSection sampleErrorLock;
    juce::String sampleLoadError;
    juce::String savedSamplePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DDIEGO_Y2K_FXAudioProcessor)
};

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

//==============================================================================
/**
    Tape Saturation
    Signal chain:
      dry tap -> wow/flutter (modulated delay)
              -> 4x oversampling [ pre-emphasis -> biased tanh saturation -> de-emphasis ]
              -> DC block -> head bump -> tone (HF roll-off) -> hiss
              -> output gain -> dry/wet mix
*/
class TapeSaturationAudioProcessor : public juce::AudioProcessor
{
public:
    TapeSaturationAudioProcessor();
    ~TapeSaturationAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

private:
    using Filter     = juce::dsp::IIR::Filter<float>;
    using Coeffs     = juce::dsp::IIR::Coefficients<float>;
    using Duplicator = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;

    static constexpr int osFactorLog2 = 2;               // 4x oversampling
    static constexpr int osFactor     = 1 << osFactorLog2;

    void processChunk (juce::dsp::AudioBlock<float> block);
    void updateFilters();

    // parameters
    std::atomic<float>* driveParam   = nullptr;
    std::atomic<float>* biasParam    = nullptr;
    std::atomic<float>* toneParam    = nullptr;
    std::atomic<float>* bumpParam    = nullptr;
    std::atomic<float>* wowParam     = nullptr;
    std::atomic<float>* flutterParam = nullptr;
    std::atomic<float>* hissParam    = nullptr;
    std::atomic<float>* mixParam     = nullptr;
    std::atomic<float>* outputParam  = nullptr;

    // DSP
    juce::dsp::Oversampling<float> oversampling;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> wowDelay { 4096 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None>        dryDelay { 8192 };

    Duplicator preEmphasis, deEmphasis;          // run at oversampled rate
    Duplicator dcBlock, headBump, toneFilter;    // run at base rate

    juce::SmoothedValue<float> driveSmooth, biasSmooth;            // oversampled rate
    juce::SmoothedValue<float> wowSmooth, flutterSmooth;           // base rate
    juce::SmoothedValue<float> outputSmooth, mixSmooth;            // base rate

    juce::AudioBuffer<float> dryBuffer;
    std::vector<float> modBuf, gainBuf, mixBuf;
    std::vector<float> driveBuf, biasBuf, compBuf;

    juce::Random rng;
    float hissPrev[2] { 0.0f, 0.0f };

    double sampleRateHz   = 44100.0;
    int    maxBlockSize   = 512;
    int    centreDelay    = 144;
    float  lastTone       = -1.0f;
    float  lastBump       = -1000.0f;

    float wowPhase = 0.0f, flutterPhase = 0.0f, flutter2Phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeSaturationAudioProcessor)
};

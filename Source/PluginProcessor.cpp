#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
namespace
{
    juce::String percentText (float v, int)  { return juce::String (juce::roundToInt (v)) + " %"; }
    juce::String dbText      (float v, int)  { return juce::String (v, 1) + " dB"; }
    juce::String hzText      (float v, int)
    {
        if (v >= 1000.0f)
            return juce::String (v / 1000.0f, 1) + " kHz";
        return juce::String (juce::roundToInt (v)) + " Hz";
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TapeSaturationAudioProcessor::createParameterLayout()
{
    using juce::AudioParameterFloat;
    using juce::AudioParameterFloatAttributes;
    using juce::NormalisableRange;
    using juce::ParameterID;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "drive", 1 }, "Drive",
        NormalisableRange<float> (0.0f, 30.0f, 0.1f), 8.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "bias", 1 }, "Bias",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (percentText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "tone", 1 }, "Tone",
        NormalisableRange<float> (1000.0f, 20000.0f, 1.0f, 0.4f), 12000.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (hzText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "bump", 1 }, "Head Bump",
        NormalisableRange<float> (0.0f, 9.0f, 0.1f), 2.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "wow", 1 }, "Wow",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 10.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (percentText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "flutter", 1 }, "Flutter",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 10.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (percentText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "hiss", 1 }, "Hiss",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (percentText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (percentText)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "output", 1 }, "Output",
        NormalisableRange<float> (-18.0f, 12.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction (dbText)));

    return layout;
}

//==============================================================================
TapeSaturationAudioProcessor::TapeSaturationAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      oversampling (2, (size_t) osFactorLog2,
                    juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
                    true, true)
{
    driveParam   = apvts.getRawParameterValue ("drive");
    biasParam    = apvts.getRawParameterValue ("bias");
    toneParam    = apvts.getRawParameterValue ("tone");
    bumpParam    = apvts.getRawParameterValue ("bump");
    wowParam     = apvts.getRawParameterValue ("wow");
    flutterParam = apvts.getRawParameterValue ("flutter");
    hissParam    = apvts.getRawParameterValue ("hiss");
    mixParam     = apvts.getRawParameterValue ("mix");
    outputParam  = apvts.getRawParameterValue ("output");
}

//==============================================================================
bool TapeSaturationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

//==============================================================================
void TapeSaturationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRateHz = sampleRate;
    maxBlockSize = juce::jmax (1, samplesPerBlock);

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };
    const juce::dsp::ProcessSpec osSpec { sampleRate * osFactor,
                                          (juce::uint32) (maxBlockSize * osFactor), 2 };

    // --- oversampling
    oversampling.initProcessing ((size_t) maxBlockSize);
    oversampling.reset();

    // --- latency: oversampling filters + fixed centre delay of the wow/flutter line
    centreDelay = juce::jmax (8, juce::roundToInt (sampleRate * 0.003));
    const int osLatency    = juce::roundToInt (oversampling.getLatencyInSamples());
    const int totalLatency = osLatency + centreDelay;
    setLatencySamples (totalLatency);

    // --- delay lines
    wowDelay.setMaximumDelayInSamples (juce::roundToInt (sampleRate * 0.01) + 8);
    wowDelay.prepare (spec);
    wowDelay.reset();

    dryDelay.setMaximumDelayInSamples (totalLatency + 16);
    dryDelay.prepare (spec);
    dryDelay.setDelay ((float) totalLatency);
    dryDelay.reset();

    // --- filters
    preEmphasis.prepare (osSpec);
    deEmphasis.prepare (osSpec);
    dcBlock.prepare (spec);
    headBump.prepare (spec);
    toneFilter.prepare (spec);

    const double osRate = sampleRate * osFactor;
    *preEmphasis.state = *Coeffs::makeHighShelf (osRate, 3000.0f, 0.707f, juce::Decibels::decibelsToGain (6.0f));
    *deEmphasis.state  = *Coeffs::makeHighShelf (osRate, 3000.0f, 0.707f, juce::Decibels::decibelsToGain (-6.0f));
    *dcBlock.state     = *Coeffs::makeHighPass  (sampleRate, 20.0f, 0.707f);

    preEmphasis.reset();
    deEmphasis.reset();
    dcBlock.reset();
    headBump.reset();
    toneFilter.reset();

    lastTone = -1.0f;
    lastBump = -1000.0f;
    updateFilters();

    // --- buffers
    dryBuffer.setSize (2, maxBlockSize);
    modBuf.assign   ((size_t) maxBlockSize, 0.0f);
    gainBuf.assign  ((size_t) maxBlockSize, 1.0f);
    mixBuf.assign   ((size_t) maxBlockSize, 1.0f);
    driveBuf.assign ((size_t) (maxBlockSize * osFactor), 1.0f);
    biasBuf.assign  ((size_t) (maxBlockSize * osFactor), 0.0f);
    compBuf.assign  ((size_t) (maxBlockSize * osFactor), 1.0f);

    // --- smoothing
    driveSmooth.reset  (osRate, 0.05);
    biasSmooth.reset   (osRate, 0.05);
    wowSmooth.reset    (sampleRate, 0.1);
    flutterSmooth.reset(sampleRate, 0.1);
    outputSmooth.reset (sampleRate, 0.05);
    mixSmooth.reset    (sampleRate, 0.05);

    driveSmooth.setCurrentAndTargetValue   (juce::Decibels::decibelsToGain (driveParam->load()));
    biasSmooth.setCurrentAndTargetValue    (biasParam->load() * 0.01f * 0.6f);
    wowSmooth.setCurrentAndTargetValue     (wowParam->load() * 0.01f);
    flutterSmooth.setCurrentAndTargetValue (flutterParam->load() * 0.01f);
    outputSmooth.setCurrentAndTargetValue  (juce::Decibels::decibelsToGain (outputParam->load()));
    mixSmooth.setCurrentAndTargetValue     (mixParam->load() * 0.01f);

    hissPrev[0] = hissPrev[1] = 0.0f;
    wowPhase = flutterPhase = flutter2Phase = 0.0f;
}

//==============================================================================
void TapeSaturationAudioProcessor::updateFilters()
{
    const float tone = juce::jmin (toneParam->load(), (float) (sampleRateHz * 0.45));
    const float bump = bumpParam->load();

    if (tone != lastTone)
    {
        *toneFilter.state = *Coeffs::makeLowPass (sampleRateHz, tone, 0.707f);
        lastTone = tone;
    }

    if (bump != lastBump)
    {
        // "head bump": broad low-frequency resonance typical for tape heads
        *headBump.state = *Coeffs::makePeakFilter (sampleRateHz, 60.0f, 0.9f,
                                                   juce::Decibels::decibelsToGain (bump));
        lastBump = bump;
    }
}

//==============================================================================
void TapeSaturationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalSamples = buffer.getNumSamples();
    const int numCh        = juce::jmin (buffer.getNumChannels(), 2);

    for (int ch = getTotalNumInputChannels(); ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, totalSamples);

    if (numCh == 0 || totalSamples == 0)
        return;

    juce::dsp::AudioBlock<float> fullBlock (buffer.getArrayOfWritePointers(), (size_t) numCh, (size_t) totalSamples);

    // The host may deliver blocks larger than what we prepared for -> split them
    int start = 0;
    while (start < totalSamples)
    {
        const int len = juce::jmin (maxBlockSize, totalSamples - start);
        processChunk (fullBlock.getSubBlock ((size_t) start, (size_t) len));
        start += len;
    }
}

//==============================================================================
void TapeSaturationAudioProcessor::processChunk (juce::dsp::AudioBlock<float> block)
{
    const int n   = (int) block.getNumSamples();
    const int chs = (int) block.getNumChannels();
    const float sr = (float) sampleRateHz;

    // ---- parameters
    driveSmooth.setTargetValue   (juce::Decibels::decibelsToGain (driveParam->load()));
    biasSmooth.setTargetValue    (biasParam->load() * 0.01f * 0.6f);
    wowSmooth.setTargetValue     (wowParam->load() * 0.01f);
    flutterSmooth.setTargetValue (flutterParam->load() * 0.01f);
    outputSmooth.setTargetValue  (juce::Decibels::decibelsToGain (outputParam->load()));
    mixSmooth.setTargetValue     (mixParam->load() * 0.01f);

    const float hiss     = hissParam->load() * 0.01f;
    const float hissGain = hiss > 0.0f ? juce::Decibels::decibelsToGain (juce::jmap (hiss, -90.0f, -45.0f)) : 0.0f;

    updateFilters();

    // ---- 1. latency-compensated dry signal
    for (int ch = 0; ch < chs; ++ch)
    {
        auto* in  = block.getChannelPointer ((size_t) ch);
        auto* dry = dryBuffer.getWritePointer (ch);

        for (int i = 0; i < n; ++i)
        {
            dryDelay.pushSample (ch, in[i]);
            dry[i] = dryDelay.popSample (ch);
        }
    }

    // ---- 2. wow & flutter (modulated delay line)
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float wowInc  = twoPi * 0.55f / sr;
    const float fl1Inc  = twoPi * 6.3f  / sr;
    const float fl2Inc  = twoPi * 11.7f / sr;

    for (int i = 0; i < n; ++i)
    {
        const float wowAmt = wowSmooth.getNextValue();
        const float fltAmt = flutterSmooth.getNextValue();

        const float wowMod = std::sin (wowPhase);
        const float fltMod = 0.7f * std::sin (flutterPhase) + 0.3f * std::sin (flutter2Phase);

        wowPhase      += wowInc; if (wowPhase      > twoPi) wowPhase      -= twoPi;
        flutterPhase  += fl1Inc; if (flutterPhase  > twoPi) flutterPhase  -= twoPi;
        flutter2Phase += fl2Inc; if (flutter2Phase > twoPi) flutter2Phase -= twoPi;

        // up to ~1.2 ms of wow and ~0.25 ms of flutter (centre delay is 3 ms)
        modBuf[(size_t) i] = (float) centreDelay
                           + wowAmt * 0.0012f * sr * wowMod
                           + fltAmt * 0.00025f * sr * fltMod;

        gainBuf[(size_t) i] = outputSmooth.getNextValue();
        mixBuf[(size_t) i]  = mixSmooth.getNextValue();
    }

    for (int ch = 0; ch < chs; ++ch)
    {
        auto* data = block.getChannelPointer ((size_t) ch);

        for (int i = 0; i < n; ++i)
        {
            wowDelay.pushSample (ch, data[i]);
            data[i] = wowDelay.popSample (ch, modBuf[(size_t) i]);
        }
    }

    // ---- 3. oversampled saturation
    auto osBlock = oversampling.processSamplesUp (block);
    const int osN = (int) osBlock.getNumSamples();

    for (int i = 0; i < osN; ++i)
    {
        const float g = driveSmooth.getNextValue();
        driveBuf[(size_t) i] = g;
        biasBuf[(size_t) i]  = biasSmooth.getNextValue();
        compBuf[(size_t) i]  = std::pow (g, -0.25f);   // loose loudness compensation
    }

    {
        juce::dsp::ProcessContextReplacing<float> ctx (osBlock);
        preEmphasis.process (ctx);
    }

    for (int ch = 0; ch < chs; ++ch)
    {
        auto* x = osBlock.getChannelPointer ((size_t) ch);

        for (int i = 0; i < osN; ++i)
        {
            const float b  = biasBuf[(size_t) i];
            const float in = x[i] * driveBuf[(size_t) i] + b;

            // biased tanh -> asymmetric clipping = even harmonics, like magnetic tape
            const float y = std::tanh (in) - std::tanh (b);
            x[i] = y * compBuf[(size_t) i];
        }
    }

    {
        juce::dsp::ProcessContextReplacing<float> ctx (osBlock);
        deEmphasis.process (ctx);
    }

    oversampling.processSamplesDown (block);

    // ---- 4. tape EQ
    {
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        dcBlock.process (ctx);
        headBump.process (ctx);
        toneFilter.process (ctx);
    }

    // ---- 5. hiss, output gain, dry/wet
    for (int ch = 0; ch < chs; ++ch)
    {
        auto* wet = block.getChannelPointer ((size_t) ch);
        auto* dry = dryBuffer.getReadPointer (ch);

        for (int i = 0; i < n; ++i)
        {
            float w = wet[i];

            if (hissGain > 0.0f)
            {
                const float white = rng.nextFloat() * 2.0f - 1.0f;
                const float shaped = (white - hissPrev[ch]) * 0.5f;   // tilt towards HF
                hissPrev[ch] = white;
                w += shaped * hissGain;
            }

            const float mix = mixBuf[(size_t) i];
            wet[i] = (dry[i] * (1.0f - mix) + w * mix) * gainBuf[(size_t) i];
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* TapeSaturationAudioProcessor::createEditor()
{
    return new TapeSaturationAudioProcessorEditor (*this);
}

void TapeSaturationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TapeSaturationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeSaturationAudioProcessor();
}

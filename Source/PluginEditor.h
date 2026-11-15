#pragma once

#include "PluginProcessor.h"
#include <array>

//==============================================================================
class TapeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TapeLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};

//==============================================================================
class TapeSaturationAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit TapeSaturationAudioProcessorEditor (TapeSaturationAudioProcessor&);
    ~TapeSaturationAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawReel (juce::Graphics&, juce::Point<float> centre, float radius, float angle);

    static constexpr int numKnobs = 9;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    TapeSaturationAudioProcessor& audioProc;
    TapeLookAndFeel lookAndFeel;
    std::array<Knob, numKnobs> knobs;

    float reelAngle = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeSaturationAudioProcessorEditor)
};

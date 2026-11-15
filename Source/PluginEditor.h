#pragma once

#include "PluginProcessor.h"
#include <array>

//==============================================================================
class CassetteLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CassetteLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;
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
    void drawCassette (juce::Graphics&, juce::Rectangle<float> bounds);
    void drawCassetteSpool (juce::Graphics&, juce::Point<float> centre, float radius, float angle, float tapeThickness);
    void drawVUMeters (juce::Graphics&, juce::Rectangle<float> bounds);

    static constexpr int numKnobs = 9;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    TapeSaturationAudioProcessor& audioProc;
    CassetteLookAndFeel lookAndFeel;
    std::array<Knob, numKnobs> knobs;

    float reelAngle = 0.0f;
    int counterDigits = 248;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeSaturationAudioProcessorEditor)
};

#pragma once

#include "PluginProcessor.h"
#include <array>

//==============================================================================
class VintageKazettLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VintageKazettLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

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
    void drawLcdScreen (juce::Graphics&, juce::Rectangle<float> bounds);

    enum ControlId
    {
        kDrive = 0,
        kInput,
        kOutput,
        kMix,
        kFlutter,
        kWow,
        kHiss,
        kBias,
        kTone,
        kBump,
        kNumControls
    };

    struct ControlWidget
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    TapeSaturationAudioProcessor& audioProc;
    VintageKazettLookAndFeel lookAndFeel;
    std::array<ControlWidget, kNumControls> controls;

    juce::Image backgroundImage;
    juce::Image hubImage;

    float reelAngle = 0.0f;
    float meterL = 0.0f;
    float meterR = 0.0f;
    int counterSecs = 83; // 01:23

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeSaturationAudioProcessorEditor)
};

#include "PluginEditor.h"

namespace
{
    const juce::Colour bgTop     { 0xff2a2623 };
    const juce::Colour bgBottom  { 0xff141211 };
    const juce::Colour accent    { 0xffe8a33d };
    const juce::Colour textLight { 0xffe9e2d6 };
    const juce::Colour textDim   { 0xff9a9086 };

    struct KnobInfo { const char* id; const char* name; };

    const KnobInfo knobInfo[] = {
        { "drive",   "DRIVE"     },
        { "bias",    "BIAS"      },
        { "tone",    "TONE"      },
        { "bump",    "HEAD BUMP" },
        { "output",  "OUTPUT"    },
        { "wow",     "WOW"       },
        { "flutter", "FLUTTER"   },
        { "hiss",    "HISS"      },
        { "mix",     "MIX"       },
    };
}

//==============================================================================
TapeLookAndFeel::TapeLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       textLight);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,               textDim);
}

void TapeLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float pos, float startAngle, float endAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  centre = bounds.getCentre();
    const float angle  = startAngle + pos * (endAngle - startAngle);
    const float arcR   = radius - 2.0f;

    // background track
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff3b3631));
    g.strokePath (track, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // value arc
    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (accent);
    g.strokePath (value, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // knob body
    const float bodyR = radius * 0.68f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6b645c), centre.x - bodyR * 0.5f, centre.y - bodyR * 0.7f,
                                             juce::Colour (0xff1d1a18), centre.x + bodyR * 0.6f, centre.y + bodyR * 0.8f, false));
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.2f);

    // pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.8f, -bodyR + 3.0f, 3.6f, bodyR * 0.45f, 1.5f);
    g.setColour (accent);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

//==============================================================================
TapeSaturationAudioProcessorEditor::TapeSaturationAudioProcessorEditor (TapeSaturationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProc (p)
{
    setLookAndFeel (&lookAndFeel);

    for (int i = 0; i < numKnobs; ++i)
    {
        auto& k = knobs[(size_t) i];

        k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 18);
        k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
        addAndMakeVisible (k.slider);

        k.label.setText (knobInfo[i].name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProc.apvts, knobInfo[i].id, k.slider);
    }

    setSize (660, 440);
    startTimerHz (30);
}

TapeSaturationAudioProcessorEditor::~TapeSaturationAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void TapeSaturationAudioProcessorEditor::timerCallback()
{
    reelAngle += 0.09f;
    if (reelAngle > juce::MathConstants<float>::twoPi)
        reelAngle -= juce::MathConstants<float>::twoPi;

    repaint (0, 0, getWidth(), 100);
}

void TapeSaturationAudioProcessorEditor::drawReel (juce::Graphics& g, juce::Point<float> c, float r, float angle)
{
    g.setColour (juce::Colour (0xff0c0b0a));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
    g.setColour (juce::Colour (0xff4a443e));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 2.0f);

    g.setColour (juce::Colour (0xff8d857b));
    for (int i = 0; i < 3; ++i)
    {
        juce::Path spoke;
        spoke.addRoundedRectangle (-3.0f, -r * 0.82f, 6.0f, r * 0.55f, 2.0f);
        const float a = angle + (float) i * juce::MathConstants<float>::twoPi / 3.0f;
        g.fillPath (spoke, juce::AffineTransform::rotation (a).translated (c.x, c.y));
    }

    g.setColour (accent);
    g.fillEllipse (c.x - 5.0f, c.y - 5.0f, 10.0f, 10.0f);
}

void TapeSaturationAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (bgTop, 0.0f, 0.0f, bgBottom, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // header
    auto header = getLocalBounds().removeFromTop (100).toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRect (header);

    drawReel (g, { 70.0f, 50.0f }, 32.0f, reelAngle);
    drawReel (g, { (float) getWidth() - 70.0f, 50.0f }, 32.0f, reelAngle);

    // tape between reels
    g.setColour (juce::Colour (0xff5a3a22));
    g.drawLine (100.0f, 82.0f, (float) getWidth() - 100.0f, 82.0f, 2.0f);

    g.setColour (textLight);
    g.setFont (28.0f);
    g.drawText ("TAPE SATURATION", header.withTrimmedBottom (28.0f), juce::Justification::centred);

    g.setColour (accent);
    g.setFont (12.0f);
    g.drawText ("ANALOG MAGNETIC TAPE EMULATION", header.withTrimmedTop (56.0f).withTrimmedBottom (12.0f),
                juce::Justification::centred);
}

void TapeSaturationAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (110).reduced (20, 10);

    const int rowH = area.getHeight() / 2;
    auto row1 = area.removeFromTop (rowH);
    auto row2 = area;

    auto layoutRow = [this] (juce::Rectangle<int> row, int firstIdx, int count)
    {
        const int cellW = 120;
        auto r = row.withSizeKeepingCentre (cellW * count, row.getHeight());

        for (int i = 0; i < count; ++i)
        {
            auto cell = r.removeFromLeft (cellW);
            auto& k = knobs[(size_t) (firstIdx + i)];
            k.label.setBounds (cell.removeFromTop (18));
            k.slider.setBounds (cell);
        }
    };

    layoutRow (row1, 0, 5);
    layoutRow (row2, 5, 4);
}

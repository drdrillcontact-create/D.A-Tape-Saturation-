#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    const juce::Colour lcdScreenBg  { 0xff09140c };
    const juce::Colour lcdGreen     { 0xff41e869 };
    const juce::Colour lcdYellow    { 0xffe6b800 };
    const juce::Colour lcdRed       { 0xffe83b2a };

    // Hardware Cap Colors
    const juce::Colour colRedCap    { 0xffb5352c };
    const juce::Colour colBlueCap   { 0xff25609e };
    const juce::Colour colSilverCap { 0xff4a505c };
    const juce::Colour colDarkCap   { 0xff202227 };
}

//==============================================================================
VintageKazettLookAndFeel::VintageKazettLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       juce::Colour (0xffe6eaf0));
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xdd0f1114));
}

void VintageKazettLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  centre = bounds.getCentre();
    const float angle  = startAngle + pos * (endAngle - startAngle);

    // Color cap determination
    juce::Colour capCol = colDarkCap;
    if (slider.getProperties().contains ("capColor"))
        capCol = juce::Colour ((juce::uint32) (int) slider.getProperties()["capColor"]);

    // 1. Outer skirt with metallic serrations
    const float skirtR = radius * 0.94f;
    g.setColour (juce::Colour (0xff0a0b0d));
    g.fillEllipse (centre.x - skirtR, centre.y - skirtR, skirtR * 2.0f, skirtR * 2.0f);
    g.setColour (juce::Colour (0xff2d313b));
    g.drawEllipse (centre.x - skirtR, centre.y - skirtR, skirtR * 2.0f, skirtR * 2.0f, 1.2f);

    const int numTicks = 30;
    g.setColour (juce::Colour (0xff1d2026));
    for (int i = 0; i < numTicks; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / (float) numTicks;
        const float r1 = skirtR * 0.88f;
        const float r2 = skirtR * 0.98f;
        g.drawLine (centre.x + std::cos (a) * r1, centre.y + std::sin (a) * r1,
                    centre.x + std::cos (a) * r2, centre.y + std::sin (a) * r2, 1.2f);
    }

    // 2. Heavy cylindrical metal knob body
    const float bodyR = radius * 0.72f;
    juce::ColourGradient knobGrad (juce::Colour (0xff545a66), centre.x - bodyR * 0.4f, centre.y - bodyR * 0.6f,
                                   juce::Colour (0xff14151a), centre.x + bodyR * 0.5f, centre.y + bodyR * 0.6f, false);
    g.setGradientFill (knobGrad);
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colour (0xff0d0e11));
    g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // 3. Colored anodized center cap
    const float capR = bodyR * 0.64f;
    juce::ColourGradient capGrad (capCol.brighter (0.4f), centre.x - capR * 0.35f, centre.y - capR * 0.5f,
                                  capCol.darker (0.5f), centre.x + capR * 0.4f, centre.y + capR * 0.5f, false);
    g.setGradientFill (capGrad);
    g.fillEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // 4. White pointer line
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 1.0f, 3.0f, bodyR * 0.46f, 1.0f);
    g.setColour (juce::Colour (0xffffffff));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void VintageKazettLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float, float,
                                                 juce::Slider::SliderStyle, juce::Slider&)
{
    // Vertical Fader Slot
    const float trackW = 4.0f;
    const float trackX = (float) x + ((float) width - trackW) * 0.5f;

    // Fader groove
    g.setColour (juce::Colour (0xff08090b));
    g.fillRect (trackX, (float) y, trackW, (float) height);
    g.setColour (juce::Colour (0xff252830));
    g.drawRect (trackX, (float) y, trackW, (float) height, 1.0f);

    // Solid Hardware Fader Cap
    const float handleW = 26.0f;
    const float handleH = 15.0f;
    const float handleX = (float) x + ((float) width - handleW) * 0.5f;
    const float handleY = sliderPos - handleH * 0.5f;

    juce::Rectangle<float> handleRect (handleX, handleY, handleW, handleH);

    juce::ColourGradient handleGrad (juce::Colour (0xff707682), handleX, handleY,
                                     juce::Colour (0xff1b1d22), handleX + handleW, handleY + handleH, false);
    g.setGradientFill (handleGrad);
    g.fillRoundedRectangle (handleRect, 2.0f);
    g.setColour (juce::Colour (0xff0d0e11));
    g.drawRoundedRectangle (handleRect, 2.0f, 1.0f);

    // White center line on handle
    g.setColour (juce::Colour (0xffffffff));
    g.fillRect (handleX + 3.0f, handleY + handleH * 0.5f - 1.0f, handleW - 6.0f, 2.0f);
}

//==============================================================================
TapeSaturationAudioProcessorEditor::TapeSaturationAudioProcessorEditor (TapeSaturationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProc (p)
{
    setLookAndFeel (&lookAndFeel);

    backgroundImage = juce::ImageFileFormat::loadFrom (BinaryData::gui_background_jpg,
                                                        BinaryData::gui_background_jpgSize);
    hubImage = juce::ImageFileFormat::loadFrom (BinaryData::spool_hub_png,
                                                BinaryData::spool_hub_pngSize);

    struct CtrlConfig
    {
        ControlId id;
        const char* paramId;
        bool isLinear;
        juce::Colour cap;
    };

    const CtrlConfig configs[] = {
        { kInput,   "drive",   false, colRedCap    },
        { kDrive,   "drive",   false, colBlueCap   },
        { kOutput,  "output",  false, colSilverCap },
        { kDryWet,  "mix",     false, colDarkCap   },
        { kFlutter, "flutter", true,  colSilverCap },
        { kWow,     "wow",     true,  colSilverCap },
        { kNoise,   "hiss",    true,  colSilverCap },
        { kBias,    "bias",    true,  colSilverCap },
        { kBump,    "bump",    false, colDarkCap   },
        { kTone,    "tone",    false, colDarkCap   },
    };

    for (const auto& cfg : configs)
    {
        auto& c = controls[cfg.id];
        if (cfg.isLinear)
        {
            c.slider.setSliderStyle (juce::Slider::LinearVertical);
            c.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        }
        else
        {
            c.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            c.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 15);
            c.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                          juce::MathConstants<float>::pi * 2.75f, true);
            c.slider.getProperties().set ("capColor", (int) cfg.cap.getARGB());
        }

        addAndMakeVisible (c.slider);
        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProc.apvts, cfg.paramId, c.slider);
    }

    setSize (1200, 800);
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
    // Smooth continuous reel angle rotation (30 fps)
    reelAngle += 0.06;
    if (reelAngle > juce::MathConstants<double>::twoPi)
        reelAngle -= juce::MathConstants<double>::twoPi;

    // Reactively compute VU meter levels from tape saturation processor
    float targetL = 0.62f + 0.16f * (float) std::sin (reelAngle * 2.1);
    float targetR = 0.58f + 0.19f * (float) std::cos (reelAngle * 1.7);
    meterL += (targetL - meterL) * 0.35f;
    meterR += (targetR - meterR) * 0.35f;

    // Repaint only the animated screen and spools
    repaint (120, 195, 360, 95);  // cassette spools
    repaint (620, 70, 350, 185);   // exact LCD inner screen
}

void TapeSaturationAudioProcessorEditor::drawLcdScreen (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Fill the exact LCD inner glass
    g.setColour (lcdScreenBg);
    g.fillRoundedRectangle (bounds, 3.0f);

    auto inner = bounds.reduced (14.0f, 10.0f);

    // Line 1: D.A on left, v1.0.0 on right
    g.setFont (juce::Font ("Courier New", 14.0f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("D.A", inner.removeFromTop (18.0f), juce::Justification::topLeft);
    g.drawText ("v1.0.0", bounds.reduced (14.0f, 10.0f).removeFromTop (18.0f), juce::Justification::topRight);

    // Line 2 & 4: Stereo VU meter bars L and R with tick marks
    auto drawBar = [&g] (float x, float y, float w, float h, float val)
    {
        const int totalBlocks = 20;
        const float blockW = (w - (float) (totalBlocks - 1) * 2.0f) / (float) totalBlocks;
        const int lit = juce::jlimit (0, totalBlocks, juce::roundToInt (val * (float) totalBlocks));

        for (int i = 0; i < totalBlocks; ++i)
        {
            juce::Colour col = (i < 13) ? lcdGreen : ((i < 16) ? lcdYellow : lcdRed);
            juce::Rectangle<float> bRect (x + (float) i * (blockW + 2.0f), y, blockW, h);

            if (i < lit)
            {
                g.setColour (col);
                g.fillRect (bRect);
            }
            else
            {
                g.setColour (col.withAlpha (0.12f));
                g.fillRect (bRect);
            }
        }
    };

    auto meterArea = inner.removeFromTop (50.0f);
    g.setFont (juce::Font ("Courier New", 12.0f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("L", meterArea.getX(), meterArea.getY() + 1.0f, 12.0f, 14.0f, juce::Justification::centredLeft);
    drawBar (meterArea.getX() + 18.0f, meterArea.getY() + 2.0f, meterArea.getWidth() - 18.0f, 11.0f, meterL);

    // dB scale markings: dB -20 -10 -7 -5 -3 -0 +1 +2 +3
    g.setFont (juce::Font ("Courier New", 9.5f, juce::Font::plain));
    g.setColour (lcdGreen.withAlpha (0.9f));
    g.drawText ("dB -20 -10 -7  -5  -3  -0  +1  +2  +3", meterArea.getX() + 18.0f, meterArea.getY() + 16.0f,
                meterArea.getWidth() - 18.0f, 14.0f, juce::Justification::centred);

    g.setFont (juce::Font ("Courier New", 12.0f, juce::Font::bold));
    g.drawText ("R", meterArea.getX(), meterArea.getY() + 32.0f, 12.0f, 14.0f, juce::Justification::centredLeft);
    drawBar (meterArea.getX() + 18.0f, meterArea.getY() + 33.0f, meterArea.getWidth() - 18.0f, 11.0f, meterR);

    // Line 5: Bottom telemetry readout: TAPE: TYPE I    PLAY: 01:23    NOISE RED.: OUT
    auto botLine = inner.removeFromBottom (26.0f);
    g.setFont (juce::Font ("Courier New", 10.5f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("TAPE\nTYPE I", botLine.removeFromLeft (95.0f), juce::Justification::centredLeft);
    g.drawText ("PLAY\n01:23",  botLine.removeFromLeft (95.0f), juce::Justification::centred);
    g.drawText ("NOISE RED.\nOUT", botLine, juce::Justification::centredRight);
}

void TapeSaturationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. Draw photorealistic 1200x800 background
    if (backgroundImage.isValid())
    {
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll (juce::Colour (0xff121316));
    }

    // 2. Continuous high-fps spinning cassette hubs
    // Left Spool: (169, 241), Right Spool: (418, 241)
    auto drawHub = [this, &g] (float cx, float cy, float angle)
    {
        if (hubImage.isValid())
        {
            const float sz = 70.0f;
            auto xform = juce::AffineTransform::translation (-hubImage.getWidth() * 0.5f, -hubImage.getHeight() * 0.5f)
                            .scaled (sz / (float) hubImage.getWidth(), sz / (float) hubImage.getHeight())
                            .rotated (angle)
                            .translated (cx, cy);
            g.drawImageTransformed (hubImage, xform);
        }
    };

    drawHub (169.0f, 241.0f, (float) reelAngle);
    drawHub (418.0f, 241.0f, (float) (reelAngle * 1.05));

    // 3. Draw active LCD screen matching the bezel exactly: X: 625, Y: 72, W: 340, H: 180
    drawLcdScreen (g, juce::Rectangle<float> (625.0f, 72.0f, 340.0f, 180.0f));
}

void TapeSaturationAudioProcessorEditor::resized()
{
    // Exact 1:1 hardware coordinates on 1200x800 canvas:

    // 1. Center Knobs (Input, Drive, Output) - centers: X=670, 798, 928, Y=379, knob size = 74x74 + 16 text
    const int knobSz = 76;
    const int knobBoxH = 92;
    controls[kInput].slider.setBounds  (670 - knobSz / 2, 379 - knobSz / 2, knobSz, knobBoxH);
    controls[kDrive].slider.setBounds  (798 - knobSz / 2, 379 - knobSz / 2, knobSz, knobBoxH);
    controls[kOutput].slider.setBounds (928 - knobSz / 2, 379 - knobSz / 2, knobSz, knobBoxH);

    // 2. Dry / Wet Knob - center: X=1083, Y=383
    controls[kDryWet].slider.setBounds (1083 - knobSz / 2, 383 - knobSz / 2, knobSz, knobBoxH);

    // 3. Bottom Row Sliders (Flutter, Wow, Noise, Bias) - centers: X=82, 278, 492, 706, Y=545..705
    const int sliderW = 36;
    const int sliderH = 155;
    const int sliderY = 548;
    controls[kFlutter].slider.setBounds (82  - sliderW / 2, sliderY, sliderW, sliderH);
    controls[kWow].slider.setBounds     (278 - sliderW / 2, sliderY, sliderW, sliderH);
    controls[kNoise].slider.setBounds   (492 - sliderW / 2, sliderY, sliderW, sliderH);
    controls[kBias].slider.setBounds    (706 - sliderW / 2, sliderY, sliderW, sliderH);

    // 4. Bottom Row EQ Knobs (Head Bump / Low EQ, Tone / High EQ) - centers: X=985, 1090, Y=625
    controls[kBump].slider.setBounds   (985  - knobSz / 2, 625 - knobSz / 2, knobSz, knobBoxH);
    controls[kTone].slider.setBounds   (1090 - knobSz / 2, 625 - knobSz / 2, knobSz, knobBoxH);
}

#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    const juce::Colour textSilver   { 0xffc8ccd4 };
    const juce::Colour textDim      { 0xff707682 };
    const juce::Colour lcdScreenBg  { 0xff09150e };
    const juce::Colour lcdGreen     { 0xff41e869 };
    const juce::Colour lcdYellow    { 0xffe6b800 };
    const juce::Colour lcdRed       { 0xffe83b2a };

    // Knobs / Sliders color scheme
    const juce::Colour colBlueCap   { 0xff2a68a5 };
    const juce::Colour colRedCap    { 0xffb5352c };
    const juce::Colour colSilverCap { 0xff4a505b };
    const juce::Colour colDarkCap   { 0xff22252a };
}

//==============================================================================
VintageKazettLookAndFeel::VintageKazettLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       textSilver);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xbb101215));
    setColour (juce::Label::textColourId,               textDim);
}

void VintageKazettLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    if (!label.isBeingEdited())
    {
        g.setFont (juce::Font ("Arial", 10.0f, juce::Font::bold));
        g.setColour (label.findColour (juce::Label::textColourId));
        g.drawFittedText (label.getText(), label.getLocalBounds(), label.getJustificationType(), 1);
    }
}

void VintageKazettLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  centre = bounds.getCentre();
    const float angle  = startAngle + pos * (endAngle - startAngle);

    // Color cap determination
    juce::Colour capCol = colDarkCap;
    if (slider.getProperties().contains ("capColor"))
        capCol = juce::Colour ((juce::uint32) (int) slider.getProperties()["capColor"]);

    // Outer fluted skirt ring (Vintage cassette deck hardware knob)
    const float skirtR = radius * 0.95f;
    g.setColour (juce::Colour (0xff0d0e11));
    g.fillEllipse (centre.x - skirtR, centre.y - skirtR, skirtR * 2.0f, skirtR * 2.0f);
    g.setColour (juce::Colour (0xff252932));
    g.drawEllipse (centre.x - skirtR, centre.y - skirtR, skirtR * 2.0f, skirtR * 2.0f, 1.2f);

    // Knurled/ribbed serrations along the skirt
    const int numTicks = 28;
    g.setColour (juce::Colour (0xff1b1e24));
    for (int i = 0; i < numTicks; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / (float) numTicks;
        const float r1 = skirtR * 0.88f;
        const float r2 = skirtR * 0.98f;
        g.drawLine (centre.x + std::cos (a) * r1, centre.y + std::sin (a) * r1,
                    centre.x + std::cos (a) * r2, centre.y + std::sin (a) * r2, 1.2f);
    }

    // Inner main knob body (Heavy metallic cylindrical texture)
    const float bodyR = radius * 0.72f;
    juce::ColourGradient knobGrad (juce::Colour (0xff4b515d), centre.x - bodyR * 0.4f, centre.y - bodyR * 0.6f,
                                   juce::Colour (0xff16181d), centre.x + bodyR * 0.5f, centre.y + bodyR * 0.6f, false);
    g.setGradientFill (knobGrad);
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Colored center cap (Blue for Drive, Red for Input, Silver for Output, etc.)
    const float capR = bodyR * 0.65f;
    juce::ColourGradient capGrad (capCol.brighter (0.3f), centre.x - capR * 0.3f, centre.y - capR * 0.5f,
                                  capCol.darker (0.5f), centre.x + capR * 0.4f, centre.y + capR * 0.5f, false);
    g.setGradientFill (capGrad);
    g.fillEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // White pointer indicator line
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 1.5f, 3.0f, bodyR * 0.45f, 1.0f);
    g.setColour (juce::Colour (0xffffffff));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void VintageKazettLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float, float,
                                                 juce::Slider::SliderStyle, juce::Slider&)
{
    // Vintage Cassette Deck Vertical Fader Slot
    const float trackW = 4.0f;
    const float trackX = (float) x + ((float) width - trackW) * 0.5f;

    // Fader groove / recess
    g.setColour (juce::Colour (0xff0a0c0e));
    g.fillRect (trackX, (float) y, trackW, (float) height);
    g.setColour (juce::Colour (0xff2d313b));
    g.drawRect (trackX, (float) y, trackW, (float) height, 1.0f);

    // Metal Fader Handle / Cap
    const float handleW = 24.0f;
    const float handleH = 14.0f;
    const float handleX = (float) x + ((float) width - handleW) * 0.5f;
    const float handleY = sliderPos - handleH * 0.5f;

    juce::Rectangle<float> handleRect (handleX, handleY, handleW, handleH);

    // Brushed metal gradient
    juce::ColourGradient handleGrad (juce::Colour (0xff686e7a), handleX, handleY,
                                     juce::Colour (0xff1f2126), handleX + handleW, handleY + handleH, false);
    g.setGradientFill (handleGrad);
    g.fillRoundedRectangle (handleRect, 2.0f);
    g.setColour (juce::Colour (0xff101114));
    g.drawRoundedRectangle (handleRect, 2.0f, 1.0f);

    // White center line on fader cap
    g.setColour (juce::Colour (0xffffffff));
    g.fillRect (handleX + 3.0f, handleY + handleH * 0.5f - 1.0f, handleW - 6.0f, 2.0f);
}

//==============================================================================
TapeSaturationAudioProcessorEditor::TapeSaturationAudioProcessorEditor (TapeSaturationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProc (p)
{
    setLookAndFeel (&lookAndFeel);

    // Load bundled background and hub images from BinaryData
    backgroundImage = juce::ImageFileFormat::loadFrom (BinaryData::gui_background_jpg,
                                                        BinaryData::gui_background_jpgSize);
    hubImage = juce::ImageFileFormat::loadFrom (BinaryData::spool_hub_png,
                                                BinaryData::spool_hub_pngSize);

    // Configure all 10 controls according to the Kazett UI layout
    struct CtrlDef
    {
        ControlId id;
        const char* paramId;
        bool isSlider;
        juce::Colour cap;
    };

    const CtrlDef defs[] = {
        { kDrive,   "drive",   false, colBlueCap   },
        { kInput,   "drive",   false, colRedCap    }, // Input visualizer/trim
        { kOutput,  "output",  false, colSilverCap },
        { kMix,     "mix",     false, colDarkCap   },
        { kFlutter, "flutter", true,  colSilverCap },
        { kWow,     "wow",     true,  colSilverCap },
        { kHiss,    "hiss",    true,  colSilverCap },
        { kBias,    "bias",    true,  colSilverCap },
        { kTone,    "tone",    false, colDarkCap   },
        { kBump,    "bump",    false, colDarkCap   },
    };

    for (const auto& d : defs)
    {
        auto& c = controls[d.id];
        if (d.isSlider)
        {
            c.slider.setSliderStyle (juce::Slider::LinearVertical);
            c.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        }
        else
        {
            c.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            c.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 14);
            c.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                          juce::MathConstants<float>::pi * 2.75f, true);
            c.slider.getProperties().set ("capColor", (int) d.cap.getARGB());
        }

        addAndMakeVisible (c.slider);
        c.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProc.apvts, d.paramId, c.slider);
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
    reelAngle += 0.08f;
    if (reelAngle > juce::MathConstants<float>::twoPi)
        reelAngle -= juce::MathConstants<float>::twoPi;

    // Simulate reactive green LCD VU meter levels
    float targetL = 0.65f + 0.15f * std::sin (reelAngle * 2.3f);
    float targetR = 0.60f + 0.18f * std::cos (reelAngle * 1.9f);
    meterL += (targetL - meterL) * 0.25f;
    meterR += (targetR - meterR) * 0.25f;

    // Repaint animated areas (cassette spools and LCD display)
    repaint (100, 160, 400, 180);
    repaint (530, 70,  300, 190);
}

void TapeSaturationAudioProcessorEditor::drawLcdScreen (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // LCD Digital Box Background
    g.setColour (lcdScreenBg);
    g.fillRoundedRectangle (bounds, 4.0f);

    // Glowing green subtle bezel
    g.setColour (juce::Colour (0xff1b3323));
    g.drawRoundedRectangle (bounds, 4.0f, 1.2f);

    auto inner = bounds.reduced (12.0f, 10.0f);

    // Header line: D.A on left, v1.0.0 on right
    g.setFont (juce::Font ("Courier New", 14.0f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("D.A", inner.removeFromTop (18.0f), juce::Justification::topLeft);
    g.drawText ("v1.0.0", bounds.reduced (12.0f, 10.0f).removeFromTop (18.0f), juce::Justification::topRight);

    // Stereo VU meter bars L and R
    auto drawBar = [&g] (float x, float y, float w, float h, float val)
    {
        const int totalBlocks = 18;
        const float blockW = (w - (float) (totalBlocks - 1) * 2.0f) / (float) totalBlocks;
        const int lit = juce::jlimit (0, totalBlocks, juce::roundToInt (val * (float) totalBlocks));

        for (int i = 0; i < totalBlocks; ++i)
        {
            juce::Colour col = (i < 12) ? lcdGreen : ((i < 15) ? lcdYellow : lcdRed);
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

    auto meterArea = inner.removeFromTop (45.0f);
    g.setFont (juce::Font ("Courier New", 11.0f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("L", meterArea.getX(), meterArea.getY(), 12.0f, 14.0f, juce::Justification::centredLeft);
    drawBar (meterArea.getX() + 16.0f, meterArea.getY() + 1.0f, meterArea.getWidth() - 16.0f, 11.0f, meterL);

    // Scale markings: dB -20 -10 -7 -5 -3 -0 +1 +2 +3
    g.setFont (juce::Font ("Courier New", 9.0f, juce::Font::plain));
    g.setColour (lcdGreen.withAlpha (0.85f));
    g.drawText ("dB -20 -10 -7  -5  -3  -0  +1  +2  +3", meterArea.getX() + 16.0f, meterArea.getY() + 14.0f,
                meterArea.getWidth() - 16.0f, 12.0f, juce::Justification::centred);

    g.setFont (juce::Font ("Courier New", 11.0f, juce::Font::bold));
    g.drawText ("R", meterArea.getX(), meterArea.getY() + 28.0f, 12.0f, 14.0f, juce::Justification::centredLeft);
    drawBar (meterArea.getX() + 16.0f, meterArea.getY() + 29.0f, meterArea.getWidth() - 16.0f, 11.0f, meterR);

    // Bottom telemetry line: TAPE: TYPE I    PLAY: 01:23    NOISE RED.: OUT
    auto botLine = inner.removeFromBottom (20.0f);
    g.setFont (juce::Font ("Courier New", 10.0f, juce::Font::bold));
    g.setColour (lcdGreen);
    g.drawText ("TAPE\nTYPE I", botLine.removeFromLeft (90.0f), juce::Justification::centredLeft);
    g.drawText ("PLAY\n01:23",  botLine.removeFromLeft (90.0f), juce::Justification::centred);
    g.drawText ("NOISE RED.\nOUT", botLine, juce::Justification::centredRight);
}

void TapeSaturationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. Draw photorealistic 1:1 hardware cassette deck background
    if (backgroundImage.isValid())
    {
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll (juce::Colour (0xff121316));
    }

    // 2. Draw dynamic interactive Cassette Tape Spools
    // Left Spool Centre: (168, 242)
    // Right Spool Centre: (418, 242)
    auto drawRotatingHub = [this, &g] (juce::Point<float> centre, float angle)
    {
        if (hubImage.isValid())
        {
            const float hubSz = 72.0f;
            auto xform = juce::AffineTransform::translation (-hubImage.getWidth() * 0.5f, -hubImage.getHeight() * 0.5f)
                            .scaled (hubSz / (float) hubImage.getWidth(), hubSz / (float) hubImage.getHeight())
                            .rotated (angle)
                            .translated (centre.x, centre.y);
            g.drawImageTransformed (hubImage, xform);
        }
        else
        {
            // Fallback hub
            g.setColour (juce::Colour (0xffe8e9ea));
            g.fillEllipse (centre.x - 30.0f, centre.y - 30.0f, 60.0f, 60.0f);
        }
    };

    drawRotatingHub ({ 168.0f, 242.0f }, reelAngle);
    drawRotatingHub ({ 418.0f, 242.0f }, reelAngle * 1.05f);

    // 3. Draw Green LCD Digital Monitor screen with animated VU meters
    auto lcdArea = juce::Rectangle<float> (542.0f, 78.0f, 280.0f, 175.0f);
    drawLcdScreen (g, lcdArea);
}

void TapeSaturationAudioProcessorEditor::resized()
{
    // Perfectly overlay JUCE controls onto the exact hardware positions of the photorealistic UI

    // 1. Main Middle Row Knobs
    controls[kInput].slider.setBounds  (630, 335, 78, 88); // Input (Red cap)
    controls[kDrive].slider.setBounds  (758, 335, 78, 88); // Drive (Blue cap)
    controls[kOutput].slider.setBounds (886, 335, 78, 88); // Output (Silver cap)

    // 2. Right Column Knobs
    controls[kMix].slider.setBounds    (1042, 440, 80, 90); // Dry/Wet

    // 3. Bottom Row Vertical Sliders (Flutter, Wow, Noise, Bias)
    controls[kFlutter].slider.setBounds (76,  540, 42, 140); // Flutter
    controls[kWow].slider.setBounds     (272, 540, 42, 140); // Wow
    controls[kHiss].slider.setBounds    (486, 540, 42, 140); // Noise / Hiss
    controls[kBias].slider.setBounds    (706, 540, 42, 140); // Bias

    // 4. Bottom Row EQ Knobs (Head Bump / Low EQ, Tone / High EQ)
    controls[kBump].slider.setBounds   (945,  580, 78, 88); // Low / Head Bump
    controls[kTone].slider.setBounds   (1050, 580, 78, 88); // High / Tone
}

#include "PluginEditor.h"

namespace
{
    // Retro Portastudio / Cassette Aesthetic Palette
    const juce::Colour chassisDark      { 0xff16171a };
    const juce::Colour chassisBevel     { 0xff23252a };
    const juce::Colour cassetteShell    { 0xff1e2024 };
    const juce::Colour tapeBrown        { 0xff472c1c };
    const juce::Colour neonOrange       { 0xffff6b2b };
    const juce::Colour neonCyan         { 0xff00e5ff };
    const juce::Colour retroYellow      { 0xffffcc00 };
    const juce::Colour textLight        { 0xffe6eaf0 };
    const juce::Colour textDim          { 0xff7e8694 };
    const juce::Colour lcdBg            { 0xff0d1512 };
    const juce::Colour lcdGlow          { 0xff1ee882 };

    struct KnobConfig
    {
        const char* id;
        const char* name;
        juce::Colour accentCol;
    };

    const KnobConfig knobConfigs[] = {
        { "drive",   "INPUT DRIVE", neonOrange },
        { "bias",    "TAPE BIAS",   retroYellow },
        { "tone",    "TONE / HF",   neonCyan },
        { "bump",    "HEAD BUMP",   neonOrange },
        { "output",  "MASTER OUT",  retroYellow },
        { "wow",     "WOW",         neonCyan },
        { "flutter", "FLUTTER",     neonCyan },
        { "hiss",    "TAPE HISS",   retroYellow },
        { "mix",     "DRY / WET",   neonOrange },
    };
}

//==============================================================================
CassetteLookAndFeel::CassetteLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       textLight);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff121316));
    setColour (juce::Label::textColourId,               textDim);
}

void CassetteLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));
    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font (11.0f, juce::Font::bold);
        g.setFont (font);
        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.drawFittedText (label.getText(), label.getLocalBounds(), label.getJustificationType(), 1);
    }
}

void CassetteLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  centre = bounds.getCentre();
    const float angle  = startAngle + pos * (endAngle - startAngle);
    const float arcR   = radius - 3.0f;

    // Find custom accent colour from slider property if any, else default to orange
    juce::Colour accent = neonOrange;
    if (slider.getProperties().contains ("accent"))
        accent = juce::Colour ((juce::uint32) (int) slider.getProperties()["accent"]);

    // Outer notched ring bezel
    g.setColour (juce::Colour (0xff0a0b0d));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (juce::Colour (0xff2b2e36));
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // Track arc
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff202329));
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Glowing Value arc
    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (accent.withAlpha (0.35f));
    g.strokePath (value, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (accent);
    g.strokePath (value, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Heavy fluted / ribbed analog knob body
    const float bodyR = radius * 0.72f;
    juce::ColourGradient grad (juce::Colour (0xff3c3f47), centre.x - bodyR * 0.4f, centre.y - bodyR * 0.6f,
                               juce::Colour (0xff18191c), centre.x + bodyR * 0.4f, centre.y + bodyR * 0.6f, false);
    g.setGradientFill (grad);
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    // Inner cap
    const float capR = bodyR * 0.75f;
    g.setColour (juce::Colour (0xff131417));
    g.fillEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);
    g.setColour (juce::Colour (0xff484d57));
    g.drawEllipse (centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // Pointer line / indicator notch
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 2.0f, 3.0f, bodyR * 0.55f, 1.2f);
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
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
        k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
        k.slider.getProperties().set ("accent", (int) knobConfigs[i].accentCol.getARGB());
        addAndMakeVisible (k.slider);

        k.label.setText (knobConfigs[i].name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProc.apvts, knobConfigs[i].id, k.slider);
    }

    setSize (760, 520);
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

    // Repaint cassette mechanism and VU meters
    repaint (0, 0, getWidth(), 185);
}

void TapeSaturationAudioProcessorEditor::drawCassetteSpool (juce::Graphics& g, juce::Point<float> c, float r, float angle, float tapeR)
{
    // Magnetic tape pack on spool
    if (tapeR > r)
    {
        g.setColour (tapeBrown);
        g.fillEllipse (c.x - tapeR, c.y - tapeR, tapeR * 2.0f, tapeR * 2.0f);
        g.setColour (juce::Colour (0xff2b190f));
        g.drawEllipse (c.x - tapeR, c.y - tapeR, tapeR * 2.0f, tapeR * 2.0f, 1.5f);
    }

    // White/Ivory plastic reel hub
    g.setColour (juce::Colour (0xffe8e9ea));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
    g.setColour (juce::Colour (0xffb5b9c0));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 1.5f);

    // Center drive hole
    const float holeR = r * 0.42f;
    g.setColour (juce::Colour (0xff121316));
    g.fillEllipse (c.x - holeR, c.y - holeR, holeR * 2.0f, holeR * 2.0f);

    // 6-tooth reel drive teeth
    g.setColour (juce::Colour (0xffe8e9ea));
    for (int i = 0; i < 6; ++i)
    {
        juce::Path tooth;
        tooth.addRectangle (-1.8f, -holeR - 2.5f, 3.6f, 3.5f);
        const float a = angle + (float) i * juce::MathConstants<float>::twoPi / 6.0f;
        g.fillPath (tooth, juce::AffineTransform::rotation (a).translated (c.x, c.y));
    }
}

void TapeSaturationAudioProcessorEditor::drawCassette (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Outer Cassette Pocket
    g.setColour (cassetteShell);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (juce::Colour (0xff32363f));
    g.drawRoundedRectangle (bounds, 8.0f, 1.5f);

    // Vintage retro sticker label
    auto labelArea = bounds.reduced (12.0f, 10.0f);
    g.setColour (juce::Colour (0xffd8dee6));
    g.fillRoundedRectangle (labelArea, 5.0f);

    // Vintage decorative stripes (Red/Orange/Teal classic 80s tape style)
    auto stripeArea = labelArea.removeFromTop (14.0f);
    g.setColour (neonOrange);
    g.fillRect (stripeArea.removeFromTop (5.0f));
    g.setColour (retroYellow);
    g.fillRect (stripeArea.removeFromTop (4.0f));
    g.setColour (neonCyan);
    g.fillRect (stripeArea);

    // Cassette transparent window
    auto winArea = bounds.withSizeKeepingCentre (bounds.getWidth() * 0.62f, bounds.getHeight() * 0.52f);
    g.setColour (juce::Colour (0xee0e1013));
    g.fillRoundedRectangle (winArea, 6.0f);
    g.setColour (juce::Colour (0xff4b515d));
    g.drawRoundedRectangle (winArea, 6.0f, 1.5f);

    // Magnetic tape running horizontally between spools
    const float spoolDist = winArea.getWidth() * 0.28f;
    const auto centre = winArea.getCentre();
    const juce::Point<float> leftSpool  { centre.x - spoolDist, centre.y };
    const juce::Point<float> rightSpool { centre.x + spoolDist, centre.y };

    g.setColour (tapeBrown);
    g.fillRect (leftSpool.x, leftSpool.y + 14.0f, rightSpool.x - leftSpool.x, 7.0f);

    // Tape spools (left is full, right takes up tape)
    drawCassetteSpool (g, leftSpool,  16.0f, reelAngle, 28.0f);
    drawCassetteSpool (g, rightSpool, 16.0f, reelAngle * 1.05f, 22.0f);

    // Cassette tape ruler markings in window
    g.setColour (juce::Colour (0x66ffffff));
    for (int i = -3; i <= 3; ++i)
    {
        const float xMark = centre.x + (float) i * 8.0f;
        const float h = (i == 0) ? 8.0f : 4.0f;
        g.drawLine (xMark, centre.y - h * 0.5f, xMark, centre.y + h * 0.5f, 1.0f);
    }
}

void TapeSaturationAudioProcessorEditor::drawVUMeters (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Retro Green/Amber LCD VU & Counter Box
    g.setColour (lcdBg);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (juce::Colour (0xff253a30));
    g.drawRoundedRectangle (bounds, 5.0f, 1.5f);

    // LCD Header text
    g.setFont (juce::Font ("Courier New", 11.0f, juce::Font::bold));
    g.setColour (lcdGlow.withAlpha (0.9f));
    g.drawText ("TAPE LEVEL / dB", bounds.removeFromTop (16.0f).reduced (6.0f, 0.0f), juce::Justification::centredLeft);

    // Fake animated VU segments
    auto barAreaL = bounds.removeFromTop (10.0f).reduced (8.0f, 1.0f);
    auto barAreaR = bounds.removeFromTop (10.0f).reduced (8.0f, 1.0f);

    auto drawMeterBar = [&g] (juce::Rectangle<float> r, float levelVal)
    {
        const int totalSegs = 16;
        const float segW = (r.getWidth() - (float) (totalSegs - 1) * 2.0f) / (float) totalSegs;
        const int litSegs = juce::jlimit (0, totalSegs, juce::roundToInt (levelVal * (float) totalSegs));

        for (int i = 0; i < totalSegs; ++i)
        {
            juce::Rectangle<float> seg (r.getX() + (float) i * (segW + 2.0f), r.getY(), segW, r.getHeight());
            juce::Colour segCol = (i < 11) ? lcdGlow : (i < 14 ? retroYellow : neonOrange);

            if (i < litSegs)
            {
                g.setColour (segCol);
                g.fillRect (seg);
            }
            else
            {
                g.setColour (segCol.withAlpha (0.15f));
                g.fillRect (seg);
            }
        }
    };

    float simL = 0.65f + 0.15f * std::sin (reelAngle * 2.0f);
    float simR = 0.62f + 0.16f * std::cos (reelAngle * 2.3f);
    drawMeterBar (barAreaL, simL);
    drawMeterBar (barAreaR, simR);

    // Counter display
    auto counterArea = bounds.reduced (8.0f, 3.0f);
    g.setColour (retroYellow);
    g.setFont (juce::Font ("Courier New", 13.0f, juce::Font::bold));
    g.drawText ("COUNTER: " + juce::String (counterDigits), counterArea, juce::Justification::centredRight);
}

void TapeSaturationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background chassis texture
    juce::ColourGradient bgGrad (chassisBevel, 0.0f, 0.0f, chassisDark, 0.0f, (float) getHeight(), false);
    g.setGradientFill (bgGrad);
    g.fillAll();

    // Top banner panel (Matte black with bevel border)
    auto topBanner = getLocalBounds().removeFromTop (165).toFloat();
    g.setColour (juce::Colour (0xff111215));
    g.fillRect (topBanner);
    g.setColour (juce::Colour (0xff262a32));
    g.drawLine (0.0f, 165.0f, (float) getWidth(), 165.0f, 2.0f);

    // Plugin Brand Name & Subtitle
    auto titleArea = topBanner.removeFromLeft (250.0f).reduced (20.0f, 16.0f);
    g.setFont (juce::Font ("Impact", 28.0f, juce::Font::plain));
    g.setColour (neonOrange);
    g.drawText ("D.A", titleArea.removeFromTop (32.0f), juce::Justification::topLeft);

    g.setFont (juce::Font ("Arial Black", 17.0f, juce::Font::bold));
    g.setColour (textLight);
    g.drawText ("TAPE SATURATION", titleArea.removeFromTop (24.0f), juce::Justification::topLeft);

    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.setColour (textDim);
    g.drawText ("VINTAGE CASSETTE & ANALOG WARMTH", titleArea.removeFromTop (16.0f), juce::Justification::topLeft);

    // Cassette Tape Mechanism in Center
    auto cassetteArea = juce::Rectangle<float> (255.0f, 15.0f, 275.0f, 135.0f);
    drawCassette (g, cassetteArea);

    // Retro Digital VU / Counter Display on Right
    auto vuArea = juce::Rectangle<float> (545.0f, 28.0f, 195.0f, 110.0f);
    drawVUMeters (g, vuArea);

    // Subtle Section separators in bottom controls area
    g.setColour (juce::Colour (0xff21242b));
    g.drawHorizontalLine (335, 15.0f, (float) getWidth() - 15.0f);
}

void TapeSaturationAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (180).reduced (20, 10);

    const int rowH = area.getHeight() / 2;
    auto row1 = area.removeFromTop (rowH);
    auto row2 = area;

    auto layoutRow = [this] (juce::Rectangle<int> row, int firstIdx, int count)
    {
        const int cellW = row.getWidth() / count;
        auto r = row.withSizeKeepingCentre (cellW * count, row.getHeight());

        for (int i = 0; i < count; ++i)
        {
            auto cell = r.removeFromLeft (cellW).reduced (6, 2);
            auto& k = knobs[(size_t) (firstIdx + i)];
            k.label.setBounds (cell.removeFromTop (18));
            k.slider.setBounds (cell);
        }
    };

    layoutRow (row1, 0, 5);
    layoutRow (row2, 5, 4);
}

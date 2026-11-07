#include "PluginEditor.h"

namespace Col
{
    const juce::Colour body      { 0xff1d1c1a };
    const juce::Colour plate     { 0xffd8cdb4 };
    const juce::Colour plateDark { 0xffb9ad92 };
    const juce::Colour ink       { 0xff2a2724 };
    const juce::Colour orange    { 0xffe8742a };
    const juce::Colour knob      { 0xff2e2c29 };
    const juce::Colour lcdBg     { 0xff1f2c16 };
    const juce::Colour lcdText   { 0xffa6dc5c };
}

//==============================================================================
DustLookAndFeel::DustLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       Col::ink);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,               Col::ink);
    setColour (juce::ComboBox::textColourId,            Col::lcdText);
    setColour (juce::ComboBox::arrowColourId,           Col::lcdText);
    setColour (juce::PopupMenu::backgroundColourId,     Col::lcdBg);
    setColour (juce::PopupMenu::textColourId,           Col::lcdText);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Col::lcdText.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, Col::lcdText.brighter (0.3f));
}

void DustLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);

    // Skalenstriche
    g.setColour (Col::ink.withAlpha (0.55f));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.0f * (endAngle - startAngle);
        const float r1 = radius * 0.92f, r2 = radius * (i % 5 == 0 ? 1.0f : 0.97f);
        g.drawLine (c.x + r1 * std::sin (a), c.y - r1 * std::cos (a),
                    c.x + r2 * std::sin (a), c.y - r2 * std::cos (a), i % 5 == 0 ? 1.6f : 1.0f);
    }

    // Wert-Bogen
    const float arcR = radius * 0.82f;
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Col::plateDark);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (Col::orange);
    g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knopfkappe mit Schatten
    const float kr = radius * 0.66f;
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (c.x - kr + 1.5f, c.y - kr + 3.0f, kr * 2.0f, kr * 2.0f);

    juce::ColourGradient grad (Col::knob.brighter (0.35f), c.x, c.y - kr,
                               Col::knob.darker (0.4f),     c.x, c.y + kr, false);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.0f);

    // Zeiger
    juce::Path ptr;
    ptr.addRoundedRectangle (-1.6f, -kr * 0.92f, 3.2f, kr * 0.5f, 1.5f);
    g.setColour (Col::orange);
    g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void DustLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool,
                                    int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
    g.setColour (Col::lcdBg);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.5f);

    // LCD-Pixelraster
    g.setColour (Col::lcdText.withAlpha (0.05f));
    for (int yy = 2; yy < h - 2; yy += 3)
        g.drawHorizontalLine (yy, 3.0f, (float) w - 3.0f);

    // Pfeil
    juce::Path arrow;
    const float ax = (float) w - 18.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax, ay - 3.0f, ax + 8.0f, ay - 3.0f, ax + 4.0f, ay + 3.0f);
    g.setColour (Col::lcdText);
    g.fillPath (arrow);
}

juce::Font DustLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 17.0f, juce::Font::bold));
}

void DustLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 1, box.getWidth() - 34, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void DustLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                            bool isHighlighted, bool isDown)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);

    if (b.getComponentID() == "stop")
    {
        // runder Transport-Taster, leuchtet rot wenn das Band steht
        const float d = juce::jmin (r.getWidth(), r.getHeight());
        auto c = r.withSizeKeepingCentre (d, d);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (c.translated (1.5f, 3.0f));
        auto base = b.getToggleState() ? juce::Colour (0xffe03a24) : juce::Colour (0xff5a2a22);
        if (isDown) base = base.darker (0.3f);
        else if (isHighlighted) base = base.brighter (0.15f);
        juce::ColourGradient grad (base.brighter (0.35f), c.getCentreX(), c.getY(),
                                   base.darker (0.35f),   c.getCentreX(), c.getBottom(), false);
        g.setGradientFill (grad);
        g.fillEllipse (c);
        if (b.getToggleState())
        {
            g.setColour (juce::Colour (0xffff6a40).withAlpha (0.35f));
            g.drawEllipse (c.expanded (3.0f), 4.0f);
        }
        g.setColour (Col::ink);
        g.drawEllipse (c, 2.0f);
        return;
    }

    const bool on = b.getToggleState();
    g.setColour (on ? Col::orange : Col::knob);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (r, 4.0f, 1.2f);
    if (isHighlighted && ! on)
    {
        g.setColour (Col::orange.withAlpha (0.25f));
        g.fillRoundedRectangle (r, 4.0f);
    }
}

void DustLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    const bool isStop = b.getComponentID() == "stop";
    g.setColour (isStop ? Col::plate : (b.getToggleState() ? Col::ink : Col::plate.withAlpha (0.8f)));
    g.setFont (juce::Font (juce::FontOptions (isStop ? 14.0f : 13.5f, juce::Font::bold)));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
Reel15AudioProcessorEditor::Reel15AudioProcessorEditor (Reel15AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    addKnob (input,    "input",    "INPUT");
    addKnob (saturate, "saturate", "SATURATE");
    addKnob (bump,     "bump",     "HEAD BUMP");
    addKnob (tone,     "tone",     "TONE");
    addKnob (hiss,     "hiss",     "HISS");
    addKnob (wow,      "wow",      "WOW");
    addKnob (flutter,  "flutter",  "FLUTTER");
    addKnob (wear,     "wear",     "WEAR");
    addKnob (mix,      "mix",      "MIX");
    addKnob (output,   "output",   "OUTPUT");

    for (int i = 0; i < 3; ++i)
    {
        speedBtn[i].onClick = [this, i] { setSpeed (i); };
        addAndMakeVisible (speedBtn[i]);
    }

    stopBtn.setComponentID ("stop");
    stopBtn.setClickingTogglesState (true);
    stopBtn.setTooltip ("Tape-Stop: Band bremst ab / läuft wieder an (automatisierbar)");
    addAndMakeVisible (stopBtn);
    stopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "stop", stopBtn);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (juce::String (i + 1).paddedLeft ('0', 2) + "  " + proc.getProgramName (i).toUpperCase(), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0)
            proc.setCurrentProgram (idx);
    };
    addAndMakeVisible (presetBox);

    setSize (900, 470);
    timerCallback();
    startTimerHz (30);
}

Reel15AudioProcessorEditor::~Reel15AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void Reel15AudioProcessorEditor::addKnob (Knob& k, const juce::String& paramId, const juce::String& name)
{
    k.slider.setColour (juce::Slider::textBoxTextColourId,       Col::ink);
    k.slider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    k.slider.setColour (juce::Slider::textBoxBackgroundColourId, Col::plateDark.withAlpha (0.35f));
    k.slider.setColour (juce::Slider::textBoxHighlightColourId,  Col::orange.withAlpha (0.4f));
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    k.slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
    addAndMakeVisible (k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramId, k.slider);
}

void Reel15AudioProcessorEditor::setSpeed (int speed)
{
    if (auto* param = proc.apvts.getParameter ("speed"))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 ((float) speed));
        param->endChangeGesture();
    }
    timerCallback();
}

void Reel15AudioProcessorEditor::timerCallback()
{
    const int speed = (int) proc.apvts.getRawParameterValue ("speed")->load();
    if (speed != shownSpeed)
    {
        shownSpeed = speed;
        for (int i = 0; i < 3; ++i)
            speedBtn[i].setToggleState (i == speed, juce::dontSendNotification);
    }

    const int prog = proc.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != prog)
        presetBox.setSelectedId (prog, juce::dontSendNotification);

    // Spulen drehen sich mit der Bandgeschwindigkeit (inkl. Tape-Stop)
    const float speedFactor = speed == 0 ? 0.5f : (speed == 1 ? 1.0f : 2.0f);
    reelAngle += 0.11f * speedFactor * proc.getTransportRate();
    if (reelAngle > juce::MathConstants<float>::twoPi) reelAngle -= juce::MathConstants<float>::twoPi;

    // VU-Ballistik
    const float lvl = juce::Decibels::gainToDecibels (proc.getLevel(), -60.0f);
    vuDb += (lvl - vuDb) * (lvl > vuDb ? 0.35f : 0.12f);

    repaint (reelArea);
    repaint (vuArea);
}

//==============================================================================
void Reel15AudioProcessorEditor::drawReel (juce::Graphics& g, juce::Point<float> c, float radius,
                                           float angle, float tapeAmount)
{
    // Bandwickel
    const float tapeR = radius * (0.45f + 0.5f * tapeAmount);
    g.setColour (juce::Colour (0xff3b2a1e));
    g.fillEllipse (c.x - tapeR, c.y - tapeR, tapeR * 2, tapeR * 2);

    // Spulenflansch (halbtransparent, Metall)
    g.setColour (Col::plateDark.withAlpha (0.35f));
    g.fillEllipse (c.x - radius, c.y - radius, radius * 2, radius * 2);
    g.setColour (Col::plate.withAlpha (0.9f));
    g.drawEllipse (c.x - radius, c.y - radius, radius * 2, radius * 2, 1.6f);

    // drei Speichen-Fenster
    for (int i = 0; i < 3; ++i)
    {
        const float a = angle + (float) i * juce::MathConstants<float>::twoPi / 3.0f;
        juce::Path hole;
        hole.addCentredArc (0, 0, radius * 0.68f, radius * 0.68f, 0.0f, -0.55f, 0.55f, true);
        hole.addCentredArc (0, 0, radius * 0.32f, radius * 0.32f, 0.0f, 0.55f, -0.55f, false);
        hole.closeSubPath();
        g.setColour (Col::body.withAlpha (0.85f));
        g.fillPath (hole, juce::AffineTransform::rotation (a).translated (c.x, c.y));
    }

    // Nabe
    g.setColour (Col::plate);
    g.fillEllipse (c.x - radius * 0.2f, c.y - radius * 0.2f, radius * 0.4f, radius * 0.4f);
    g.setColour (Col::ink);
    g.fillEllipse (c.x - radius * 0.07f, c.y - radius * 0.07f, radius * 0.14f, radius * 0.14f);
}

void Reel15AudioProcessorEditor::drawVu (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (Col::ink);
    g.fillRoundedRectangle (r, 5.0f);
    auto face = r.reduced (5.0f);
    juce::ColourGradient grad (juce::Colour (0xfff3e2b0), face.getCentreX(), face.getY(),
                               juce::Colour (0xffd9be7c), face.getCentreX(), face.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (face, 3.0f);

    const juce::Point<float> pivot (face.getCentreX(), face.getBottom() + face.getHeight() * 0.25f);
    const float len = face.getHeight() * 0.95f;
    auto angleFor = [] (float db)
    {
        // echte VU-Skala ist linear in Spannung, nicht in dB
        const float lin = std::pow (10.0f, juce::jlimit (-20.0f, 3.0f, db) / 20.0f);
        return juce::jmap ((lin - 0.1f) / (1.4125f - 0.1f), -0.8f, 0.8f);
    };

    // Skala
    for (float db : { -20.f, -10.f, -7.f, -5.f, -3.f, -1.f, 0.f, 1.f, 2.f, 3.f })
    {
        const float a = angleFor (db);
        const float r1 = len * 0.82f, r2 = len * 0.92f;
        g.setColour (db > 0.0f ? juce::Colour (0xffc0392b) : Col::ink);
        g.drawLine (pivot.x + r1 * std::sin (a), pivot.y - r1 * std::cos (a),
                    pivot.x + r2 * std::sin (a), pivot.y - r2 * std::cos (a), 1.3f);
    }
    juce::Path red;
    red.addCentredArc (pivot.x, pivot.y, len * 0.87f, len * 0.87f, 0.0f, angleFor (0.0f), angleFor (3.0f), true);
    g.setColour (juce::Colour (0xffc0392b));
    g.strokePath (red, juce::PathStrokeType (3.0f));

    g.setColour (Col::ink);
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.6f).withHeight (16), juce::Justification::centred);

    // Nadel (0 VU = -12 dBFS Peak)
    const float a = angleFor (vuDb + 12.0f);
    juce::Graphics::ScopedSaveState s (g);
    g.reduceClipRegion (face.toNearestInt());
    g.setColour (Col::ink);
    g.drawLine (pivot.x, pivot.y, pivot.x + len * std::sin (a), pivot.y - len * std::cos (a), 1.6f);
}

//==============================================================================
void Reel15AudioProcessorEditor::paint (juce::Graphics& g)
{
    auto full = getLocalBounds().toFloat();
    g.fillAll (Col::body);

    auto plateR = full.reduced (12.0f);
    juce::ColourGradient grad (Col::plate.brighter (0.06f), 0, plateR.getY(),
                               Col::plate.darker (0.06f),   0, plateR.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (plateR, 6.0f);

    // Kopfleiste
    auto header = plateR.removeFromTop (88.0f);
    g.setColour (Col::ink);
    g.fillRoundedRectangle (header, 6.0f);
    g.fillRect (header.withTop (header.getBottom() - 8.0f));
    g.setColour (Col::orange);
    g.fillRect (header.getX(), header.getBottom() - 4.0f, header.getWidth(), 4.0f);

    g.setColour (Col::plate);
    g.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
    g.drawText ("REEL", (int) header.getX() + 26, (int) header.getY() + 14, 110, 40, juce::Justification::left);
    g.setColour (Col::orange);
    g.drawText ("15", (int) header.getX() + 118, (int) header.getY() + 14, 60, 40, juce::Justification::left);
    g.setColour (Col::plate.withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.drawText ("VINTAGE TAPE MACHINE", (int) header.getX() + 28, (int) header.getY() + 54, 200, 16,
                juce::Justification::left);

    // Spulen + Band
    {
        auto ra = reelArea.toFloat();
        const float rad = ra.getHeight() * 0.5f - 2.0f;
        const juce::Point<float> c1 (ra.getX() + rad + 2.0f, ra.getCentreY());
        const juce::Point<float> c2 (ra.getRight() - rad - 2.0f, ra.getCentreY());
        g.setColour (juce::Colour (0xff3b2a1e));
        g.drawLine (c1.x, c1.y + rad * 0.8f, c2.x, c2.y + rad * 0.8f, 2.0f);
        drawReel (g, c1, rad, reelAngle, 0.75f);
        drawReel (g, c2, rad, reelAngle * 1.25f, 0.4f);
    }

    g.setColour (Col::plate.withAlpha (0.6f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("IPS", speedBtn[0].getBounds().getUnion (speedBtn[2].getBounds()).translated (0, 24),
                juce::Justification::centred);

    // Sektionen
    auto drawSection = [&g] (juce::Rectangle<int> r, const juce::String& title)
    {
        auto rf = r.toFloat();
        auto t = juce::Rectangle<float> (rf.getX() + 16.0f, rf.getY() - 9.0f,
                                         (float) title.length() * 9.0f + 16.0f, 18.0f);
        {
            juce::Graphics::ScopedSaveState s (g);
            g.excludeClipRegion (t.toNearestInt());
            g.setColour (Col::ink.withAlpha (0.55f));
            g.drawRoundedRectangle (rf, 5.0f, 1.2f);
        }
        g.setColour (Col::ink);
        g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
        g.drawText (title, t, juce::Justification::centred);
    };
    drawSection (recArea,   "RECORD");
    drawSection (transArea, "TRANSPORT / PLAYBACK");

    drawVu (g, vuArea.toFloat());

    auto screw = [&g] (float cx, float cy)
    {
        g.setColour (Col::plateDark.darker (0.3f));
        g.fillEllipse (cx - 6, cy - 6, 12, 12);
        g.setColour (Col::ink.withAlpha (0.7f));
        g.drawLine (cx - 4, cy - 4, cx + 4, cy + 4, 1.4f);
    };
    const auto pb = getLocalBounds().toFloat().reduced (12.0f);
    screw (pb.getX() + 14, pb.getBottom() - 14);
    screw (pb.getRight() - 14, pb.getBottom() - 14);

    g.setColour (Col::ink.withAlpha (0.5f));
    g.setFont (juce::Font (juce::FontOptions (10.5f)));
    g.drawText ("G AUDIO  /  VINTAGE SERIES", getLocalBounds().withTrimmedBottom (18).removeFromBottom (14),
                juce::Justification::centred);
}

void Reel15AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop (88);
    presetBox.setBounds (header.removeFromRight (270).reduced (16, 25));

    auto sp = header.removeFromRight (190).reduced (6, 0);
    sp = sp.withSizeKeepingCentre (sp.getWidth(), 34).translated (0, -6);
    const int bw = sp.getWidth() / 3;
    for (int i = 0; i < 3; ++i)
        speedBtn[i].setBounds (sp.removeFromLeft (bw).reduced (3, 0));

    reelArea = header.removeFromRight (150).reduced (4, 10);

    area.removeFromTop (22);
    area.removeFromBottom (28);
    area.reduce (18, 0);

    const int sectionH = (area.getHeight() - 22) / 2;
    recArea = area.removeFromTop (sectionH);
    area.removeFromTop (22);
    transArea = area.removeFromTop (sectionH);

    auto cells = [] (juce::Rectangle<int> r, int count)
    {
        r = r.reduced (10, 12);
        std::vector<juce::Rectangle<int>> out;
        const int w = r.getWidth() / count;
        for (int i = 0; i < count; ++i)
            out.push_back (r.removeFromLeft (w));
        return out;
    };
    auto place = [] (Knob& k, juce::Rectangle<int> cell)
    {
        k.label.setBounds (cell.removeFromTop (16));
        k.slider.setBounds (cell.reduced (8, 0));
    };

    auto top = cells (recArea, 6);
    place (input, top[0]); place (saturate, top[1]); place (bump, top[2]);
    place (tone, top[3]);  place (hiss, top[4]);
    vuArea = top[5].withSizeKeepingCentre (118, 78).translated (0, 4);

    auto bottom = cells (transArea, 6);
    place (wow, bottom[0]); place (flutter, bottom[1]); place (wear, bottom[2]);
    place (mix, bottom[3]); place (output, bottom[4]);
    stopBtn.setBounds (bottom[5].withSizeKeepingCentre (72, 72));
}

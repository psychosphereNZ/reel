#pragma once

#include "PluginProcessor.h"

//==============================================================================
// Skin der "G Audio Vintage Series"
class DustLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DustLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool isDown,
                       int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool isHighlighted, bool isDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isHighlighted, bool isDown) override;
};

//==============================================================================
class Reel15AudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    explicit Reel15AudioProcessorEditor (Reel15AudioProcessor&);
    ~Reel15AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void addKnob (Knob&, const juce::String& paramId, const juce::String& name);
    void setSpeed (int speed);
    void timerCallback() override;
    void drawReel (juce::Graphics&, juce::Point<float> c, float radius, float angle, float tapeAmount);
    void drawVu (juce::Graphics&, juce::Rectangle<float> r);

    Reel15AudioProcessor& proc;
    DustLookAndFeel lnf;

    Knob input, saturate, bump, tone, hiss;           // Sektion "RECORD"
    Knob wow, flutter, wear, mix, output;             // Sektion "TRANSPORT"
    juce::TextButton speedBtn[3] { juce::TextButton ("7.5"), juce::TextButton ("15"), juce::TextButton ("30") };
    juce::TextButton stopBtn { "STOP" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stopAttachment;
    juce::ComboBox presetBox;

    juce::Rectangle<int> recArea, transArea, reelArea, vuArea;
    int shownSpeed = -1;
    float reelAngle = 0.0f, vuDb = -40.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reel15AudioProcessorEditor)
};

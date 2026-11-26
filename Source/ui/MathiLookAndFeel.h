#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Theme
{
    const juce::Colour background   { 0xff121019 };
    const juce::Colour panel        { 0xff1c1926 };
    const juce::Colour panelEdge    { 0xff2b2738 };
    const juce::Colour track        { 0xff302b3f };
    const juce::Colour text         { 0xffe8e4f0 };
    const juce::Colour textDim      { 0xff8d86a0 };

    // One accent per functional group so the eye finds sections instantly.
    const juce::Colour osc          { 0xff4fd1c5 };   // teal
    const juce::Colour filter       { 0xfff6ad55 };   // amber
    const juce::Colour env          { 0xfff687b3 };   // pink
    const juce::Colour lfo          { 0xff9f7aea };   // violet
    const juce::Colour fx           { 0xff63b3ed };   // blue
    const juce::Colour voice        { 0xffa0d468 };   // green
}

class MathiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MathiLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool isDown, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;

    void drawPopupMenuSectionHeader (juce::Graphics&, const juce::Rectangle<int>&, const juce::String&) override;

    juce::Font getLabelFont (juce::Label&) override;
};

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "MathiLookAndFeel.h"

// A rotary knob whose caption turns into the live value while hovered or dragged,
// so the panel stays uncluttered but exact values are one glance away.
class ValueKnob : public juce::Slider
{
public:
    ValueKnob() : juce::Slider (RotaryHorizontalVerticalDrag, NoTextBox) {}
    juce::Label* caption = nullptr;
    juce::String name;

    void mouseEnter (const juce::MouseEvent& e) override { Slider::mouseEnter (e); hovering = true;  refresh(); }
    void mouseExit  (const juce::MouseEvent& e) override { Slider::mouseExit (e);  hovering = false; refresh(); }
    void valueChanged() override { refresh(); }
    void refresh()
    {
        if (caption != nullptr)
            caption->setText (hovering || isMouseButtonDown() ? getTextFromValue (getValue()) : name,
                              juce::dontSendNotification);
    }

private:
    bool hovering = false;
};

// Draws an ADSR shape from four parameters; purely visual feedback.
class EnvelopeDisplay : public juce::Component, private juce::Timer
{
public:
    EnvelopeDisplay (juce::AudioProcessorValueTreeState& s, const char* a, const char* d, const char* su,
                     const char* r, juce::Colour colour);
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    std::atomic<float> *pa, *pd, *ps, *pr;
    float last[4] {};
    juce::Colour colour;
};

// A titled panel holding a grid of parameter-bound controls (knobs, menus, switches).
class Section : public juce::Component
{
public:
    Section (const juce::String& title, juce::Colour accent, juce::AudioProcessorValueTreeState& state);

    void addKnob   (const char* paramID, const juce::String& label, int row);
    void addChoice (const char* paramID, const juce::String& label, int row, float cells = 2.0f);
    void addToggle (const char* paramID, const juce::String& label, int row);
    void setTopComponent (std::unique_ptr<juce::Component> c, int height);

    // Grey out a group of controls when a toggle is off (e.g. Osc B disabled).
    void bindEnableToggle (const char* toggleParamID);

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int titleHeight = 26;
    static constexpr int rowHeight   = 70;

private:
    struct Item
    {
        std::unique_ptr<juce::Component> control;
        std::unique_ptr<juce::Label> caption;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAtt;
        int row = 0;
        float cells = 1.0f;
        bool isToggle = false;
        juce::String paramID;
    };

    juce::Label* makeCaption (const juce::String& text);

    juce::String title;
    juce::Colour accent;
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Item> items;
    std::unique_ptr<juce::Component> topComponent;
    int topHeight = 0;

    std::unique_ptr<juce::ParameterAttachment> enableAttachment;
};

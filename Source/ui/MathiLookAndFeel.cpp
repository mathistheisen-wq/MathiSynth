#include "MathiLookAndFeel.h"

using namespace juce;

MathiLookAndFeel::MathiLookAndFeel()
{
    setColour (ResizableWindow::backgroundColourId, Theme::background);
    setColour (Label::textColourId, Theme::text);
    setColour (ComboBox::backgroundColourId, Theme::track);
    setColour (ComboBox::textColourId, Theme::text);
    setColour (ComboBox::arrowColourId, Theme::textDim);
    setColour (ComboBox::outlineColourId, Colours::transparentBlack);
    setColour (PopupMenu::backgroundColourId, Theme::panel);
    setColour (PopupMenu::textColourId, Theme::text);
    setColour (PopupMenu::highlightedBackgroundColourId, Theme::track.brighter (0.2f));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (TextButton::buttonColourId, Theme::track);
    setColour (TextButton::textColourOffId, Theme::text);
    setColour (AlertWindow::backgroundColourId, Theme::panel);
    setColour (AlertWindow::textColourId, Theme::text);
    setColour (TextEditor::backgroundColourId, Theme::track);
    setColour (TextEditor::textColourId, Theme::text);
    setColour (TextEditor::outlineColourId, Theme::panelEdge);
}

void MathiLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, Slider& s)
{
    const auto accent = s.findColour (Slider::rotarySliderFillColourId);
    const auto bounds = Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float lineW = jmax (2.5f, radius * 0.16f);
    const float arcR = radius - lineW * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;

    Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Theme::track);
    g.strokePath (track, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    // Value arc grows from 12 o'clock for bipolar params (env amount, LFO amount), else from the start.
    const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, jmin (from, angle), jmax (from, angle), true);
    g.setColour (s.isEnabled() ? accent : accent.withSaturation (0.1f).darker());
    g.strokePath (value, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    // Knob body + pointer
    const float bodyR = arcR - lineW * 1.1f;
    g.setGradientFill (ColourGradient (Theme::panel.brighter (0.25f), centre.x, centre.y - bodyR,
                                       Theme::panel.darker (0.3f), centre.x, centre.y + bodyR, false));
    g.fillEllipse (Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (centre));

    const Point<float> tip = centre.getPointOnCircumference (bodyR * 0.78f, angle);
    const Point<float> base = centre.getPointOnCircumference (bodyR * 0.25f, angle);
    g.setColour (Theme::text);
    g.drawLine ({ base, tip }, jmax (1.5f, lineW * 0.55f));
}

void MathiLookAndFeel::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box)
{
    const auto r = Rectangle<int> (w, h).toFloat().reduced (0.5f);
    g.setColour (box.findColour (ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 5.0f);
    if (box.hasKeyboardFocus (false) || box.isMouseOver (true))
    {
        g.setColour (Theme::textDim.withAlpha (0.5f));
        g.drawRoundedRectangle (r, 5.0f, 1.0f);
    }

    Path arrow;
    const float ax = (float) w - 12.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.0f, ax + 4.0f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (box.findColour (ComboBox::arrowColourId));
    g.fillPath (arrow);
}

Font MathiLookAndFeel::getComboBoxFont (ComboBox&) { return Font (FontOptions (13.0f)); }

void MathiLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (6, 0, box.getWidth() - 22, box.getHeight());
    label.setFont (getComboBoxFont (box));
    label.setJustificationType (Justification::centredLeft);
}

void MathiLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool highlighted, bool)
{
    // Pill switch centred in the button; the label is drawn by the owning Section.
    const auto accent = b.findColour (ToggleButton::tickColourId);
    const auto area = b.getLocalBounds().toFloat();
    const auto pill = Rectangle<float> (34.0f, 18.0f).withCentre (area.getCentre());
    const bool on = b.getToggleState();

    g.setColour (on ? accent.withAlpha (0.85f) : Theme::track.brighter (highlighted ? 0.15f : 0.0f));
    g.fillRoundedRectangle (pill, 9.0f);
    const float knobX = on ? pill.getRight() - 16.0f : pill.getX() + 2.0f;
    g.setColour (on ? Colours::white : Theme::textDim);
    g.fillEllipse (knobX, pill.getY() + 2.0f, 14.0f, 14.0f);
}

void MathiLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour& c, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto col = c;
    if (down) col = col.brighter (0.2f); else if (highlighted) col = col.brighter (0.1f);
    g.setColour (col);
    g.fillRoundedRectangle (r, 5.0f);
}

void MathiLookAndFeel::drawPopupMenuSectionHeader (Graphics& g, const Rectangle<int>& area, const String& name)
{
    g.setColour (Theme::textDim);
    g.setFont (Font (FontOptions (12.0f, Font::bold)));
    g.drawText (name.toUpperCase(), area.reduced (10, 0), Justification::bottomLeft, true);
}

Font MathiLookAndFeel::getLabelFont (Label& l)
{
    return l.getFont();
}

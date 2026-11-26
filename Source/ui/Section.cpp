#include "Section.h"

using namespace juce;
using APVTS = AudioProcessorValueTreeState;

//==============================================================================
EnvelopeDisplay::EnvelopeDisplay (APVTS& s, const char* a, const char* d, const char* su, const char* r, Colour c)
    : pa (s.getRawParameterValue (a)), pd (s.getRawParameterValue (d)),
      ps (s.getRawParameterValue (su)), pr (s.getRawParameterValue (r)), colour (c)
{
    setInterceptsMouseClicks (false, false);
    timerCallback();   // draw the current shape immediately
    startTimerHz (20);
}

void EnvelopeDisplay::timerCallback()
{
    const float now[4] { pa->load(), pd->load(), ps->load(), pr->load() };
    if (std::memcmp (now, last, sizeof (now)) != 0)
    {
        std::memcpy (last, now, sizeof (now));
        repaint();
    }
}

void EnvelopeDisplay::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (Theme::background.withAlpha (0.6f));
    g.fillRoundedRectangle (r, 4.0f);
    r = r.reduced (6.0f, 5.0f);

    // Each stage gets a width proportional to sqrt(time) so short and long envelopes both read well.
    auto w = [] (float t) { return std::sqrt (jmax (0.0005f, t)); };
    const float a = w (last[0]), d = w (last[1]), rel = w (last[3]), hold = 0.6f;
    const float total = a + d + hold + rel;
    const float sx = r.getWidth() / total;
    const float s = last[2];

    const float x0 = r.getX(), yB = r.getBottom(), yT = r.getY();
    const float sy = yB - s * r.getHeight();
    const float x1 = x0 + a * sx, x2 = x1 + d * sx, x3 = x2 + hold * sx, x4 = x3 + rel * sx;

    Path p;
    p.startNewSubPath (x0, yB);
    p.lineTo (x1, yT);
    p.quadraticTo (x1 + (x2 - x1) * 0.15f, sy, x2, sy);    // curved decay
    p.lineTo (x3, sy);
    p.quadraticTo (x3 + (x4 - x3) * 0.15f, yB, x4, yB);    // curved release

    Path fill (p);
    fill.lineTo (x0, yB);
    fill.closeSubPath();
    g.setGradientFill (ColourGradient (colour.withAlpha (0.35f), 0, yT, colour.withAlpha (0.02f), 0, yB, false));
    g.fillPath (fill);
    g.setColour (colour);
    g.strokePath (p, PathStrokeType (1.6f, PathStrokeType::curved, PathStrokeType::rounded));
}

//==============================================================================
Section::Section (const String& t, Colour a, APVTS& s) : title (t), accent (a), apvts (s) {}

Label* Section::makeCaption (const String& text)
{
    auto* l = new Label ({}, text);
    l->setJustificationType (Justification::centred);
    l->setFont (Font (FontOptions (11.5f)));
    l->setColour (Label::textColourId, Theme::textDim);
    l->setInterceptsMouseClicks (false, false);
    l->setMinimumHorizontalScale (0.7f);
    addAndMakeVisible (l);
    return l;
}

void Section::addKnob (const char* id, const String& label, int row)
{
    Item it;
    auto knob = std::make_unique<ValueKnob>();
    knob->setColour (Slider::rotarySliderFillColourId, accent);
    knob->setTooltip (apvts.getParameter (id)->getName (64));
    it.caption.reset (makeCaption (label));
    knob->caption = it.caption.get();
    knob->name = label;
    addAndMakeVisible (*knob);
    it.sliderAtt = std::make_unique<APVTS::SliderAttachment> (apvts, id, *knob);

    // Double-click returns to the parameter's default value.
    auto* p = apvts.getParameter (id);
    knob->setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    knob->refresh();

    it.control = std::move (knob);
    it.row = row;
    it.paramID = id;
    items.push_back (std::move (it));
}

void Section::addChoice (const char* id, const String& label, int row, float cells)
{
    Item it;
    auto box = std::make_unique<ComboBox>();
    if (auto* p = dynamic_cast<AudioParameterChoice*> (apvts.getParameter (id)))
        box->addItemList (p->choices, 1);
    addAndMakeVisible (*box);
    it.comboAtt = std::make_unique<APVTS::ComboBoxAttachment> (apvts, id, *box);
    it.caption.reset (makeCaption (label));
    it.control = std::move (box);
    it.row = row;
    it.cells = cells;
    it.paramID = id;
    items.push_back (std::move (it));
}

void Section::addToggle (const char* id, const String& label, int row)
{
    Item it;
    auto b = std::make_unique<ToggleButton>();
    b->setColour (ToggleButton::tickColourId, accent);
    addAndMakeVisible (*b);
    it.buttonAtt = std::make_unique<APVTS::ButtonAttachment> (apvts, id, *b);
    it.caption.reset (makeCaption (label));
    it.control = std::move (b);
    it.row = row;
    it.isToggle = true;
    it.paramID = id;
    items.push_back (std::move (it));
}

void Section::setTopComponent (std::unique_ptr<Component> c, int height)
{
    topComponent = std::move (c);
    topHeight = height;
    addAndMakeVisible (*topComponent);
}

void Section::bindEnableToggle (const char* toggleID)
{
    auto* p = apvts.getParameter (toggleID);
    enableAttachment = std::make_unique<ParameterAttachment> (*p, [this, toggleID = String (toggleID)] (float v)
    {
        for (auto& it : items)
            if (it.paramID != toggleID)
            {
                it.control->setEnabled (v > 0.5f);
                it.control->setAlpha (v > 0.5f ? 1.0f : 0.4f);
            }
    });
    enableAttachment->sendInitialUpdate();
}

void Section::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (Theme::panel);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (Theme::panelEdge);
    g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

    // Title strip: accent dot + caps label
    g.setColour (accent);
    g.fillEllipse (12.0f, 10.0f, 7.0f, 7.0f);
    g.setFont (Font (FontOptions (12.5f, Font::bold)));
    g.drawText (title.toUpperCase(), 26, 4, getWidth() - 36, titleHeight - 6, Justification::centredLeft);
}

void Section::resized()
{
    auto area = getLocalBounds().reduced (8, 0);
    area.removeFromTop (titleHeight);
    area.removeFromBottom (6);

    if (topComponent != nullptr)
        topComponent->setBounds (area.removeFromTop (topHeight).reduced (2, 0));

    int numRows = 0;
    for (auto& it : items) numRows = jmax (numRows, it.row + 1);
    if (numRows == 0) return;

    const int rowH = jmin (rowHeight, area.getHeight() / numRows);

    for (int row = 0; row < numRows; ++row)
    {
        auto rowArea = area.removeFromTop (rowH);
        float cells = 0.0f;
        for (auto& it : items) if (it.row == row) cells += it.cells;
        if (cells <= 0.0f) continue;

        // Cells are capped so sparse rows stay left-aligned instead of stretching the knobs.
        const float cellW = jmin (68.0f, (float) rowArea.getWidth() / cells);
        float x = (float) rowArea.getX();

        for (auto& it : items)
        {
            if (it.row != row) continue;
            const auto cell = Rectangle<float> (x, (float) rowArea.getY(), cellW * it.cells, (float) rowH).toNearestInt();
            x += cellW * it.cells;

            auto c = cell.reduced (2, 0);
            it.caption->setBounds (c.removeFromBottom (16));

            if (dynamic_cast<ComboBox*> (it.control.get()) != nullptr || it.isToggle)
                it.control->setBounds (c.withSizeKeepingCentre (c.getWidth() - 4, 24));
            else
            {
                const int d = jmin (c.getWidth(), c.getHeight());
                it.control->setBounds (c.withSizeKeepingCentre (d, d));
            }
        }
    }
}

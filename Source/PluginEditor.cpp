#include "PluginEditor.h"

using namespace juce;
using namespace ParamID;

Section& MathiSynthEditor::makeSection (const String& title, Colour accent)
{
    sections.push_back (std::make_unique<Section> (title, accent, processor.apvts));
    content.addAndMakeVisible (*sections.back());
    return *sections.back();
}

MathiSynthEditor::MathiSynthEditor (MathiSynthProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);

    // ---- Oscillators ---------------------------------------------------------------
    auto buildOsc = [this] (const String& name, bool a) -> Section*
    {
        auto& s = makeSection (name, Theme::osc);
        s.addToggle (a ? oscAOn : oscBOn,         "On", 0);
        s.addChoice (a ? oscAWave : oscBWave,     "Wave", 0, 2.0f);
        s.addKnob   (a ? oscAOctave : oscBOctave, "Octave", 0);
        s.addKnob   (a ? oscASemi : oscBSemi,     "Semi", 0);
        s.addKnob   (a ? oscAFine : oscBFine,     "Fine", 1);
        s.addKnob   (a ? oscALevel : oscBLevel,   "Level", 1);
        s.addKnob   (a ? oscAUnison : oscBUnison, "Unison", 1);
        s.addKnob   (a ? oscADetune : oscBDetune, "Detune", 1);
        s.addKnob   (a ? oscAWidth : oscBWidth,   "Width", 1);
        s.bindEnableToggle (a ? oscAOn : oscBOn);
        return &s;
    };
    oscA = buildOsc ("Oscillator A", true);
    oscB = buildOsc ("Oscillator B", false);

    subNoise = &makeSection ("Sub / Noise", Theme::osc);
    subNoise->addKnob   (subLevel, "Sub", 0);
    subNoise->addChoice (subWave,  "Sub Wave", 0, 2.0f);
    subNoise->addKnob   (noiseLevel, "Noise", 1);

    // ---- Filter + envelopes -----------------------------------------------------------
    filter = &makeSection ("Filter", Theme::filter);
    filter->addChoice (filterType,   "Type", 0, 2.0f);
    filter->addKnob   (cutoff,       "Cutoff", 0);
    filter->addKnob   (resonance,    "Reso", 0);
    filter->addKnob   (drive,        "Drive", 0);
    filter->addKnob   (filterEnvAmt, "Env Amt", 1);
    filter->addKnob   (keyTrack,     "Key Trk", 1);
    filter->addKnob   (velToFilter,  "Vel", 1);

    auto buildEnv = [this] (const String& name, const char* a, const char* d, const char* s, const char* r)
    {
        auto& sec = makeSection (name, Theme::env);
        sec.setTopComponent (std::make_unique<EnvelopeDisplay> (processor.apvts, a, d, s, r, Theme::env), 62);
        sec.addKnob (a, "Attack", 0);
        sec.addKnob (d, "Decay", 0);
        sec.addKnob (s, "Sustain", 0);
        sec.addKnob (r, "Release", 0);
        return &sec;
    };
    filterEnv = buildEnv ("Filter Envelope", fAttack, fDecay, fSustain, fRelease);
    ampEnv    = buildEnv ("Amp Envelope",    aAttack, aDecay, aSustain, aRelease);

    // ---- Modulation + voice --------------------------------------------------------------
    auto buildLfo = [this] (const String& name, bool first)
    {
        auto& s = makeSection (name, Theme::lfo);
        s.addChoice (first ? lfo1Shape : lfo2Shape,   "Shape", 0, 2.0f);
        s.addChoice (first ? lfo1Dest : lfo2Dest,     "Destination", 0, 2.0f);
        s.addKnob   (first ? lfo1Amount : lfo2Amount, "Amount", 0);
        s.addKnob   (first ? lfo1Rate : lfo2Rate,     "Rate", 1);
        s.addToggle (first ? lfo1Sync : lfo2Sync,     "Sync", 1);
        s.addChoice (first ? lfo1Div : lfo2Div,       "Division", 1, 2.0f);
        return &s;
    };
    lfo1 = buildLfo ("LFO 1  (mod wheel)", true);
    lfo2 = buildLfo ("LFO 2", false);

    voice = &makeSection ("Voice", Theme::voice);
    voice->addChoice (voiceMode, "Mode", 0, 2.0f);
    voice->addKnob   (glide,     "Glide", 0);
    voice->addKnob   (bendRange, "Bend", 1);
    voice->addKnob   (velToAmp,  "Vel > Amp", 1);

    // ---- Effects -------------------------------------------------------------------------
    chorus = &makeSection ("Chorus", Theme::fx);
    chorus->addKnob (chorusMix,   "Mix", 0);
    chorus->addKnob (chorusRate,  "Rate", 0);
    chorus->addKnob (chorusDepth, "Depth", 0);

    delay = &makeSection ("Delay", Theme::fx);
    delay->addKnob   (delayMix,      "Mix", 0);
    delay->addChoice (delayDiv,      "Time", 0, 1.6f);
    delay->addKnob   (delayFeedback, "Feedback", 0);
    delay->addKnob   (delayTone,     "Tone", 0);
    delay->addToggle (delayPingPong, "Ping-Pong", 0);

    reverb = &makeSection ("Reverb", Theme::fx);
    reverb->addKnob (reverbMix,   "Mix", 0);
    reverb->addKnob (reverbSize,  "Size", 0);
    reverb->addKnob (reverbDamp,  "Damping", 0);
    reverb->addKnob (reverbWidth, "Width", 0);

    // ---- Header --------------------------------------------------------------------------
    masterSection = std::make_unique<Section> ("Master", Theme::text, processor.apvts);
    masterSection->addKnob (masterGain, "Volume", 0);
    content.addAndMakeVisible (*masterSection);

    presetBox.setTextWhenNothingSelected ("Init");
    presetBox.setTooltip ("Factory presets by role. Save your own with the Save button.");
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0 && idx != processor.presets.getCurrentIndex())
        {
            processor.presets.load (idx);
            shownPresetIndex = idx;
        }
    };
    prevButton.onClick = [this] { processor.presets.loadPrevious(); };
    nextButton.onClick = [this] { processor.presets.loadNext(); };
    saveButton.onClick = [this] { showSaveDialog(); };

    voiceLabel.setJustificationType (Justification::centredRight);
    voiceLabel.setFont (Font (FontOptions (12.0f)));
    voiceLabel.setColour (Label::textColourId, Theme::textDim);

    for (Component* c : std::initializer_list<Component*> { &presetBox, &prevButton, &nextButton, &saveButton, &voiceLabel })
        content.addAndMakeVisible (c);

    rebuildPresetMenu();

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) designWidth / designHeight);
    setResizeLimits (designWidth * 6 / 10, designHeight * 6 / 10, designWidth * 2, designHeight * 2);
    setSize (designWidth, designHeight);

    timerCallback();   // show the current preset name right away
    startTimerHz (10);
}

MathiSynthEditor::~MathiSynthEditor()
{
    setLookAndFeel (nullptr);
}

void MathiSynthEditor::rebuildPresetMenu()
{
    presetBox.clear (dontSendNotification);
    const auto& entries = processor.presets.getEntries();
    String currentCat;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (entries[i].category != currentCat)
        {
            currentCat = entries[i].category;
            presetBox.addSectionHeading (currentCat);
        }
        presetBox.addItem (entries[i].name, (int) i + 1);
    }
    shownPresetCount = entries.size();
    shownPresetIndex = -1;   // force refresh of the selection
}

void MathiSynthEditor::timerCallback()
{
    if (processor.presets.getEntries().size() != shownPresetCount)
        rebuildPresetMenu();

    const int idx = processor.presets.getCurrentIndex();
    if (idx != shownPresetIndex)
    {
        presetBox.setSelectedId (idx + 1, dontSendNotification);
        shownPresetIndex = idx;
    }

    voiceLabel.setText (String (processor.activeVoices.load()) + " voices", dontSendNotification);
}

void MathiSynthEditor::showSaveDialog()
{
    auto* w = new AlertWindow ("Save Preset", "Name your preset. It is stored in\n"
                                   + PresetManager::getUserPresetFolder().getFullPathName(),
                               MessageBoxIconType::NoIcon, this);
    w->setLookAndFeel (&lnf);
    w->addTextEditor ("name", processor.presets.getCurrentName() + " copy", "Name");
    w->addComboBox ("category", getPresetCategories(), "Category");
    if (auto* cb = w->getComboBoxComponent ("category"))
    {
        const auto& e = processor.presets.getEntries();
        const int cur = processor.presets.getCurrentIndex();
        const auto cat = isPositiveAndBelow (cur, (int) e.size()) ? e[(size_t) cur].category.replace ("User ", "") : String();
        const int ci = getPresetCategories().indexOf (cat);
        cb->setSelectedItemIndex (jmax (0, ci));
    }
    w->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    w->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

    w->enterModalState (true, ModalCallbackFunction::create ([this, w] (int result)
    {
        if (result == 1)
        {
            const auto name = w->getTextEditorContents ("name");
            const auto cat = w->getComboBoxComponent ("category")->getText();
            if (! processor.presets.saveUserPreset (name, cat))
                AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon, "Save Preset",
                                                  "Could not write the preset file.");
        }
    }), true);
}

void MathiSynthEditor::paint (Graphics& g)
{
    g.fillAll (Theme::background);
}

void MathiSynthEditor::resized()
{
    const float scale = (float) getWidth() / (float) designWidth;
    content.setBounds (0, 0, designWidth, designHeight);
    content.setTransform (AffineTransform::scale (scale));
}

//==============================================================================
void MathiSynthEditor::Content::paint (Graphics& g)
{
    g.fillAll (Theme::background);

    // Logo
    g.setColour (Theme::text);
    g.setFont (Font (FontOptions (24.0f, Font::bold)));
    g.drawText ("Mathi", 20, 12, 80, 30, Justification::centredLeft);
    g.setColour (Theme::osc);
    g.drawText ("Synth", 86, 12, 80, 30, Justification::centredLeft);
    g.setColour (Theme::textDim);
    g.setFont (Font (FontOptions (11.0f)));
    g.drawText ("v" + String (MATHISYNTH_VERSION_STRING), 160, 18, 60, 20, Justification::centredLeft);
}

void MathiSynthEditor::Content::resized()
{
    auto& o = owner;
    auto r = getLocalBounds().reduced (10);

    // Header
    auto header = r.removeFromTop (44);
    auto centre = header.withSizeKeepingCentre (420, 30);
    o.prevButton.setBounds (centre.removeFromLeft (30));
    o.saveButton.setBounds (centre.removeFromRight (60));
    centre.removeFromRight (6);
    o.nextButton.setBounds (centre.removeFromRight (30));
    o.presetBox.setBounds (centre.reduced (6, 0));
    o.voiceLabel.setBounds (header.removeFromRight (120).withTrimmedRight (6).withSizeKeepingCentre (110, 20));

    r.removeFromTop (6);
    const int gap = 8;
    const int rowH = Section::titleHeight + 2 * Section::rowHeight + 10;   // two control rows

    auto layoutRow = [gap] (Rectangle<int> row, std::initializer_list<std::pair<Section*, float>> parts)
    {
        float total = 0.0f;
        for (auto& p : parts) total += p.second;
        const float avail = (float) (row.getWidth() - gap * ((int) parts.size() - 1));
        for (auto& p : parts)
        {
            p.first->setBounds (row.removeFromLeft (roundToInt (avail * p.second / total)));
            row.removeFromLeft (gap);
        }
    };

    layoutRow (r.removeFromTop (rowH), { { o.oscA, 5.0f }, { o.oscB, 5.0f }, { o.subNoise, 2.6f } });
    r.removeFromTop (gap);
    layoutRow (r.removeFromTop (rowH), { { o.filter, 4.6f }, { o.filterEnv, 4.0f }, { o.ampEnv, 4.0f } });
    r.removeFromTop (gap);
    layoutRow (r.removeFromTop (rowH), { { o.lfo1, 4.4f }, { o.lfo2, 4.4f }, { o.voice, 3.0f } });
    r.removeFromTop (gap);

    auto fxRow = r;
    auto masterArea = fxRow.removeFromRight (96);
    fxRow.removeFromRight (gap);
    layoutRow (fxRow, { { o.chorus, 3.0f }, { o.delay, 5.2f }, { o.reverb, 4.0f } });
    o.masterSection->setBounds (masterArea);
}

#pragma once

#include "PluginProcessor.h"
#include "ui/MathiLookAndFeel.h"
#include "ui/Section.h"

// The whole UI is laid out at a fixed design size inside `content`, which is then scaled
// to the editor size — so resizing in FL Studio / Ableton keeps proportions crisp.
class MathiSynthEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MathiSynthEditor (MathiSynthProcessor&);
    ~MathiSynthEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int designWidth  = 1080;
    static constexpr int designHeight = 740;

private:
    class Content : public juce::Component
    {
    public:
        explicit Content (MathiSynthEditor& o) : owner (o) {}
        void paint (juce::Graphics&) override;
        void resized() override;
        MathiSynthEditor& owner;
    };

    void timerCallback() override;
    void rebuildPresetMenu();
    void showSaveDialog();
    Section& makeSection (const juce::String& title, juce::Colour accent);

    MathiSynthProcessor& processor;
    MathiLookAndFeel lnf;
    juce::TooltipWindow tooltips { this, 600 };

    Content content { *this };

    // Header
    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" }, saveButton { "Save" };
    juce::Label voiceLabel;
    std::unique_ptr<Section> masterSection;

    // Sections (owned in display order)
    std::vector<std::unique_ptr<Section>> sections;
    Section *oscA{}, *oscB{}, *subNoise{}, *filter{}, *filterEnv{}, *ampEnv{},
            *lfo1{}, *lfo2{}, *voice{}, *chorus{}, *delay{}, *reverb{};

    int shownPresetIndex = -1;
    size_t shownPresetCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MathiSynthEditor)
};

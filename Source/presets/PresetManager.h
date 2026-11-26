#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FactoryPresets.h"

// Owns the preset list (factory + user files) and applies them to the APVTS.
// User presets are XML files in  <Documents>/MathiSynth/Presets/<Category>/<Name>.mathipreset
class PresetManager
{
public:
    struct Entry
    {
        juce::String name, category;
        int factoryIndex = -1;    // >= 0 for factory presets
        juce::File file;          // set for user presets
    };

    explicit PresetManager (juce::AudioProcessorValueTreeState& state);

    const std::vector<Entry>& getEntries() const { return entries; }
    int getCurrentIndex() const { return currentIndex; }
    juce::String getCurrentName() const;

    void load (int index);
    void loadNext()     { if (! entries.empty()) load ((currentIndex + 1) % (int) entries.size()); }
    void loadPrevious() { if (! entries.empty()) load ((currentIndex - 1 + (int) entries.size()) % (int) entries.size()); }

    bool saveUserPreset (const juce::String& name, const juce::String& category);
    void rescanUserPresets();

    static juce::File getUserPresetFolder();

    // Restores the "which preset is selected" marker after a project reload.
    void setCurrentIndexSilently (int i) { currentIndex = i; }

private:
    void applyFactory (const FactoryPreset& p);
    void resetToDefaults();

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<Entry> entries;
    int currentIndex = 0;
};

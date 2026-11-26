#pragma once

#include <juce_core/juce_core.h>
#include <utility>
#include <vector>

// A factory preset is a list of overrides on top of the parameter defaults.
// Values are in real units: Hz, seconds, dB, 0..1 for percent, choice *index*, 0/1 for toggles.
struct FactoryPreset
{
    juce::String name;
    juce::String category;     // Bass / Pluck / Lead / Pad / Texture
    std::vector<std::pair<const char*, float>> values;
};

const std::vector<FactoryPreset>& getFactoryPresets();
const juce::StringArray& getPresetCategories();

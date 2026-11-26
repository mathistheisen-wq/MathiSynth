#include "PresetManager.h"

namespace { const juce::String presetExtension = ".mathipreset"; }

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& state) : apvts (state)
{
    rescanUserPresets();
}

juce::File PresetManager::getUserPresetFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
               .getChildFile ("MathiSynth").getChildFile ("Presets");
}

void PresetManager::rescanUserPresets()
{
    entries.clear();

    // Factory presets, grouped by category in display order.
    const auto& factory = getFactoryPresets();
    for (const auto& cat : getPresetCategories())
        for (int i = 0; i < (int) factory.size(); ++i)
            if (factory[(size_t) i].category == cat)
                entries.push_back ({ factory[(size_t) i].name, cat, i, {} });

    // User presets (category = sub-folder name, shown as "User <Category>").
    const auto folder = getUserPresetFolder();
    if (folder.isDirectory())
    {
        auto files = folder.findChildFiles (juce::File::findFiles, true, "*" + presetExtension);
        files.sort();
        for (const auto& f : files)
        {
            auto parent = f.getParentDirectory();
            const auto cat = parent == folder ? juce::String ("User") : "User " + parent.getFileName();
            entries.push_back ({ f.getFileNameWithoutExtension(), cat, -1, f });
        }
    }

    currentIndex = juce::jlimit (0, juce::jmax (0, (int) entries.size() - 1), currentIndex);
}

juce::String PresetManager::getCurrentName() const
{
    if (juce::isPositiveAndBelow (currentIndex, (int) entries.size()))
        return entries[(size_t) currentIndex].name;
    return "Init";
}

void PresetManager::resetToDefaults()
{
    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());
}

void PresetManager::applyFactory (const FactoryPreset& preset)
{
    resetToDefaults();
    for (const auto& [id, value] : preset.values)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
        else
            jassertfalse;   // typo in a preset parameter ID
    }
}

void PresetManager::load (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) entries.size()))
        return;

    const auto& e = entries[(size_t) index];
    if (e.factoryIndex >= 0)
    {
        applyFactory (getFactoryPresets()[(size_t) e.factoryIndex]);
    }
    else if (auto xml = juce::parseXML (e.file))
    {
        // Apply parameter by parameter (rather than replaceState) so the host sees each change.
        resetToDefaults();
        if (auto* params = xml->getChildByName ("Params"))
            for (auto* el : params->getChildIterator())
                if (auto* p = apvts.getParameter (el->getStringAttribute ("id")))
                    p->setValueNotifyingHost (p->convertTo0to1 ((float) el->getDoubleAttribute ("value")));
    }

    currentIndex = index;
}

bool PresetManager::saveUserPreset (const juce::String& rawName, const juce::String& category)
{
    const auto name = juce::File::createLegalFileName (rawName.trim());
    if (name.isEmpty()) return false;

    auto folder = getUserPresetFolder();
    if (category.isNotEmpty()) folder = folder.getChildFile (juce::File::createLegalFileName (category));
    if (! folder.createDirectory()) return false;

    juce::XmlElement root ("MathiSynthPreset");
    root.setAttribute ("version", 1);
    root.setAttribute ("name", name);
    auto* params = root.createNewChildElement ("Params");
    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            auto* el = params->createNewChildElement ("P");
            el->setAttribute ("id", rp->paramID);
            el->setAttribute ("value", rp->convertFrom0to1 (rp->getValue()));
        }

    const auto file = folder.getChildFile (name + presetExtension);
    if (! root.writeTo (file)) return false;

    rescanUserPresets();
    for (int i = 0; i < (int) entries.size(); ++i)
        if (entries[(size_t) i].file == file) { currentIndex = i; break; }
    return true;
}

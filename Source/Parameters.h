#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Every automatable parameter ID lives here so the DSP, UI and presets share one source of truth.
// IDs are stable: renaming one breaks saved projects and presets, so add new ones instead.
namespace ParamID
{
    // Oscillator A / B (same layout, prefix "oscA" / "oscB")
    inline constexpr const char* oscAOn      = "oscAOn";
    inline constexpr const char* oscAWave    = "oscAWave";
    inline constexpr const char* oscAOctave  = "oscAOctave";
    inline constexpr const char* oscASemi    = "oscASemi";
    inline constexpr const char* oscAFine    = "oscAFine";
    inline constexpr const char* oscALevel   = "oscALevel";
    inline constexpr const char* oscAUnison  = "oscAUnison";
    inline constexpr const char* oscADetune  = "oscADetune";
    inline constexpr const char* oscAWidth   = "oscAWidth";

    inline constexpr const char* oscBOn      = "oscBOn";
    inline constexpr const char* oscBWave    = "oscBWave";
    inline constexpr const char* oscBOctave  = "oscBOctave";
    inline constexpr const char* oscBSemi    = "oscBSemi";
    inline constexpr const char* oscBFine    = "oscBFine";
    inline constexpr const char* oscBLevel   = "oscBLevel";
    inline constexpr const char* oscBUnison  = "oscBUnison";
    inline constexpr const char* oscBDetune  = "oscBDetune";
    inline constexpr const char* oscBWidth   = "oscBWidth";

    // Sub / noise
    inline constexpr const char* subLevel    = "subLevel";
    inline constexpr const char* subWave     = "subWave";
    inline constexpr const char* noiseLevel  = "noiseLevel";

    // Voice
    inline constexpr const char* voiceMode   = "voiceMode";   // Poly / Mono / Legato
    inline constexpr const char* glide       = "glide";
    inline constexpr const char* bendRange   = "bendRange";
    inline constexpr const char* velToAmp    = "velToAmp";
    inline constexpr const char* velToFilter = "velToFilter";

    // Filter
    inline constexpr const char* filterType  = "filterType";
    inline constexpr const char* cutoff      = "cutoff";
    inline constexpr const char* resonance   = "resonance";
    inline constexpr const char* drive       = "drive";
    inline constexpr const char* filterEnvAmt= "filterEnvAmt";
    inline constexpr const char* keyTrack    = "keyTrack";

    // Envelopes
    inline constexpr const char* fAttack     = "fAttack";
    inline constexpr const char* fDecay      = "fDecay";
    inline constexpr const char* fSustain    = "fSustain";
    inline constexpr const char* fRelease    = "fRelease";
    inline constexpr const char* aAttack     = "aAttack";
    inline constexpr const char* aDecay      = "aDecay";
    inline constexpr const char* aSustain    = "aSustain";
    inline constexpr const char* aRelease    = "aRelease";

    // LFOs
    inline constexpr const char* lfo1Shape   = "lfo1Shape";
    inline constexpr const char* lfo1Rate    = "lfo1Rate";
    inline constexpr const char* lfo1Sync    = "lfo1Sync";
    inline constexpr const char* lfo1Div     = "lfo1Div";
    inline constexpr const char* lfo1Amount  = "lfo1Amount";
    inline constexpr const char* lfo1Dest    = "lfo1Dest";
    inline constexpr const char* lfo2Shape   = "lfo2Shape";
    inline constexpr const char* lfo2Rate    = "lfo2Rate";
    inline constexpr const char* lfo2Sync    = "lfo2Sync";
    inline constexpr const char* lfo2Div     = "lfo2Div";
    inline constexpr const char* lfo2Amount  = "lfo2Amount";
    inline constexpr const char* lfo2Dest    = "lfo2Dest";

    // Effects
    inline constexpr const char* chorusMix   = "chorusMix";
    inline constexpr const char* chorusRate  = "chorusRate";
    inline constexpr const char* chorusDepth = "chorusDepth";
    inline constexpr const char* delayMix    = "delayMix";
    inline constexpr const char* delayDiv    = "delayDiv";
    inline constexpr const char* delayFeedback = "delayFeedback";
    inline constexpr const char* delayPingPong = "delayPingPong";
    inline constexpr const char* delayTone   = "delayTone";
    inline constexpr const char* reverbMix   = "reverbMix";
    inline constexpr const char* reverbSize  = "reverbSize";
    inline constexpr const char* reverbDamp  = "reverbDamp";
    inline constexpr const char* reverbWidth = "reverbWidth";

    // Master
    inline constexpr const char* masterGain  = "masterGain";
}

namespace Choices
{
    const juce::StringArray waves      { "Sine", "Triangle", "Saw", "Square", "Pulse 25%" };
    const juce::StringArray subWaves   { "Sine", "Triangle", "Square" };
    const juce::StringArray voiceModes { "Poly", "Mono", "Legato" };
    const juce::StringArray filterTypes{ "Low 24", "Low 12", "Band 12", "High 12" };
    const juce::StringArray lfoShapes  { "Sine", "Triangle", "Saw", "Square", "S&H" };
    const juce::StringArray lfoDests   { "Off", "Cutoff", "Pitch", "Amp", "Pan", "Detune", "Resonance" };
    const juce::StringArray syncDivs   { "4/1", "2/1", "1/1", "1/2", "1/4", "1/4T", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
    const juce::StringArray delayDivs  { "1/32", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4T", "1/4", "1/4D", "1/2" };
}

// Length of each sync division in quarter-note beats (matches the arrays above).
namespace SyncBeats
{
    inline constexpr float lfo[]   { 16.0f, 8.0f, 4.0f, 2.0f, 1.0f, 2.0f / 3.0f, 0.5f, 1.0f / 3.0f, 0.25f, 1.0f / 6.0f, 0.125f };
    inline constexpr float delay[] { 0.125f, 0.25f, 0.375f, 1.0f / 3.0f, 0.5f, 0.75f, 2.0f / 3.0f, 1.0f, 1.5f, 2.0f };
}

enum class LfoDest { Off = 0, Cutoff, Pitch, Amp, Pan, Detune, Resonance };

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

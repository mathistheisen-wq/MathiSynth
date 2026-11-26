#include "Parameters.h"

using namespace juce;

namespace
{
    using Layout = AudioProcessorValueTreeState::ParameterLayout;

    NormalisableRange<float> skewed (float lo, float hi, float centre)
    {
        NormalisableRange<float> r (lo, hi);
        r.setSkewForCentre (centre);
        return r;
    }

    void addFloat (Layout& l, const char* id, const String& name, NormalisableRange<float> range,
                   float def, const String& unit = {}, int decimals = 2)
    {
        l.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, name, range, def,
            AudioParameterFloatAttributes()
                .withLabel (unit)
                .withStringFromValueFunction ([decimals, unit] (float v, int)
                {
                    if (unit == "s" && v < 1.0f) return String (roundToInt (v * 1000.0f)) + " ms";
                    if (unit == "Hz" && v >= 1000.0f) return String (v / 1000.0f, 2) + " kHz";
                    if (unit == "%") return String (roundToInt (v * 100.0f)) + " %";
                    return String (v, decimals) + (unit.isNotEmpty() ? " " + unit : String());
                })));
    }

    void addPercent (Layout& l, const char* id, const String& name, float def, bool bipolar = false)
    {
        addFloat (l, id, name, { bipolar ? -1.0f : 0.0f, 1.0f }, def, "%");
    }

    void addInt (Layout& l, const char* id, const String& name, int lo, int hi, int def)
    {
        l.add (std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, name, lo, hi, def));
    }

    void addChoice (Layout& l, const char* id, const String& name, const StringArray& c, int def)
    {
        l.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, c, def));
    }

    void addBool (Layout& l, const char* id, const String& name, bool def)
    {
        l.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    }

    void addOsc (Layout& l, bool isA)
    {
        const String n = isA ? "Osc A " : "Osc B ";
        addBool    (l, isA ? ParamID::oscAOn     : ParamID::oscBOn,     n + "On", isA);
        addChoice  (l, isA ? ParamID::oscAWave   : ParamID::oscBWave,   n + "Wave", Choices::waves, 2);
        addInt     (l, isA ? ParamID::oscAOctave : ParamID::oscBOctave, n + "Octave", -3, 3, 0);
        addInt     (l, isA ? ParamID::oscASemi   : ParamID::oscBSemi,   n + "Semi", -12, 12, 0);
        addFloat   (l, isA ? ParamID::oscAFine   : ParamID::oscBFine,   n + "Fine", { -100.0f, 100.0f }, 0.0f, "ct", 0);
        addPercent (l, isA ? ParamID::oscALevel  : ParamID::oscBLevel,  n + "Level", 0.8f);
        addInt     (l, isA ? ParamID::oscAUnison : ParamID::oscBUnison, n + "Unison", 1, 7, 1);
        addPercent (l, isA ? ParamID::oscADetune : ParamID::oscBDetune, n + "Detune", 0.2f);
        addPercent (l, isA ? ParamID::oscAWidth  : ParamID::oscBWidth,  n + "Width", 0.5f);
    }

    void addLfo (Layout& l, bool first)
    {
        const String n = first ? "LFO 1 " : "LFO 2 ";
        addChoice  (l, first ? ParamID::lfo1Shape  : ParamID::lfo2Shape,  n + "Shape", Choices::lfoShapes, 0);
        addFloat   (l, first ? ParamID::lfo1Rate   : ParamID::lfo2Rate,   n + "Rate", skewed (0.02f, 20.0f, 2.0f), 1.0f, "Hz");
        addBool    (l, first ? ParamID::lfo1Sync   : ParamID::lfo2Sync,   n + "Sync", false);
        addChoice  (l, first ? ParamID::lfo1Div    : ParamID::lfo2Div,    n + "Division", Choices::syncDivs, 4);
        addPercent (l, first ? ParamID::lfo1Amount : ParamID::lfo2Amount, n + "Amount", 0.0f, true);
        addChoice  (l, first ? ParamID::lfo1Dest   : ParamID::lfo2Dest,   n + "Destination", Choices::lfoDests, 0);
    }
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    Layout l;

    addOsc (l, true);
    addOsc (l, false);

    addPercent (l, ParamID::subLevel,   "Sub Level", 0.0f);
    addChoice  (l, ParamID::subWave,    "Sub Wave", Choices::subWaves, 0);
    addPercent (l, ParamID::noiseLevel, "Noise Level", 0.0f);

    addChoice  (l, ParamID::voiceMode,  "Voice Mode", Choices::voiceModes, 0);
    addFloat   (l, ParamID::glide,      "Glide", skewed (0.0f, 2.0f, 0.25f), 0.0f, "s");
    addInt     (l, ParamID::bendRange,  "Bend Range", 1, 24, 2);
    addPercent (l, ParamID::velToAmp,   "Velocity > Amp", 0.5f);
    addPercent (l, ParamID::velToFilter,"Velocity > Filter", 0.0f);

    addChoice  (l, ParamID::filterType,   "Filter Type", Choices::filterTypes, 0);
    addFloat   (l, ParamID::cutoff,       "Cutoff", skewed (20.0f, 20000.0f, 1000.0f), 8000.0f, "Hz", 0);
    addPercent (l, ParamID::resonance,    "Resonance", 0.15f);
    addPercent (l, ParamID::drive,        "Drive", 0.0f);
    addPercent (l, ParamID::filterEnvAmt, "Filter Env Amount", 0.0f, true);
    addPercent (l, ParamID::keyTrack,     "Key Track", 0.3f);

    const auto envTime = skewed (0.0005f, 10.0f, 0.5f);
    addFloat   (l, ParamID::fAttack,  "Filter Attack",  envTime, 0.005f, "s");
    addFloat   (l, ParamID::fDecay,   "Filter Decay",   envTime, 0.4f, "s");
    addPercent (l, ParamID::fSustain, "Filter Sustain", 0.0f);
    addFloat   (l, ParamID::fRelease, "Filter Release", envTime, 0.3f, "s");
    addFloat   (l, ParamID::aAttack,  "Amp Attack",  envTime, 0.003f, "s");
    addFloat   (l, ParamID::aDecay,   "Amp Decay",   envTime, 0.5f, "s");
    addPercent (l, ParamID::aSustain, "Amp Sustain", 0.8f);
    addFloat   (l, ParamID::aRelease, "Amp Release", envTime, 0.25f, "s");

    addLfo (l, true);
    addLfo (l, false);

    addPercent (l, ParamID::chorusMix,     "Chorus Mix", 0.0f);
    addFloat   (l, ParamID::chorusRate,    "Chorus Rate", skewed (0.05f, 5.0f, 0.8f), 0.6f, "Hz");
    addPercent (l, ParamID::chorusDepth,   "Chorus Depth", 0.4f);
    addPercent (l, ParamID::delayMix,      "Delay Mix", 0.0f);
    addChoice  (l, ParamID::delayDiv,      "Delay Time", Choices::delayDivs, 5);
    addPercent (l, ParamID::delayFeedback, "Delay Feedback", 0.35f);
    addBool    (l, ParamID::delayPingPong, "Delay Ping-Pong", true);
    addPercent (l, ParamID::delayTone,     "Delay Tone", 0.55f);
    addPercent (l, ParamID::reverbMix,     "Reverb Mix", 0.0f);
    addPercent (l, ParamID::reverbSize,    "Reverb Size", 0.6f);
    addPercent (l, ParamID::reverbDamp,    "Reverb Damping", 0.5f);
    addPercent (l, ParamID::reverbWidth,   "Reverb Width", 1.0f);

    addFloat   (l, ParamID::masterGain, "Master", { -36.0f, 6.0f }, -6.0f, "dB", 1);

    return l;
}

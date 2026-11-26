#include "FactoryPresets.h"
#include "../Parameters.h"

// Sound-design notes
// ------------------
// The bank targets dark, melodic trap / R&B production: long gliding 808s and reeses that sit
// under the kick, glassy bell and keys plucks that carry the melody, airy whistle/flute leads,
// and slow, wide, chorused pads and textures for the background. Rules used throughout:
//   * Basses keep Osc width low and use mono/legato voice modes so the low end stays centred.
//   * Plucks are zero-sustain envelopes with the filter envelope doing the "pluck"; the delay
//     (mostly dotted 1/8) and a short reverb make them sound finished on their own.
//   * Pads use wide unison + chorus + big reverb and slow LFOs on the filter so they move.
// Choice indices: waves  0 Sine 1 Tri 2 Saw 3 Square 4 Pulse25 | filter 0 LP24 1 LP12 2 BP 3 HP
//                 voice  0 Poly 1 Mono 2 Legato | LFO dest 0 Off 1 Cut 2 Pitch 3 Amp 4 Pan 5 Det 6 Res
//                 LFO div 4=1/4 6=1/8 8=1/16 | delay div 1=1/16 4=1/8 5=1/8D 7=1/4 8=1/4D

using namespace ParamID;

namespace
{
    std::vector<FactoryPreset> buildPresets()
    {
        return {
            //=============================== BASS ===============================
            { "Slide 808", "Bass", {
                { oscAWave, 0 }, { oscALevel, 1.0f },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBOctave, 1 }, { oscBLevel, 0.12f },
                { drive, 0.38f }, { cutoff, 2600 }, { resonance, 0.0f }, { keyTrack, 0.0f },
                { aAttack, 0.001f }, { aDecay, 3.2f }, { aSustain, 0.0f }, { aRelease, 0.12f },
                { voiceMode, 2 }, { glide, 0.09f }, { velToAmp, 0.2f }, { bendRange, 12 },
                { masterGain, 0.0f } } },

            { "Distorted 808 Glide", "Bass", {
                { oscAWave, 0 }, { oscALevel, 1.0f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBLevel, 0.08f },
                { drive, 0.72f }, { cutoff, 1700 }, { resonance, 0.1f }, { keyTrack, 0.2f },
                { aAttack, 0.001f }, { aDecay, 2.2f }, { aSustain, 0.0f }, { aRelease, 0.1f },
                { voiceMode, 1 }, { glide, 0.12f }, { velToAmp, 0.15f }, { bendRange, 12 },
                { masterGain, -2.0f } } },

            { "Night Reese", "Bass", {
                { oscAWave, 2 }, { oscAUnison, 5 }, { oscADetune, 0.35f }, { oscAWidth, 0.25f },
                { oscBOn, 1 }, { oscBWave, 2 }, { oscBUnison, 3 }, { oscBDetune, 0.25f }, { oscBFine, 7 },
                { oscBLevel, 0.6f }, { oscBWidth, 0.2f },
                { subLevel, 0.6f },
                { filterType, 0 }, { cutoff, 420 }, { resonance, 0.25f }, { drive, 0.3f }, { keyTrack, 0.4f },
                { aAttack, 0.006f }, { aSustain, 1.0f }, { aRelease, 0.2f },
                { lfo1Shape, 1 }, { lfo1Rate, 0.25f }, { lfo1Amount, 0.22f }, { lfo1Dest, 1 },
                { lfo2Shape, 0 }, { lfo2Rate, 0.15f }, { lfo2Amount, 0.18f }, { lfo2Dest, 5 },
                { voiceMode, 1 }, { glide, 0.05f }, { velToAmp, 0.1f },
                { masterGain, -2.0f } } },

            { "Velvet Sub", "Bass", {
                { oscAWave, 1 }, { oscALevel, 0.7f },
                { subLevel, 0.8f }, { subWave, 0 },
                { filterType, 1 }, { cutoff, 900 }, { resonance, 0.05f }, { keyTrack, 0.5f },
                { aAttack, 0.003f }, { aSustain, 1.0f }, { aRelease, 0.12f },
                { voiceMode, 2 }, { glide, 0.06f }, { velToAmp, 0.25f },
                { masterGain, -4.0f } } },

            { "Groove Pluck Bass", "Bass", {
                { oscAWave, 2 },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, -1 }, { oscBLevel, 0.5f },
                { filterType, 0 }, { cutoff, 180 }, { resonance, 0.35f }, { filterEnvAmt, 0.55f }, { drive, 0.25f },
                { fAttack, 0.001f }, { fDecay, 0.22f }, { fSustain, 0.05f }, { fRelease, 0.15f },
                { aAttack, 0.001f }, { aDecay, 0.4f }, { aSustain, 0.5f }, { aRelease, 0.1f },
                { voiceMode, 1 }, { velToFilter, 0.4f },
                { masterGain, -1.0f } } },

            { "Wub Wobble", "Bass", {
                { oscAWave, 2 }, { oscAUnison, 3 }, { oscADetune, 0.2f }, { oscAWidth, 0.3f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, -1 }, { oscBLevel, 0.5f },
                { filterType, 0 }, { cutoff, 260 }, { resonance, 0.5f }, { drive, 0.4f },
                { aSustain, 1.0f }, { aRelease, 0.12f },
                { lfo1Shape, 2 }, { lfo1Sync, 1 }, { lfo1Div, 6 }, { lfo1Amount, 0.55f }, { lfo1Dest, 1 },
                { voiceMode, 1 },
                { masterGain, -2.0f } } },

            { "Rubber Bass", "Bass", {
                { oscAWave, 4 }, { oscALevel, 0.7f },
                { subLevel, 0.6f },
                { filterType, 0 }, { cutoff, 300 }, { resonance, 0.45f }, { filterEnvAmt, 0.5f },
                { fDecay, 0.15f }, { fSustain, 0.1f },
                { aSustain, 0.9f }, { aRelease, 0.1f },
                { voiceMode, 2 }, { glide, 0.07f },
                { masterGain, -3.0f } } },

            //=============================== PLUCK ==============================
            { "Glass Bell Pluck", "Pluck", {
                { oscAWave, 0 }, { oscALevel, 0.8f },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBOctave, 2 }, { oscBFine, 5 }, { oscBLevel, 0.35f },
                { filterType, 1 }, { cutoff, 3000 }, { filterEnvAmt, 0.3f }, { fDecay, 0.3f },
                { aAttack, 0.001f }, { aDecay, 0.9f }, { aSustain, 0.0f }, { aRelease, 0.6f },
                { delayMix, 0.2f }, { delayDiv, 5 }, { delayFeedback, 0.3f },
                { reverbMix, 0.3f }, { reverbSize, 0.7f },
                { masterGain, -5.0f } } },

            { "Midnight Pluck", "Pluck", {
                { oscAWave, 2 }, { oscAUnison, 3 }, { oscADetune, 0.15f }, { oscAWidth, 0.7f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, 1 }, { oscBLevel, 0.3f },
                { filterType, 0 }, { cutoff, 350 }, { resonance, 0.3f }, { filterEnvAmt, 0.6f },
                { fAttack, 0.001f }, { fDecay, 0.25f }, { fSustain, 0.0f },
                { aAttack, 0.001f }, { aDecay, 0.7f }, { aSustain, 0.0f }, { aRelease, 0.4f },
                { velToFilter, 0.3f },
                { chorusMix, 0.3f },
                { delayMix, 0.25f }, { delayDiv, 5 }, { delayFeedback, 0.35f }, { delayTone, 0.4f },
                { reverbMix, 0.25f },
                { masterGain, -6.0f } } },

            { "Crystal Keys", "Pluck", {
                { oscAWave, 1 },
                { oscBOn, 1 }, { oscBWave, 0 }, { oscBOctave, 1 }, { oscBLevel, 0.4f },
                { filterType, 1 }, { cutoff, 5000 }, { resonance, 0.05f },
                { aAttack, 0.002f }, { aDecay, 1.2f }, { aSustain, 0.15f }, { aRelease, 0.5f },
                { chorusMix, 0.4f }, { chorusRate, 0.5f },
                { reverbMix, 0.3f },
                { masterGain, -5.0f } } },

            { "Dark Mallet", "Pluck", {
                { oscAWave, 0 },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBSemi, 12 }, { oscBLevel, 0.4f },
                { noiseLevel, 0.05f },
                { filterType, 0 }, { cutoff, 1200 }, { filterEnvAmt, 0.35f }, { fDecay, 0.12f },
                { aAttack, 0.001f }, { aDecay, 0.35f }, { aSustain, 0.0f }, { aRelease, 0.3f },
                { reverbMix, 0.35f }, { reverbDamp, 0.7f },
                { masterGain, -4.0f } } },

            { "Lo-Fi Harp", "Pluck", {
                { oscAWave, 2 },
                { noiseLevel, 0.03f },
                { filterType, 1 }, { cutoff, 800 }, { resonance, 0.2f }, { filterEnvAmt, 0.5f }, { fDecay, 0.4f },
                { aAttack, 0.001f }, { aDecay, 1.0f }, { aSustain, 0.0f }, { aRelease, 0.5f },
                { lfo1Shape, 0 }, { lfo1Rate, 5.5f }, { lfo1Amount, 0.01f }, { lfo1Dest, 2 },
                { chorusMix, 0.5f }, { chorusRate, 0.3f },
                { reverbMix, 0.3f },
                { masterGain, -5.0f } } },

            { "Arp Spark", "Pluck", {
                { oscAWave, 2 }, { oscAUnison, 5 }, { oscADetune, 0.25f }, { oscAWidth, 0.8f },
                { filterType, 0 }, { cutoff, 600 }, { filterEnvAmt, 0.55f }, { fDecay, 0.18f },
                { aAttack, 0.001f }, { aDecay, 0.3f }, { aSustain, 0.0f }, { aRelease, 0.25f },
                { delayMix, 0.3f }, { delayDiv, 5 }, { delayFeedback, 0.4f },
                { reverbMix, 0.2f },
                { masterGain, -7.0f } } },

            { "Kalimba Drip", "Pluck", {
                { oscAWave, 0 },
                { oscBOn, 1 }, { oscBWave, 4 }, { oscBOctave, 1 }, { oscBLevel, 0.2f },
                { filterType, 1 }, { cutoff, 2500 }, { filterEnvAmt, 0.4f }, { fDecay, 0.08f },
                { aAttack, 0.001f }, { aDecay, 0.5f }, { aSustain, 0.0f }, { aRelease, 0.3f },
                { delayMix, 0.2f }, { delayDiv, 7 }, { delayFeedback, 0.3f },
                { reverbMix, 0.25f },
                { masterGain, -4.0f } } },

            //=============================== LEAD ===============================
            { "Silk Lead", "Lead", {
                { oscAWave, 2 }, { oscAUnison, 3 }, { oscADetune, 0.12f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, 1 }, { oscBLevel, 0.25f },
                { filterType, 0 }, { cutoff, 3500 }, { resonance, 0.2f },
                { aAttack, 0.01f }, { aSustain, 0.9f }, { aRelease, 0.3f },
                { lfo1Shape, 0 }, { lfo1Rate, 5.5f }, { lfo1Amount, 0.015f }, { lfo1Dest, 2 },
                { voiceMode, 2 }, { glide, 0.08f },
                { delayMix, 0.2f }, { delayDiv, 5 },
                { reverbMix, 0.2f },
                { masterGain, -7.0f } } },

            { "Whistle Lead", "Lead", {
                { oscAWave, 0 }, { oscAOctave, 1 },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBOctave, 1 }, { oscBLevel, 0.3f },
                { noiseLevel, 0.04f },
                { filterType, 1 }, { cutoff, 6000 },
                { aAttack, 0.04f }, { aSustain, 1.0f }, { aRelease, 0.35f },
                { lfo1Shape, 0 }, { lfo1Rate, 5.0f }, { lfo1Amount, 0.012f }, { lfo1Dest, 2 },
                { voiceMode, 1 }, { glide, 0.12f },
                { delayMix, 0.25f }, { delayDiv, 7 },
                { reverbMix, 0.35f },
                { masterGain, -9.0f } } },

            { "Vapor Square", "Lead", {
                { oscAWave, 3 }, { oscAUnison, 3 }, { oscADetune, 0.1f },
                { oscBOn, 1 }, { oscBWave, 4 }, { oscBOctave, -1 }, { oscBLevel, 0.4f },
                { filterType, 0 }, { cutoff, 2200 }, { resonance, 0.3f }, { filterEnvAmt, 0.25f },
                { fDecay, 0.5f }, { fSustain, 0.4f },
                { aSustain, 0.9f }, { aRelease, 0.3f },
                { voiceMode, 2 }, { glide, 0.05f },
                { chorusMix, 0.4f },
                { delayMix, 0.25f }, { delayDiv, 5 },
                { masterGain, -7.0f } } },

            { "Siren Fifths", "Lead", {
                { oscAWave, 2 },
                { oscBOn, 1 }, { oscBWave, 2 }, { oscBSemi, 7 }, { oscBLevel, 0.5f },
                { filterType, 0 }, { cutoff, 2800 }, { drive, 0.3f },
                { aSustain, 1.0f }, { aRelease, 0.25f },
                { lfo1Shape, 0 }, { lfo1Rate, 4.5f }, { lfo1Amount, 0.02f }, { lfo1Dest, 2 },
                { voiceMode, 1 }, { glide, 0.2f },
                { reverbMix, 0.25f },
                { masterGain, -8.0f } } },

            { "Flute Air", "Lead", {
                { oscAWave, 1 },
                { noiseLevel, 0.08f },
                { filterType, 1 }, { cutoff, 3500 },
                { aAttack, 0.08f }, { aSustain, 0.95f }, { aRelease, 0.4f },
                { lfo1Shape, 0 }, { lfo1Rate, 4.8f }, { lfo1Amount, 0.01f }, { lfo1Dest, 2 },
                { lfo2Shape, 0 }, { lfo2Rate, 4.5f }, { lfo2Amount, 0.15f }, { lfo2Dest, 3 },
                { voiceMode, 2 }, { glide, 0.06f },
                { reverbMix, 0.35f },
                { masterGain, -4.0f } } },

            { "Hyper Saw Lead", "Lead", {
                { oscAWave, 2 }, { oscAUnison, 7 }, { oscADetune, 0.3f }, { oscAWidth, 0.9f },
                { oscBOn, 1 }, { oscBWave, 2 }, { oscBOctave, 1 }, { oscBUnison, 5 }, { oscBLevel, 0.35f },
                { filterType, 0 }, { cutoff, 7000 },
                { aSustain, 0.9f }, { aRelease, 0.3f },
                { chorusMix, 0.2f },
                { delayMix, 0.2f }, { delayDiv, 5 },
                { reverbMix, 0.25f },
                { masterGain, -9.0f } } },

            //================================ PAD ===============================
            { "Dream Pad", "Pad", {
                { oscAWave, 2 }, { oscAUnison, 7 }, { oscADetune, 0.3f }, { oscAWidth, 1.0f },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBOctave, 1 }, { oscBLevel, 0.3f },
                { filterType, 0 }, { cutoff, 1400 }, { resonance, 0.15f }, { filterEnvAmt, 0.2f },
                { fAttack, 1.5f }, { fDecay, 3.0f }, { fSustain, 0.6f }, { fRelease, 2.5f },
                { aAttack, 1.2f }, { aDecay, 2.0f }, { aSustain, 0.9f }, { aRelease, 2.5f },
                { lfo1Shape, 1 }, { lfo1Rate, 0.12f }, { lfo1Amount, 0.15f }, { lfo1Dest, 1 },
                { chorusMix, 0.45f },
                { reverbMix, 0.55f }, { reverbSize, 0.85f },
                { masterGain, -9.0f } } },

            { "Warm Analog Strings", "Pad", {
                { oscAWave, 2 }, { oscAUnison, 5 }, { oscADetune, 0.22f }, { oscAWidth, 0.8f },
                { oscBOn, 1 }, { oscBWave, 2 }, { oscBFine, 8 }, { oscBUnison, 3 }, { oscBLevel, 0.6f },
                { filterType, 1 }, { cutoff, 2200 }, { resonance, 0.1f },
                { aAttack, 0.6f }, { aSustain, 1.0f }, { aRelease, 1.5f },
                { chorusMix, 0.6f }, { chorusRate, 0.4f }, { chorusDepth, 0.5f },
                { reverbMix, 0.4f },
                { masterGain, -9.0f } } },

            { "Midnight Choir", "Pad", {
                { oscAWave, 1 }, { oscAUnison, 5 }, { oscADetune, 0.2f }, { oscAWidth, 0.9f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, 1 }, { oscBUnison, 3 }, { oscBLevel, 0.25f },
                { filterType, 2 }, { cutoff, 1100 }, { resonance, 0.35f },
                { aAttack, 1.5f }, { aSustain, 1.0f }, { aRelease, 3.0f },
                { lfo1Shape, 0 }, { lfo1Rate, 0.2f }, { lfo1Amount, 0.2f }, { lfo1Dest, 1 },
                { chorusMix, 0.5f },
                { reverbMix, 0.6f },
                { masterGain, -2.0f } } },

            { "Glass Air Pad", "Pad", {
                { oscAWave, 4 }, { oscAUnison, 5 }, { oscADetune, 0.18f }, { oscAWidth, 1.0f },
                { oscBOn, 1 }, { oscBWave, 0 }, { oscBOctave, 2 }, { oscBLevel, 0.2f },
                { noiseLevel, 0.06f },
                { filterType, 3 }, { cutoff, 400 }, { resonance, 0.1f },
                { aAttack, 2.0f }, { aSustain, 1.0f }, { aRelease, 3.0f },
                { delayMix, 0.2f }, { delayDiv, 8 },
                { reverbMix, 0.7f }, { reverbSize, 0.9f }, { reverbDamp, 0.3f },
                { masterGain, -9.0f } } },

            { "Underwater Pad", "Pad", {
                { oscAWave, 2 }, { oscAUnison, 7 }, { oscADetune, 0.25f }, { oscAWidth, 1.0f },
                { subLevel, 0.3f },
                { filterType, 0 }, { cutoff, 500 }, { resonance, 0.4f },
                { aAttack, 1.0f }, { aSustain, 1.0f }, { aRelease, 2.5f },
                { lfo1Shape, 0 }, { lfo1Rate, 0.08f }, { lfo1Amount, 0.3f }, { lfo1Dest, 1 },
                { lfo2Shape, 1 }, { lfo2Rate, 0.3f }, { lfo2Amount, 0.4f }, { lfo2Dest, 4 },
                { chorusMix, 0.3f },
                { reverbMix, 0.5f }, { reverbDamp, 0.8f },
                { masterGain, -8.0f } } },

            { "Sunset Keys Pad", "Pad", {
                { oscAWave, 3 }, { oscAUnison, 3 }, { oscADetune, 0.12f },
                { oscBOn, 1 }, { oscBWave, 1 }, { oscBOctave, 1 }, { oscBLevel, 0.4f },
                { filterType, 0 }, { cutoff, 1800 }, { filterEnvAmt, 0.3f },
                { fAttack, 0.02f }, { fDecay, 1.5f }, { fSustain, 0.4f },
                { aAttack, 0.08f }, { aDecay, 2.0f }, { aSustain, 0.6f }, { aRelease, 1.2f },
                { chorusMix, 0.5f },
                { reverbMix, 0.4f },
                { masterGain, -8.0f } } },

            //============================== TEXTURE =============================
            { "Vinyl Haze", "Texture", {
                { oscAWave, 0 }, { oscAUnison, 3 }, { oscADetune, 0.1f }, { oscALevel, 0.5f },
                { noiseLevel, 0.3f },
                { filterType, 2 }, { cutoff, 2500 }, { resonance, 0.5f },
                { aAttack, 0.5f }, { aSustain, 1.0f }, { aRelease, 2.0f },
                { lfo1Shape, 4 }, { lfo1Rate, 6.0f }, { lfo1Amount, 0.35f }, { lfo1Dest, 1 },
                { reverbMix, 0.6f },
                { masterGain, 0.0f } } },

            { "Gated Saw Texture", "Texture", {
                { oscAWave, 2 }, { oscAUnison, 7 }, { oscADetune, 0.3f }, { oscAWidth, 1.0f },
                { filterType, 0 }, { cutoff, 2500 },
                { aSustain, 1.0f }, { aRelease, 0.5f },
                { lfo1Shape, 3 }, { lfo1Sync, 1 }, { lfo1Div, 8 }, { lfo1Amount, 1.0f }, { lfo1Dest, 3 },
                { delayMix, 0.25f }, { delayDiv, 5 },
                { reverbMix, 0.4f },
                { masterGain, -9.0f } } },

            { "Night Rain", "Texture", {
                { oscAOn, 0 },
                { noiseLevel, 0.5f },
                { filterType, 2 }, { cutoff, 4000 }, { resonance, 0.6f },
                { aAttack, 0.3f }, { aSustain, 1.0f }, { aRelease, 1.5f },
                { lfo1Shape, 4 }, { lfo1Sync, 1 }, { lfo1Div, 8 }, { lfo1Amount, 0.4f }, { lfo1Dest, 1 },
                { delayMix, 0.35f }, { delayDiv, 5 }, { delayFeedback, 0.5f },
                { reverbMix, 0.6f },
                { masterGain, 0.0f } } },

            { "Drift Atmosphere", "Texture", {
                { oscAWave, 1 }, { oscAUnison, 7 }, { oscADetune, 0.5f }, { oscAWidth, 1.0f },
                { filterType, 1 }, { cutoff, 3000 },
                { aAttack, 3.0f }, { aSustain, 1.0f }, { aRelease, 5.0f },
                { lfo2Shape, 0 }, { lfo2Rate, 0.05f }, { lfo2Amount, 0.3f }, { lfo2Dest, 5 },
                { chorusMix, 0.4f },
                { reverbMix, 0.8f }, { reverbSize, 0.95f },
                { masterGain, -7.0f } } },

            { "Pump Chords", "Texture", {
                { oscAWave, 2 }, { oscAUnison, 5 }, { oscADetune, 0.2f }, { oscAWidth, 0.8f },
                { oscBOn, 1 }, { oscBWave, 3 }, { oscBOctave, -1 }, { oscBLevel, 0.4f },
                { filterType, 0 }, { cutoff, 3000 },
                { aSustain, 1.0f }, { aRelease, 0.4f },
                { lfo1Shape, 2 }, { lfo1Sync, 1 }, { lfo1Div, 4 }, { lfo1Amount, -0.9f }, { lfo1Dest, 3 },
                { chorusMix, 0.3f },
                { reverbMix, 0.3f },
                { masterGain, -9.0f } } },
        };
    }
}

const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = buildPresets();
    return presets;
}

const juce::StringArray& getPresetCategories()
{
    static const juce::StringArray cats { "Bass", "Pluck", "Lead", "Pad", "Texture" };
    return cats;
}

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
#include "DspUtils.h"

// Plain snapshot of every parameter the voices need. The processor fills this once per block
// from the APVTS atomics, so the per-sample code never touches parameter objects.
struct EngineParams
{
    struct Osc
    {
        bool  on = true;
        int   wave = 2;
        int   octave = 0, semi = 0;
        float fine = 0.0f;      // cents
        float level = 0.8f;
        int   unison = 1;
        float detune = 0.2f;    // 0..1
        float width = 0.5f;     // 0..1
    };

    struct LfoP
    {
        int   shape = 0;
        float rateHz = 1.0f;    // already converted from tempo sync if needed
        float amount = 0.0f;    // -1..1
        int   dest = 0;
        bool  synced = false;
        float syncBeats = 1.0f;
    };

    std::array<Osc, 2> osc;
    float subLevel = 0.0f;  int subWave = 0;
    float noiseLevel = 0.0f;

    int   voiceMode = 0;    // 0 poly, 1 mono, 2 legato
    float glide = 0.0f;     // seconds
    int   bendRange = 2;
    float velToAmp = 0.5f, velToFilter = 0.0f;

    int   filterType = 0;
    float cutoff = 8000.0f, resonance = 0.15f, drive = 0.0f, filterEnvAmt = 0.0f, keyTrack = 0.3f;

    float fA = 0.005f, fD = 0.4f, fS = 0.0f, fR = 0.3f;
    float aA = 0.003f, aD = 0.5f, aS = 0.8f, aR = 0.25f;

    std::array<LfoP, 2> lfo;
};

class SynthEngine
{
public:
    static constexpr int maxVoices  = 16;
    static constexpr int maxUnison  = 7;
    static constexpr int controlRate = 16;   // pitch/unison recomputed every N samples

    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void setParams (const EngineParams& p) { params = p; }

    // Host transport for tempo-synced LFO phase. ppq < 0 means "not playing".
    void setTransport (double ppqPosition, bool isPlaying) { hostPpq = ppqPosition; hostPlaying = isPlaying; }

    // Renders into (and adds nothing to) the first two channels of buffer.
    void process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);

    int getNumActiveVoices() const;

private:
    struct Voice
    {
        int   note = -1;
        bool  gate = false;
        bool  sustained = false;   // key released while sustain pedal held
        float velocity = 1.0f;
        uint64_t age = 0;

        float pitch = 60.0f, targetPitch = 60.0f;

        float phase[2][maxUnison] {};
        float inc[2][maxUnison] {};
        float gainL[2][maxUnison] {}, gainR[2][maxUnison] {};
        float subPhase = 0.0f, subInc = 0.0f;

        mdsp::Envelope ampEnv, filtEnv;
        mdsp::SVF svf[2][2];        // [stage][channel]

        bool isActive() const noexcept { return ampEnv.isActive(); }
    };

    void handleMidi (const juce::MidiMessage& m);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void allNotesOff (bool hard);
    void startVoice (Voice& v, int note, float velocity, bool retrigger, bool glideFromLast);
    Voice* findVoiceFor (int note);

    void renderLfos (int numSamples);
    void updateVoiceControl (Voice& v, float lfoPitch, float lfoDetune);
    void renderVoice (Voice& v, float* L, float* R, int start, int num);

    EngineParams params;
    double sampleRate = 44100.0;
    float  srF = 44100.0f;

    std::array<Voice, maxVoices> voices;
    std::vector<int> monoStack;           // held notes, last = most recent
    uint64_t ageCounter = 0;
    float lastPitch = -1.0f;              // for glide in poly mode / mono first note

    float pitchBend = 0.0f;               // -1..1
    float modWheel = 0.0f;                // 0..1
    bool  sustainPedal = false;

    mdsp::Lfo lfo[2];
    std::vector<float> lfoBuf[2];         // [0]: per-destination mod buffers, [1]: mono-host scratch
    int curBlockSize = 0;
    double hostPpq = -1.0;
    bool hostPlaying = false;

    mdsp::Random noise;
};

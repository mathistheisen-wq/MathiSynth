#pragma once

#include <juce_dsp/juce_dsp.h>

struct EffectParams
{
    float chorusMix = 0.0f, chorusRate = 0.6f, chorusDepth = 0.4f;
    float delayMix = 0.0f, delaySeconds = 0.375f, delayFeedback = 0.35f, delayTone = 0.55f;
    bool  delayPingPong = true;
    float reverbMix = 0.0f, reverbSize = 0.6f, reverbDamp = 0.5f, reverbWidth = 1.0f;
    float masterGain = 0.5f;   // linear
};

// Serial FX chain: Chorus -> Delay -> Reverb -> master gain -> soft clip.
// Each stage is skipped entirely when its mix is zero, so dry presets cost nothing.
class EffectsChain
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, const EffectParams& p);

private:
    void processDelay (float* L, float* R, int n, const EffectParams& p);

    double sampleRate = 44100.0;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;

    std::vector<float> delayBuf[2];
    int writePos = 0;
    juce::SmoothedValue<float> delayTimeSmoothed;   // in samples; smooth so tempo changes don't click
    float toneState[2] {}, hpState[2] {};

    juce::SmoothedValue<float> gainSmoothed;
};

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "dsp/SynthEngine.h"
#include "dsp/Effects.h"
#include "presets/PresetManager.h"

class MathiSynthProcessor : public juce::AudioProcessor
{
public:
    MathiSynthProcessor();
    ~MathiSynthProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MathiSynth"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    // Factory + user presets are exposed as host programs, so FL Studio's and Ableton's
    // own preset arrows can step through them too.
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

    // Lets the UI show a live voice counter without locking.
    std::atomic<int> activeVoices { 0 };

private:
    struct OscPtrs { std::atomic<float> *on, *wave, *oct, *semi, *fine, *level, *unison, *detune, *width; };
    struct LfoPtrs { std::atomic<float> *shape, *rate, *sync, *div, *amount, *dest; };

    std::atomic<float>* raw (const char* id) { return apvts.getRawParameterValue (id); }
    void cacheParameterPointers();
    void readParameters (double bpm);

    OscPtrs oscP[2] {};
    LfoPtrs lfoP[2] {};
    std::atomic<float> *subLevel{}, *subWave{}, *noiseLevel{}, *voiceMode{}, *glide{}, *bendRange{},
                       *velToAmp{}, *velToFilter{}, *filterType{}, *cutoff{}, *resonance{}, *drive{},
                       *filterEnvAmt{}, *keyTrack{}, *fA{}, *fD{}, *fS{}, *fR{}, *aA{}, *aD{}, *aS{}, *aR{},
                       *chorusMix{}, *chorusRate{}, *chorusDepth{}, *delayMix{}, *delayDiv{}, *delayFeedback{},
                       *delayPingPong{}, *delayTone{}, *reverbMix{}, *reverbSize{}, *reverbDamp{}, *reverbWidth{},
                       *masterGain{};

    SynthEngine engine;
    EffectsChain effects;
    EngineParams engineParams;
    EffectParams fxParams;
    juce::SmoothedValue<float> cutoffSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MathiSynthProcessor)
};

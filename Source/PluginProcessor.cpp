#include "PluginProcessor.h"
#include "PluginEditor.h"

MathiSynthProcessor::MathiSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MathiSynth", createParameterLayout()),
      presets (apvts)
{
    cacheParameterPointers();
    presets.load (0);   // start on the first Bass preset rather than a bare init patch
}

void MathiSynthProcessor::cacheParameterPointers()
{
    using namespace ParamID;
    oscP[0] = { raw (oscAOn), raw (oscAWave), raw (oscAOctave), raw (oscASemi), raw (oscAFine),
                raw (oscALevel), raw (oscAUnison), raw (oscADetune), raw (oscAWidth) };
    oscP[1] = { raw (oscBOn), raw (oscBWave), raw (oscBOctave), raw (oscBSemi), raw (oscBFine),
                raw (oscBLevel), raw (oscBUnison), raw (oscBDetune), raw (oscBWidth) };
    lfoP[0] = { raw (lfo1Shape), raw (lfo1Rate), raw (lfo1Sync), raw (lfo1Div), raw (lfo1Amount), raw (lfo1Dest) };
    lfoP[1] = { raw (lfo2Shape), raw (lfo2Rate), raw (lfo2Sync), raw (lfo2Div), raw (lfo2Amount), raw (lfo2Dest) };

    subLevel = raw (ParamID::subLevel);   subWave = raw (ParamID::subWave);   noiseLevel = raw (ParamID::noiseLevel);
    voiceMode = raw (ParamID::voiceMode); glide = raw (ParamID::glide);       bendRange = raw (ParamID::bendRange);
    velToAmp = raw (ParamID::velToAmp);   velToFilter = raw (ParamID::velToFilter);
    filterType = raw (ParamID::filterType); cutoff = raw (ParamID::cutoff);   resonance = raw (ParamID::resonance);
    drive = raw (ParamID::drive);         filterEnvAmt = raw (ParamID::filterEnvAmt); keyTrack = raw (ParamID::keyTrack);
    fA = raw (fAttack); fD = raw (fDecay); fS = raw (fSustain); fR = raw (fRelease);
    aA = raw (aAttack); aD = raw (aDecay); aS = raw (aSustain); aR = raw (aRelease);
    chorusMix = raw (ParamID::chorusMix); chorusRate = raw (ParamID::chorusRate); chorusDepth = raw (ParamID::chorusDepth);
    delayMix = raw (ParamID::delayMix);   delayDiv = raw (ParamID::delayDiv);     delayFeedback = raw (ParamID::delayFeedback);
    delayPingPong = raw (ParamID::delayPingPong); delayTone = raw (ParamID::delayTone);
    reverbMix = raw (ParamID::reverbMix); reverbSize = raw (ParamID::reverbSize); reverbDamp = raw (ParamID::reverbDamp);
    reverbWidth = raw (ParamID::reverbWidth);
    masterGain = raw (ParamID::masterGain);
}

void MathiSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    effects.prepare (sampleRate, samplesPerBlock);
    cutoffSmoothed.reset (sampleRate / (double) juce::jmax (1, samplesPerBlock), 0.03);  // per-block smoothing
    cutoffSmoothed.setCurrentAndTargetValue (cutoff->load());
}

bool MathiSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void MathiSynthProcessor::readParameters (double bpm)
{
    auto& e = engineParams;
    for (int o = 0; o < 2; ++o)
    {
        auto& d = e.osc[(size_t) o];
        const auto& s = oscP[o];
        d.on = s.on->load() > 0.5f;
        d.wave = (int) s.wave->load();
        d.octave = (int) s.oct->load();
        d.semi = (int) s.semi->load();
        d.fine = s.fine->load();
        d.level = s.level->load();
        d.unison = juce::jlimit (1, SynthEngine::maxUnison, (int) s.unison->load());
        d.detune = s.detune->load();
        d.width = s.width->load();
    }

    for (int l = 0; l < 2; ++l)
    {
        auto& d = e.lfo[(size_t) l];
        const auto& s = lfoP[l];
        d.shape  = (int) s.shape->load();
        d.amount = s.amount->load();
        d.dest   = (int) s.dest->load();
        d.synced = s.sync->load() > 0.5f;
        d.syncBeats = SyncBeats::lfo[juce::jlimit (0, (int) std::size (SyncBeats::lfo) - 1, (int) s.div->load())];
        d.rateHz = d.synced ? (float) (bpm / 60.0) / d.syncBeats : s.rate->load();
    }

    e.subLevel = subLevel->load();  e.subWave = (int) subWave->load();  e.noiseLevel = noiseLevel->load();
    e.voiceMode = (int) voiceMode->load(); e.glide = glide->load(); e.bendRange = (int) bendRange->load();
    e.velToAmp = velToAmp->load();  e.velToFilter = velToFilter->load();
    e.filterType = (int) filterType->load();
    cutoffSmoothed.setTargetValue (cutoff->load());
    e.cutoff = cutoffSmoothed.getNextValue();
    e.resonance = resonance->load(); e.drive = drive->load();
    e.filterEnvAmt = filterEnvAmt->load(); e.keyTrack = keyTrack->load();
    e.fA = fA->load(); e.fD = fD->load(); e.fS = fS->load(); e.fR = fR->load();
    e.aA = aA->load(); e.aD = aD->load(); e.aS = aS->load(); e.aR = aR->load();
    engine.setParams (e);

    auto& f = fxParams;
    f.chorusMix = chorusMix->load(); f.chorusRate = chorusRate->load(); f.chorusDepth = chorusDepth->load();
    f.delayMix = delayMix->load();
    const int div = juce::jlimit (0, (int) std::size (SyncBeats::delay) - 1, (int) delayDiv->load());
    f.delaySeconds = (float) (60.0 / bpm) * SyncBeats::delay[div];
    f.delayFeedback = delayFeedback->load(); f.delayPingPong = delayPingPong->load() > 0.5f;
    f.delayTone = delayTone->load();
    f.reverbMix = reverbMix->load(); f.reverbSize = reverbSize->load(); f.reverbDamp = reverbDamp->load();
    f.reverbWidth = reverbWidth->load();
    f.masterGain = juce::Decibels::decibelsToGain (masterGain->load());
}

void MathiSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    double bpm = 120.0, ppq = -1.0;
    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = juce::jlimit (20.0, 999.0, *b);
            playing = pos->getIsPlaying();
            if (auto p = pos->getPpqPosition()) ppq = *p;
        }

    readParameters (bpm);
    engine.setTransport (ppq, playing);
    engine.process (buffer, midi);
    effects.process (buffer, fxParams);

    activeVoices.store (engine.getNumActiveVoices(), std::memory_order_relaxed);
    midi.clear();
}

//==============================================================================
int MathiSynthProcessor::getNumPrograms()                     { return juce::jmax (1, (int) presets.getEntries().size()); }
int MathiSynthProcessor::getCurrentProgram()                  { return presets.getCurrentIndex(); }
void MathiSynthProcessor::setCurrentProgram (int index)       { presets.load (index); }

const juce::String MathiSynthProcessor::getProgramName (int index)
{
    const auto& e = presets.getEntries();
    if (juce::isPositiveAndBelow (index, (int) e.size()))
        return e[(size_t) index].category + " - " + e[(size_t) index].name;
    return "Init";
}

void MathiSynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", presets.getCurrentIndex(), nullptr);
    state.setProperty ("presetName", presets.getCurrentName(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MathiSynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            presets.setCurrentIndexSilently ((int) tree.getProperty ("presetIndex", 0));
            apvts.replaceState (tree);   // the editor polls the preset index, so no callback needed
        }
}

juce::AudioProcessorEditor* MathiSynthProcessor::createEditor()
{
    return new MathiSynthEditor (*this);
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MathiSynthProcessor();
}

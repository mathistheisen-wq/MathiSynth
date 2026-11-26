#include "SynthEngine.h"
#include "../Parameters.h"

using namespace mdsp;

namespace
{
    constexpr int numDests = 7;
    constexpr float voiceGain = 0.32f;   // headroom: several unison voices x polyphony
}

void SynthEngine::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    srF = (float) sr;

    for (auto& v : voices)
    {
        v.ampEnv.setSampleRate (sr);
        v.filtEnv.setSampleRate (sr);
    }

    for (auto& b : lfoBuf)
        b.assign ((size_t) (numDests * std::max (1, maxBlockSize)), 0.0f);

    noise.seed (0x1234567u);
    lfo[0].rng.seed (0xA5A5A5u);
    lfo[1].rng.seed (0x5A5A5Au);
    reset();
}

void SynthEngine::reset()
{
    for (auto& v : voices)
    {
        v.ampEnv.reset();
        v.filtEnv.reset();
        for (auto& stage : v.svf) for (auto& f : stage) f.reset();
        v.note = -1;
        v.gate = v.sustained = false;
    }
    monoStack.clear();
    monoStack.reserve (128);
    lastPitch = -1.0f;
    pitchBend = 0.0f;
    sustainPedal = false;
    lfo[0].phase = lfo[1].phase = 0.0f;
}

int SynthEngine::getNumActiveVoices() const
{
    int n = 0;
    for (auto& v : voices) n += v.isActive() ? 1 : 0;
    return n;
}

//==============================================================================
// MIDI / voice allocation

void SynthEngine::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())                 noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())           noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())        pitchBend = (float) (m.getPitchWheelValue() - 8192) / 8192.0f;
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        if (cc == 1)
            modWheel = (float) m.getControllerValue() / 127.0f;
        else if (cc == 64)
        {
            const bool down = m.getControllerValue() >= 64;
            if (sustainPedal && ! down)
                for (auto& v : voices)
                    if (v.sustained) { v.sustained = false; v.ampEnv.noteOff(); v.filtEnv.noteOff(); }
            sustainPedal = down;
        }
        else if (cc == 123 || cc == 120)
            allNotesOff (cc == 120);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        allNotesOff (m.isAllSoundOff());
}

void SynthEngine::allNotesOff (bool hard)
{
    monoStack.clear();
    for (auto& v : voices)
    {
        v.gate = v.sustained = false;
        if (hard) { v.ampEnv.reset(); v.filtEnv.reset(); v.note = -1; }
        else      { v.ampEnv.noteOff(); v.filtEnv.noteOff(); }
    }
}

SynthEngine::Voice* SynthEngine::findVoiceFor (int note)
{
    for (auto& v : voices) if (v.isActive() && v.note == note) return &v;   // retrigger same key
    for (auto& v : voices) if (! v.isActive()) return &v;

    Voice* best = nullptr;                                                  // steal oldest released voice...
    for (auto& v : voices)
        if (! v.gate && ! v.sustained && (best == nullptr || v.age < best->age)) best = &v;
    if (best != nullptr) return best;

    best = &voices[0];                                                      // ...or the oldest held one
    for (auto& v : voices) if (v.age < best->age) best = &v;
    return best;
}

void SynthEngine::startVoice (Voice& v, int note, float velocity, bool retrigger, bool glideFromLast)
{
    const bool fresh = ! v.isActive();

    if (fresh)
    {
        for (auto& stage : v.svf) for (auto& f : stage) f.reset();
        for (int o = 0; o < 2; ++o)
        {
            const int n = params.osc[(size_t) o].unison;
            for (int u = 0; u < maxUnison; ++u)
                // Single voice: start at phase 0 so basses hit with the same punch every time.
                // Unison: random phases so the stack sounds wide instead of flanging on attack.
                v.phase[o][u] = n == 1 ? 0.0f : noise.unipolar();
        }
        v.subPhase = 0.0f;
        v.pitch = (glideFromLast && params.glide > 0.0f && lastPitch >= 0.0f) ? lastPitch : (float) note;
    }
    else if (! glideFromLast || params.glide <= 0.0f)
    {
        v.pitch = (float) note;
    }

    v.note = note;
    v.targetPitch = (float) note;
    v.gate = true;
    v.sustained = false;
    v.velocity = velocity;
    v.age = ++ageCounter;

    if (retrigger)
    {
        v.ampEnv.noteOn();
        v.filtEnv.noteOn();
    }

    lastPitch = (float) note;
}

void SynthEngine::noteOn (int note, float velocity)
{
    if (params.voiceMode == 0)
    {
        startVoice (*findVoiceFor (note), note, velocity, true, true);
        return;
    }

    // Mono / Legato: one voice with a held-note stack (last-note priority).
    monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
    const bool otherKeyHeld = ! monoStack.empty() && voices[0].gate;
    monoStack.push_back (note);

    const bool legato = params.voiceMode == 2;
    // Mono: always glides and retriggers. Legato: glides + skips retrigger only for overlapping notes —
    // that's the classic 808 slide.
    const bool glide     = legato ? otherKeyHeld : true;
    const bool retrigger = legato ? ! otherKeyHeld : true;
    startVoice (voices[0], note, velocity, retrigger, glide);
}

void SynthEngine::noteOff (int note)
{
    if (params.voiceMode == 0)
    {
        for (auto& v : voices)
        {
            if (v.note != note || ! v.gate) continue;
            v.gate = false;
            if (sustainPedal) v.sustained = true;
            else { v.ampEnv.noteOff(); v.filtEnv.noteOff(); }
        }
        return;
    }

    monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
    auto& v = voices[0];
    if (v.note != note || ! v.gate) return;

    if (! monoStack.empty())
    {
        // Fall back to the previous held key, gliding to it without retriggering.
        const int back = monoStack.back();
        v.note = back;
        v.targetPitch = (float) back;
        if (params.glide <= 0.0f) v.pitch = (float) back;
        lastPitch = (float) back;
    }
    else
    {
        v.gate = false;
        if (sustainPedal) v.sustained = true;
        else { v.ampEnv.noteOff(); v.filtEnv.noteOff(); }
    }
}

//==============================================================================
// Modulation

void SynthEngine::renderLfos (int numSamples)
{
    float* mod = lfoBuf[0].data();   // layout: [dest * numSamples + i]
    std::fill (mod, mod + numDests * numSamples, 0.0f);

    for (int l = 0; l < 2; ++l)
    {
        const auto& p = params.lfo[(size_t) l];
        float amount = p.amount;
        if (l == 0 && std::abs (amount) > 1.0e-6f)   // mod wheel deepens LFO 1 (vibrato / wobble on the wheel)
            amount = juce::jlimit (-1.0f, 1.0f, amount + std::copysign (modWheel * 0.5f, amount));

        if (p.synced && hostPlaying && hostPpq >= 0.0)
        {
            const float newPhase = (float) std::fmod (hostPpq / (double) p.syncBeats, 1.0);
            if (newPhase < lfo[l].phase - 0.5f) lfo[l].held = lfo[l].rng.bipolar();
            lfo[l].phase = newPhase;   // lock to the bar grid while the DAW plays
        }

        const float inc = p.rateHz / srF;

        if (p.dest == (int) LfoDest::Off || std::abs (amount) <= 1.0e-6f)
        {
            for (int i = 0; i < numSamples; ++i) lfo[l].next (p.shape, inc);   // keep phase running
            continue;
        }

        float* d = mod + p.dest * numSamples;
        const bool isAmp = p.dest == (int) LfoDest::Amp;
        for (int i = 0; i < numSamples; ++i)
        {
            const float x = lfo[l].next (p.shape, inc);
            // Amp mod is unipolar dipping (tremolo / sidechain-pump feel), everything else bipolar.
            // Negative amount flips the shape: Saw + negative Amp = sidechain-style pump.
            d[i] += isAmp ? std::abs (amount) * (0.5f * (amount < 0.0f ? -x : x) - 0.5f)
                          : amount * x;
        }
    }
}

void SynthEngine::updateVoiceControl (Voice& v, float lfoPitch, float lfoDetune)
{
    // Constant-time portamento: reach ~99% of the target in `glide` seconds.
    if (params.glide > 0.0f)
    {
        const float coef = 1.0f - std::exp (-4.6f * (float) controlRate / (params.glide * srF));
        v.pitch += (v.targetPitch - v.pitch) * coef;
    }
    else
        v.pitch = v.targetPitch;

    const float base = v.pitch + pitchBend * (float) params.bendRange + lfoPitch * 12.0f;

    for (int o = 0; o < 2; ++o)
    {
        const auto& op = params.osc[(size_t) o];
        const float oscPitch = base + (float) (op.octave * 12 + op.semi) + op.fine * 0.01f;
        const int n = op.unison;
        const float d = juce::jlimit (0.0f, 1.0f, op.detune + lfoDetune);
        const float spread = d * d;                     // semitones each side at the outer voices
        const float norm = 1.0f / std::sqrt ((float) n);

        for (int u = 0; u < n; ++u)
        {
            const float pos = n == 1 ? 0.0f : 2.0f * (float) u / (float) (n - 1) - 1.0f;
            v.inc[o][u] = std::min (0.45f, midiToHz (oscPitch + pos * spread) / srF);
            const float pan = pos * op.width;           // -1..1
            const float angle = (pan + 1.0f) * pi * 0.25f;
            v.gainL[o][u] = std::cos (angle) * 1.41421356f * norm * op.level;
            v.gainR[o][u] = std::sin (angle) * 1.41421356f * norm * op.level;
        }
    }

    const float subPitch = base + (float) (params.osc[0].octave * 12) - 12.0f;
    v.subInc = std::min (0.45f, midiToHz (subPitch) / srF);
}

//==============================================================================
// Audio

void SynthEngine::renderVoice (Voice& v, float* L, float* R, int start, int num)
{
    const auto& p = params;
    const float* modCut   = lfoBuf[0].data() + (int) LfoDest::Cutoff    * curBlockSize;
    const float* modPitch = lfoBuf[0].data() + (int) LfoDest::Pitch     * curBlockSize;
    const float* modAmp   = lfoBuf[0].data() + (int) LfoDest::Amp       * curBlockSize;
    const float* modPan   = lfoBuf[0].data() + (int) LfoDest::Pan       * curBlockSize;
    const float* modDet   = lfoBuf[0].data() + (int) LfoDest::Detune    * curBlockSize;
    const float* modRes   = lfoBuf[0].data() + (int) LfoDest::Resonance * curBlockSize;

    const Wave waveA = (Wave) p.osc[0].wave, waveB = (Wave) p.osc[1].wave;
    const Wave subW  = p.subWave == 0 ? Wave::Sine : (p.subWave == 1 ? Wave::Triangle : Wave::Square);
    const int  nA = p.osc[0].on ? p.osc[0].unison : 0;
    const int  nB = p.osc[1].on ? p.osc[1].unison : 0;

    const float driveGain = 1.0f + p.drive * p.drive * 15.0f;
    const float driveNorm = 1.0f / fastTanh (std::min (3.0f, driveGain));
    // Mono/legato play one voice at a time, so give them +6 dB to sit level with poly chords.
    const float modeGain  = p.voiceMode == 0 ? 1.0f : 2.0f;
    const float velGain   = (1.0f - p.velToAmp * (1.0f - v.velocity)) * modeGain;
    const float velOct    = p.velToFilter * (v.velocity - 1.0f) * 3.0f;      // soft hits are darker
    const float keyOct    = p.keyTrack * (v.targetPitch - 60.0f) / 12.0f;
    const float maxFc     = std::min (20000.0f, srF * 0.45f);
    const float piOverSr  = pi / srF;

    for (int i = start; i < start + num; ++i)
    {
        if ((i - start) % controlRate == 0)
            updateVoiceControl (v, modPitch[i], modDet[i]);

        // --- Oscillators -------------------------------------------------------
        float l = 0.0f, r = 0.0f;
        for (int u = 0; u < nA; ++u)
        {
            const float s = oscSample (waveA, v.phase[0][u], v.inc[0][u]);
            l += s * v.gainL[0][u]; r += s * v.gainR[0][u];
            v.phase[0][u] += v.inc[0][u]; if (v.phase[0][u] >= 1.0f) v.phase[0][u] -= 1.0f;
        }
        for (int u = 0; u < nB; ++u)
        {
            const float s = oscSample (waveB, v.phase[1][u], v.inc[1][u]);
            l += s * v.gainL[1][u]; r += s * v.gainR[1][u];
            v.phase[1][u] += v.inc[1][u]; if (v.phase[1][u] >= 1.0f) v.phase[1][u] -= 1.0f;
        }
        if (p.subLevel > 0.0f)
        {
            const float s = oscSample (subW, v.subPhase, v.subInc) * p.subLevel;
            l += s; r += s;
        }
        v.subPhase += v.subInc; if (v.subPhase >= 1.0f) v.subPhase -= 1.0f;
        if (p.noiseLevel > 0.0f)
        {
            l += noise.bipolar() * p.noiseLevel * 0.5f;
            r += noise.bipolar() * p.noiseLevel * 0.5f;
        }

        // --- Drive (pre-filter, so resonance stays clean and the bass gets grit) -
        if (p.drive > 0.0f)
        {
            l = fastTanh (l * driveGain) * driveNorm;
            r = fastTanh (r * driveGain) * driveNorm;
        }

        // --- Filter --------------------------------------------------------------
        const float fe = v.filtEnv.next();
        const float oct = p.filterEnvAmt * fe * 7.0f + keyOct + velOct + modCut[i] * 4.0f;
        const float fc = juce::jlimit (20.0f, maxFc, p.cutoff * std::exp2 (oct));
        const float g = std::tan (fc * piOverSr);
        const float res = juce::jlimit (0.0f, 1.0f, p.resonance + modRes[i] * 0.5f);
        const float k = 2.0f - 1.96f * res;

        float lp, bp, hp;
        switch (p.filterType)
        {
            case 0: // 24 dB low-pass: Butterworth stage into a resonant stage
                v.svf[0][0].setCoeffs (g, 1.8478f); v.svf[0][1].setCoeffs (g, 1.8478f);
                v.svf[1][0].setCoeffs (g, k);       v.svf[1][1].setCoeffs (g, k);
                v.svf[0][0].process (l, lp, bp, hp); v.svf[1][0].process (lp, l, bp, hp);
                v.svf[0][1].process (r, lp, bp, hp); v.svf[1][1].process (lp, r, bp, hp);
                break;
            case 1:
                v.svf[0][0].setCoeffs (g, k); v.svf[0][1].setCoeffs (g, k);
                v.svf[0][0].process (l, l, bp, hp);
                v.svf[0][1].process (r, r, bp, hp);
                break;
            case 2:
                v.svf[0][0].setCoeffs (g, k); v.svf[0][1].setCoeffs (g, k);
                v.svf[0][0].process (l, lp, bp, hp); l = bp * k;
                v.svf[0][1].process (r, lp, bp, hp); r = bp * k;
                break;
            default:
                v.svf[0][0].setCoeffs (g, k); v.svf[0][1].setCoeffs (g, k);
                v.svf[0][0].process (l, lp, bp, l);
                v.svf[0][1].process (r, lp, bp, r);
                break;
        }

        // --- Amp -----------------------------------------------------------------
        const float amp = v.ampEnv.next() * velGain * (1.0f + modAmp[i]) * voiceGain;
        const float pan = juce::jlimit (-1.0f, 1.0f, modPan[i]);
        L[i] += l * amp * std::min (1.0f, 1.0f - pan);
        R[i] += r * amp * std::min (1.0f, 1.0f + pan);

        if (! v.ampEnv.isActive())
        {
            v.note = -1;
            v.gate = v.sustained = false;
            v.filtEnv.reset();
            break;
        }
    }
}

void SynthEngine::process (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi)
{
    const int numSamples = buffer.getNumSamples();
    curBlockSize = numSamples;
    buffer.clear();

    if ((int) (lfoBuf[0].size()) < numDests * numSamples)   // host sent a larger block than promised
        lfoBuf[0].resize ((size_t) (numDests * numSamples), 0.0f);

    // Leaving poly mode: release extra voices so nothing hangs.
    if (params.voiceMode != 0)
        for (size_t i = 1; i < voices.size(); ++i)
            if (voices[i].gate) { voices[i].gate = false; voices[i].ampEnv.noteOff(); voices[i].filtEnv.noteOff(); }

    for (auto& v : voices)
    {
        v.ampEnv.setParams  (params.aA, params.aD, params.aS, params.aR);
        v.filtEnv.setParams (params.fA, params.fD, params.fS, params.fR);
    }

    renderLfos (numSamples);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    std::vector<float>& scratchR = lfoBuf[1];
    if (R == nullptr)
    {
        if ((int) scratchR.size() < numSamples) scratchR.resize ((size_t) numSamples);
        std::fill (scratchR.begin(), scratchR.begin() + numSamples, 0.0f);
        R = scratchR.data();
    }

    int pos = 0;
    auto renderTo = [&] (int end)
    {
        if (end <= pos) return;
        for (auto& v : voices)
            if (v.isActive())
                renderVoice (v, L, R, pos, end - pos);
        pos = end;
    };

    for (const auto meta : midi)
    {
        renderTo (juce::jlimit (0, numSamples, meta.samplePosition));
        handleMidi (meta.getMessage());
    }
    renderTo (numSamples);

    if (buffer.getNumChannels() == 1)   // mono host bus: fold the right side in
    {
        juce::FloatVectorOperations::add (L, R, numSamples);
        juce::FloatVectorOperations::multiply (L, 0.5f, numSamples);
    }
}

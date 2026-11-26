#include "Effects.h"
#include "DspUtils.h"

void EffectsChain::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    const juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlockSize, 2 };
    chorus.prepare (spec);
    reverb.prepare (spec);

    const auto maxDelay = (size_t) (sr * 4.5) + 8;   // 1/2 note at 30 BPM fits
    for (auto& b : delayBuf) b.assign (maxDelay, 0.0f);

    delayTimeSmoothed.reset (sr, 0.12);
    gainSmoothed.reset (sr, 0.02);
    reset();
}

void EffectsChain::reset()
{
    chorus.reset();
    reverb.reset();
    for (auto& b : delayBuf) std::fill (b.begin(), b.end(), 0.0f);
    writePos = 0;
    toneState[0] = toneState[1] = hpState[0] = hpState[1] = 0.0f;
}

void EffectsChain::processDelay (float* L, float* R, int n, const EffectParams& p)
{
    const int size = (int) delayBuf[0].size();
    delayTimeSmoothed.setTargetValue (juce::jlimit (1.0f, (float) size - 4.0f, p.delaySeconds * (float) sampleRate));

    // One-pole low-pass in the feedback path: "tone" 0 = dark tape echo, 1 = bright digital.
    const float toneHz = 800.0f * std::pow (20.0f, p.delayTone);           // 800 Hz .. 16 kHz
    const float lpCoef = 1.0f - std::exp (-mdsp::twoPi * toneHz / (float) sampleRate);
    const float hpCoef = 1.0f - std::exp (-mdsp::twoPi * 150.0f / (float) sampleRate); // keep lows out of the echoes
    const float fb = std::min (0.95f, p.delayFeedback);

    float* bufL = delayBuf[0].data();
    float* bufR = delayBuf[1].data();

    for (int i = 0; i < n; ++i)
    {
        const float d = delayTimeSmoothed.getNextValue();
        float readPos = (float) writePos - d;
        if (readPos < 0.0f) readPos += (float) size;
        const int i0 = (int) readPos;
        const int i1 = (i0 + 1) % size;
        const float frac = readPos - (float) i0;
        float wl = bufL[i0] + frac * (bufL[i1] - bufL[i0]);
        float wr = bufR[i0] + frac * (bufR[i1] - bufR[i0]);

        // Tone filtering of the echoes
        toneState[0] += (wl - toneState[0]) * lpCoef; wl = toneState[0];
        toneState[1] += (wr - toneState[1]) * lpCoef; wr = toneState[1];
        hpState[0] += (wl - hpState[0]) * hpCoef; wl -= hpState[0];
        hpState[1] += (wr - hpState[1]) * hpCoef; wr -= hpState[1];

        const float inL = L[i], inR = R[i];
        if (p.delayPingPong)
        {
            // Mono input enters on the left, echoes bounce L -> R -> L.
            bufL[writePos] = 0.5f * (inL + inR) + mdsp::fastTanh (wr * fb);
            bufR[writePos] = mdsp::fastTanh (wl * fb);
        }
        else
        {
            bufL[writePos] = inL + mdsp::fastTanh (wl * fb);
            bufR[writePos] = inR + mdsp::fastTanh (wr * fb);
        }

        L[i] = inL + wl * p.delayMix;
        R[i] = inR + wr * p.delayMix;

        if (++writePos >= size) writePos = 0;
    }
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer, const EffectParams& p)
{
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || n == 0) return;

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    if (p.chorusMix > 0.001f)
    {
        chorus.setRate (p.chorusRate);
        chorus.setDepth (p.chorusDepth);
        chorus.setCentreDelay (7.0f);
        chorus.setFeedback (0.0f);
        chorus.setMix (p.chorusMix * 0.5f);   // juce Chorus mix 0.5 = equal dry/wet, the classic ensemble point
        juce::dsp::AudioBlock<float> full (buffer);
        auto block = full.getSubsetChannelBlock (0, 2);
        chorus.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    if (p.delayMix > 0.001f)
        processDelay (L, R, n, p);

    if (p.reverbMix > 0.001f)
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = p.reverbSize;
        rp.damping  = p.reverbDamp;
        rp.width    = p.reverbWidth;
        rp.wetLevel = p.reverbMix * 0.6f;
        rp.dryLevel = 1.0f - p.reverbMix * 0.35f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);
        juce::dsp::AudioBlock<float> full (buffer);
        auto block = full.getSubsetChannelBlock (0, 2);
        reverb.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    gainSmoothed.setTargetValue (p.masterGain);
    for (int i = 0; i < n; ++i)
    {
        const float g = gainSmoothed.getNextValue();
        // Gentle safety clip: transparent below ~-6 dBFS, rounds off overs instead of hard clipping.
        auto clip = [] (float x) { return std::abs (x) < 0.5f ? x : std::copysign (0.5f + 0.5f * mdsp::fastTanh ((std::abs (x) - 0.5f) * 2.0f), x); };
        L[i] = clip (L[i] * g);
        R[i] = clip (R[i] * g);
    }
}

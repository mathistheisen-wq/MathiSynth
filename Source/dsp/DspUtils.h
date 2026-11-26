#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>

namespace mdsp
{
    constexpr float pi    = 3.14159265358979f;
    constexpr float twoPi = 6.28318530717959f;

    inline float midiToHz (float note) noexcept { return 440.0f * std::exp2 ((note - 69.0f) / 12.0f); }

    // Cheap, smooth saturator (Padé tanh approximation, clamped so it never exceeds +-1).
    inline float fastTanh (float x) noexcept
    {
        x = std::clamp (x, -3.0f, 3.0f);
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Small xorshift RNG — realtime safe, no locks, no allocation.
    struct Random
    {
        uint32_t state = 0x9E3779B9u;
        void seed (uint32_t s) noexcept { state = s ? s : 0x9E3779B9u; }
        uint32_t next() noexcept { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
        float bipolar() noexcept { return (float) next() * (2.0f / 4294967295.0f) - 1.0f; }
        float unipolar() noexcept { return (float) next() * (1.0f / 4294967295.0f); }
    };

    // PolyBLEP residual: removes the aliasing step discontinuity of naive saw/square waves.
    inline float polyBlep (float t, float dt) noexcept
    {
        if (t < dt)        { t /= dt;            return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    enum class Wave { Sine = 0, Triangle, Saw, Square, Pulse25 };

    // One band-limited sample of a waveform at normalised phase [0,1) and increment dt (freq/sr).
    inline float oscSample (Wave w, float phase, float dt) noexcept
    {
        switch (w)
        {
            case Wave::Sine:     return std::sin (twoPi * phase);
            case Wave::Triangle: return 1.0f - 4.0f * std::abs (phase - 0.5f);  // harmonics fall at 12dB/oct, aliasing negligible
            case Wave::Saw:      return (2.0f * phase - 1.0f) - polyBlep (phase, dt);
            case Wave::Square:
            case Wave::Pulse25:
            {
                const float pw = (w == Wave::Square) ? 0.5f : 0.25f;
                float v = phase < pw ? 1.0f : -1.0f;
                v += polyBlep (phase, dt);
                float p2 = phase + (1.0f - pw);
                if (p2 >= 1.0f) p2 -= 1.0f;
                v -= polyBlep (p2, dt);
                return (w == Wave::Pulse25) ? v + 0.5f : v;   // remove DC offset of 25% pulse
            }
        }
        return 0.0f;
    }

    // Zero-delay-feedback state-variable filter (Zavalishin / Simper TPT form).
    // Stable under fast per-sample cutoff modulation, which is what plucks and wobbles need.
    struct SVF
    {
        float ic1 = 0.0f, ic2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f, a3 = 0.0f, k = 2.0f;

        void reset() noexcept { ic1 = ic2 = 0.0f; }

        void setCoeffs (float g, float kDamping) noexcept
        {
            k  = kDamping;
            a1 = 1.0f / (1.0f + g * (g + k));
            a2 = g * a1;
            a3 = g * a2;
        }

        // Returns low/band/high outputs simultaneously.
        inline void process (float v0, float& lp, float& bp, float& hp) noexcept
        {
            const float v3 = v0 - ic2;
            const float v1 = a1 * ic1 + a2 * v3;
            const float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            lp = v2; bp = v1; hp = v0 - k * v1 - v2;
        }
    };

    // Linear-segment ADSR with exponential-feeling curves via per-stage one-pole targets.
    // Unlike juce::ADSR it supports legato re-triggering without resetting to zero.
    struct Envelope
    {
        enum class Stage { Idle, Attack, Decay, Sustain, Release };

        Stage stage = Stage::Idle;
        float value = 0.0f;
        float sampleRate = 44100.0f;
        float attack = 0.01f, decay = 0.3f, sustain = 0.7f, release = 0.3f;
        float attackInc = 0.0f, decayCoef = 0.0f, releaseCoef = 0.0f;

        void setSampleRate (double sr) noexcept { sampleRate = (float) sr; }

        void setParams (float a, float d, float s, float r) noexcept
        {
            attack = a; decay = d; sustain = s; release = r;
            attackInc   = 1.0f / std::max (1.0f, a * sampleRate);
            // Coefficient so the curve covers ~99% of the distance in the stage time (time constant = t/4.6).
            decayCoef   = 1.0f - std::exp (-4.6f / std::max (1.0f, d * sampleRate));
            releaseCoef = 1.0f - std::exp (-4.6f / std::max (1.0f, r * sampleRate));
        }

        void noteOn() noexcept  { stage = Stage::Attack; }   // starts from current value: no click on retrigger
        void noteOff() noexcept { if (stage != Stage::Idle) stage = Stage::Release; }
        void reset() noexcept   { stage = Stage::Idle; value = 0.0f; }
        bool isActive() const noexcept { return stage != Stage::Idle; }

        inline float next() noexcept
        {
            switch (stage)
            {
                case Stage::Idle: break;
                case Stage::Attack:
                    value += attackInc;
                    if (value >= 1.0f) { value = 1.0f; stage = Stage::Decay; }
                    break;
                case Stage::Decay:
                    value += (sustain - value) * decayCoef;
                    if (std::abs (value - sustain) < 1.0e-4f)
                    {
                        value = sustain;
                        // A zero-sustain envelope (plucks) is finished here, so the voice can be freed.
                        stage = sustain <= 1.0e-4f ? Stage::Idle : Stage::Sustain;
                    }
                    break;
                case Stage::Sustain:
                    value = sustain;
                    break;
                case Stage::Release:
                    value -= value * releaseCoef;
                    if (value < 1.0e-5f) { value = 0.0f; stage = Stage::Idle; }
                    break;
            }
            return value;
        }
    };

    // Global (per-instance) LFO, bipolar output in [-1, 1].
    struct Lfo
    {
        float phase = 0.0f;
        float held = 0.0f;
        Random rng;

        inline float next (int shape, float inc) noexcept
        {
            float out = 0.0f;
            switch (shape)
            {
                case 0:  out = std::sin (twoPi * phase); break;
                case 1:  out = 1.0f - 4.0f * std::abs (phase - 0.5f); break;
                case 2:  out = 1.0f - 2.0f * phase; break;              // ramp down: classic "wub" shape
                case 3:  out = phase < 0.5f ? 1.0f : -1.0f; break;
                default: out = held; break;
            }
            phase += inc;
            if (phase >= 1.0f)
            {
                phase -= std::floor (phase);
                held = rng.bipolar();
            }
            return out;
        }
    };
}

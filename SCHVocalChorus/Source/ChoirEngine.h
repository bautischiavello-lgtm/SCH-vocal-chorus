#pragma once

// ============================================================================
//  SCH VOCAL CHORUS - DSP voice generator (header-only, no JUCE dependency)
//
//  One shared mono ring buffer feeds up to 32 voices. Every voice has:
//    * a time-domain pitch shifter (two crossfaded delay taps, no FFT)
//    * an independent random micro-delay (slew limited, re-targeted randomly)
//    * an independent pitch-drift LFO
//    * two peaking biquads acting as formant shapers (F1 / F2)
//    * optional tanh excitation (Shout mode)
//    * a fixed random pan position scaled by WIDTH
//
//  All memory is allocated in prepare(). process() never allocates.
// ============================================================================

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace sch
{
constexpr int kMaxVoices = 32;

enum class ChoirMode { Classic = 0, Gospel, Kids, Shout, Custom };

struct ModeProfile
{
    float pitchBiasCents;   // constant pitch offset applied to every voice
    float driftMult;        // scales the per-voice pitch drift LFO depth
    float delayMult;        // scales the micro-timing range
    float f1Hz, f1GainDb;   // first formant region
    float f2Hz, f2GainDb;   // second formant region
    float formantScale;     // global formant shift (1 = neutral)
    float formantSpread;    // per-voice random formant variation (+/- fraction)
    float drive;            // >1 enables tanh saturation per voice
};

inline const ModeProfile& getModeProfile (ChoirMode m)
{
    static const ModeProfile profiles[5] = {
        //  bias  drift delay   f1    g1    f2     g2   scale spread drive
        {   0.0f, 1.00f, 1.00f, 600.f, 2.0f, 1800.f, 1.5f, 1.00f, 0.12f, 1.0f }, // Classic
        {   0.0f, 1.80f, 1.30f, 450.f, 4.5f, 1300.f, 1.0f, 0.95f, 0.18f, 1.0f }, // Gospel
        {  45.0f, 1.00f, 0.80f, 800.f, 2.0f, 3000.f, 4.0f, 1.25f, 0.10f, 1.0f }, // Kids
        {   0.0f, 0.70f, 0.60f, 700.f, 5.0f, 1900.f, 6.0f, 1.05f, 0.10f, 3.0f }, // Shout
        {   0.0f, 1.00f, 1.00f, 600.f, 1.5f, 1800.f, 1.0f, 1.00f, 0.20f, 1.0f }  // Custom
    };
    return profiles[(int) m];
}

struct Params
{
    float mix         = 0.5f;   // 0..1
    float detuneCents = 12.0f;  // -20..+20 (|x| = spread, sign = flat/sharp skew)
    float delayMs     = 20.0f;  // 0..50
    float width       = 1.0f;   // 0..2
    ChoirMode mode    = ChoirMode::Classic;
    bool mono         = false;
    int voices        = 16;     // 1..32
};

// ---------------------------------------------------------------------------
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    void setPeaking (double sr, double freq, double q, double gainDb)
    {
        freq = std::min (freq, sr * 0.45);
        const double A     = std::pow (10.0, gainDb / 40.0);
        const double w0    = 6.283185307179586 * freq / sr;
        const double cw    = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * q);
        const double a0    = 1.0 + alpha / A;
        b0 = (float) ((1.0 + alpha * A) / a0);
        b1 = (float) ((-2.0 * cw) / a0);
        b2 = (float) ((1.0 - alpha * A) / a0);
        a1 = b1;
        a2 = (float) ((1.0 - alpha / A) / a0);
    }

    inline float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = z2 = 0; }
};

// ---------------------------------------------------------------------------
class ChoirEngine
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        int size = 1024;
        while ((double) size < sr * 0.25) size <<= 1;      // >= 250 ms, power of two
        ring.assign ((size_t) size, 0.0f);                 // allocation happens HERE only
        ringMask = size - 1;
        grain = (float) (0.030 * sr);                      // 30 ms delay sweep range
        mixSmoothed = -1.0f;
        mixCoef = 1.0f - std::exp (-1.0f / (0.02f * (float) sr));

        for (int i = 0; i < kMaxVoices; ++i)
        {
            auto& v = voices[(size_t) i];
            v = Voice();
            v.seedPitch   = 2.0f * frac (i * 0.7548776662f + 0.30f) - 1.0f;
            v.seedFormant = 2.0f * frac (i * 0.5698402910f + 0.10f) - 1.0f;
            v.seedPan     = 2.0f * frac (i * 0.6180339887f + 0.17f) - 1.0f;
            v.seedDelay   = frac (i * 0.8191725134f + 0.50f);
            v.lfoPhase    = 6.2831853f * frac (i * 0.3247179572f);
            v.lfoInc      = 6.2831853f * (0.15f + 0.70f * frac (i * 0.4142135623f + 0.2f)) / (float) sr;
            v.rng         = 0x9E3779B9u * (uint32_t) (i + 1);
            v.retarget    = (int) ((1.0f + 2.0f * v.seedDelay) * (float) sr);
        }
        reset();
    }

    void reset()
    {
        std::fill (ring.begin(), ring.end(), 0.0f);
        writeIdx = 0;
        for (auto& v : voices)
        {
            v.gain = 0; v.active = false;
            v.f1.reset(); v.f2.reset();
        }
    }

    // L is required. R may be nullptr (mono bus). In-place processing.
    void process (float* L, float* R, int numSamples, const Params& p)
    {
        if (ring.empty() || L == nullptr) return;

        updateFormants (p);

        int pos = 0;
        while (pos < numSamples)
        {
            const int c = std::min (kChunk, numSamples - pos);
            processChunk (L + pos, R != nullptr ? R + pos : nullptr, c, p);
            pos += c;
        }
    }

private:
    static constexpr int kChunk = 32;

    struct Voice
    {
        float phase = 0, delay = 2, delayTarget = 2;
        float gain = 0;
        float lfoPhase = 0, lfoInc = 0;
        float seedPitch = 0, seedDelay = 0.5f, seedPan = 0, seedFormant = 0;
        float pitchInc = 0;
        float panL = 0.7071f, panR = 0.7071f, panLT = 0.7071f, panRT = 0.7071f;
        uint32_t rng = 1;
        int retarget = 0;
        bool active = false;
        Biquad f1, f2;
    };

    static float frac (float x) { return x - std::floor (x); }

    static float rand01 (uint32_t& s)
    {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return (float) (s & 0xFFFFFF) / 16777216.0f;
    }

    void updateFormants (const Params& p)
    {
        const auto& m = getModeProfile (p.mode);
        for (auto& v : voices)
        {
            const double k = (double) m.formantScale * (1.0 + (double) m.formantSpread * v.seedFormant);
            v.f1.setPeaking (sr, m.f1Hz * k, 1.4, m.f1GainDb);
            v.f2.setPeaking (sr, m.f2Hz * k, 1.4, m.f2GainDb);
        }
    }

    inline float readTap (int idx, float delay) const
    {
        const float posf = (float) (idx & ringMask) - delay;
        const int   i0   = (int) std::floor (posf);
        const float fr   = posf - (float) i0;
        const float a = ring[(size_t) ((i0 + (int) ring.size()) & ringMask)];
        const float b = ring[(size_t) ((i0 + 1 + (int) ring.size()) & ringMask)];
        return a + (b - a) * fr;
    }

    static inline float fadeWeight (float ph)
    {
        const float x = std::min (1.0f, std::min (ph, 1.0f - ph) / 0.15f);
        return x * x * (3.0f - 2.0f * x);
    }

    void processChunk (float* L, float* R, int c, const Params& p)
    {
        const auto& m = getModeProfile (p.mode);

        // 1. write input (mono sum) into the shared ring buffer
        for (int i = 0; i < c; ++i)
        {
            const float mono = R != nullptr ? 0.5f * (L[i] + R[i]) : L[i];
            ring[(size_t) ((writeIdx + i) & ringMask)] = mono;
        }

        std::array<float, kChunk> wetL {}, wetR {};

        const int   nVoices = std::clamp (p.voices, 1, kMaxVoices);
        const float norm    = 1.0f / std::pow ((float) nVoices, 0.7f); // between coherent (1/N) and incoherent (1/sqrt N) sums
        const float spread  = std::abs (p.detuneCents);
        const float skew    = 0.25f * p.detuneCents;
        const float delaySamples = p.delayMs * m.delayMult * 0.001f * (float) sr;
        const float drive   = m.drive;
        const float invTanh = drive > 1.0f ? 1.0f / std::tanh (drive) : 1.0f;
        const float gainSlew = 0.002f;
        const float delaySlew = 0.012f;   // samples per sample: keeps pitch bend small

        for (int vi = 0; vi < kMaxVoices; ++vi)
        {
            auto& v = voices[(size_t) vi];
            // normalisation lives inside the smoothed gain, so changing AMOUNT never jumps
            const float gTarget = vi < nVoices ? norm : 0.0f;

            if (gTarget == 0.0f && v.gain < 1.0e-5f)
            {
                v.gain = 0; v.active = false;
                v.f1.reset(); v.f2.reset();
                continue;
            }

            // ----- control-rate update (once per chunk) -----
            v.lfoPhase += v.lfoInc * (float) c;
            if (v.lfoPhase > 6.2831853f) v.lfoPhase -= 6.2831853f;

            v.retarget -= c;
            if (v.retarget <= 0)
            {
                v.seedDelay = rand01 (v.rng);
                v.retarget = (int) ((1.0f + 2.0f * rand01 (v.rng)) * (float) sr);
            }
            v.delayTarget = 2.0f + v.seedDelay * delaySamples;

            const float cents = m.pitchBiasCents + skew + spread * v.seedPitch
                              + 0.5f * spread * m.driftMult * std::sin (v.lfoPhase);
            const float ratio = std::exp2 (cents / 1200.0f);
            v.pitchInc = (1.0f - ratio) / grain;

            const float pan = std::clamp (v.seedPan * p.width, -1.0f, 1.0f);
            const float ang = (pan + 1.0f) * 0.7853981634f;
            v.panLT = std::cos (ang);
            v.panRT = std::sin (ang);

            if (! v.active)                     // (re)activated: jump silently
            {
                v.delay = v.delayTarget;
                v.panL = v.panLT; v.panR = v.panRT;
                v.active = true;
            }

            // ----- audio-rate loop -----
            float ph = v.phase, d = v.delay, g = v.gain, pl = v.panL, pr = v.panR;
            for (int i = 0; i < c; ++i)
            {
                ph += v.pitchInc;
                if (ph >= 1.0f) ph -= 1.0f; else if (ph < 0.0f) ph += 1.0f;
                float ph2 = ph + 0.5f;
                if (ph2 >= 1.0f) ph2 -= 1.0f;

                d += std::clamp (v.delayTarget - d, -delaySlew, delaySlew);

                const int   idx = writeIdx + i;
                const float w   = fadeWeight (ph);
                float s = readTap (idx, ph * grain + d) * w
                        + readTap (idx, ph2 * grain + d) * (1.0f - w);

                s = v.f1.process (s);
                s = v.f2.process (s);
                if (drive > 1.0f) s = std::tanh (drive * s) * invTanh;

                g  += (gTarget - g) * gainSlew;
                pl += (v.panLT - pl) * 0.01f;
                pr += (v.panRT - pr) * 0.01f;

                s *= g;
                wetL[(size_t) i] += s * pl;
                wetR[(size_t) i] += s * pr;
            }
            v.phase = ph; v.delay = d; v.gain = g; v.panL = pl; v.panR = pr;
        }

        // 2. dry/wet, mono-sum, write back
        constexpr float wetTrim = 1.3f;
        if (mixSmoothed < 0.0f) mixSmoothed = p.mix;

        for (int i = 0; i < c; ++i)
        {
            mixSmoothed += (p.mix - mixSmoothed) * mixCoef;
            const float dryL = L[i];
            const float dryR = R != nullptr ? R[i] : dryL;
            float outL = dryL * (1.0f - mixSmoothed) + wetL[(size_t) i] * wetTrim * mixSmoothed;
            float outR = dryR * (1.0f - mixSmoothed) + wetR[(size_t) i] * wetTrim * mixSmoothed;

            if (p.mono) outL = outR = 0.5f * (outL + outR);

            L[i] = outL;
            if (R != nullptr) R[i] = outR;
        }

        writeIdx = (writeIdx + c) & ringMask;
    }

    double sr = 44100.0;
    float grain = 1323.0f;
    std::vector<float> ring;
    int ringMask = 0, writeIdx = 0;
    float mixSmoothed = -1.0f, mixCoef = 0.01f;
    std::array<Voice, kMaxVoices> voices;
};
} // namespace sch

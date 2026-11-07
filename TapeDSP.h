#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <vector>

//==============================================================================
// Biquad (RBJ-Kochbuch), Transposed Direct Form II, Stereo, ohne Allokation
//==============================================================================
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1[2] {}, z2[2] {};

    void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0.0f; }

    float process (float x, int ch)
    {
        const float y = b0 * x + z1[ch];
        z1[ch] = b1 * x - a1 * y + z2[ch];
        z2[ch] = b2 * x - a2 * y;
        return y;
    }

    void setRaw (double nb0, double nb1, double nb2, double na0, double na1, double na2)
    {
        b0 = (float) (nb0 / na0); b1 = (float) (nb1 / na0); b2 = (float) (nb2 / na0);
        a1 = (float) (na1 / na0); a2 = (float) (na2 / na0);
    }

    static double w (double f, double fs) { return juce::MathConstants<double>::twoPi * juce::jlimit (5.0, fs * 0.49, f) / fs; }

    void lowpass (double fs, double f, double q)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        setRaw ((1 - cs) / 2, 1 - cs, (1 - cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void highpass (double fs, double f, double q)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        setRaw ((1 + cs) / 2, -(1 + cs), (1 + cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void peak (double fs, double f, double q, double gainDb)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        const double A = std::pow (10.0, gainDb / 40.0);
        setRaw (1 + al * A, -2 * cs, 1 - al * A, 1 + al / A, -2 * cs, 1 - al / A);
    }
    void highShelf (double fs, double f, double gainDb)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * 0.707);
        const double A = std::pow (10.0, gainDb / 40.0), sq = 2.0 * std::sqrt (A) * al;
        setRaw (A * ((A + 1) + (A - 1) * cs + sq), -2 * A * ((A - 1) + (A + 1) * cs), A * ((A + 1) + (A - 1) * cs - sq),
                (A + 1) - (A - 1) * cs + sq, 2 * ((A - 1) - (A + 1) * cs), (A + 1) - (A - 1) * cs - sq);
    }
};

//==============================================================================
// Bandtransport: Ringpuffer mit variabler Lesegeschwindigkeit.
// Wow, Flutter und Tape-Stop entstehen durch Modulation der Leseposition.
//==============================================================================
class TapeTransport
{
public:
    void prepare (double sampleRate, float baseDelayMs)
    {
        fs = sampleRate;
        int size = 1;
        while (size < (int) (4.2 * fs)) size <<= 1;
        for (auto& b : buf) b.assign ((size_t) size, 0.0f);
        mask = size - 1;
        w = 0;
        base = baseDelayMs * 0.001f * (float) fs;
        delay = base;
        rate = 1.0f;
        xfade = 0.0f;
        maxDelay = (float) (4.0 * fs);
    }

    float getBaseDelay() const { return base; }
    float getRate() const { return rate; }

    // modSamples: Wow/Flutter-Versatz in Samples. stopped: Tape-Stop aktiv.
    void process (float inL, float inR, float modSamples, bool stopped, float& outL, float& outR)
    {
        buf[0][(size_t) w] = inL;
        buf[1][(size_t) w] = inR;

        // Bandgeschwindigkeit hoch-/runterfahren
        if (stopped) rate = juce::jmax (0.0f, rate - stopStep);
        else         rate = juce::jmin (1.0f, rate + startStep);

        delay = juce::jmin (maxDelay, delay + (1.0f - rate));

        // Nach dem Wiederanlauf: Rücksprung zur normalen Latenz mit kurzer Überblendung
        if (! stopped && rate >= 1.0f && delay > base + 1.0f && xfade <= 0.0f)
        {
            oldDelay = delay;
            delay = base;
            xfade = 1.0f;
        }

        float l = read (0, delay + modSamples), r = read (1, delay + modSamples);
        if (xfade > 0.0f)
        {
            const float g = xfade * xfade;
            l = l * (1.0f - g) + read (0, oldDelay + modSamples) * g;
            r = r * (1.0f - g) + read (1, oldDelay + modSamples) * g;
            xfade -= xfadeStep;
        }

        const float gain = juce::jmin (1.0f, rate * 4.0f);   // stehendes Band = Stille
        outL = l * gain;
        outR = r * gain;

        w = (w + 1) & mask;
    }

    void setTimes (float stopSec, float startSec)
    {
        stopStep  = 1.0f / (stopSec  * (float) fs);
        startStep = 1.0f / (startSec * (float) fs);
        xfadeStep = 1.0f / (0.03f * (float) fs);
    }

private:
    // kubische Hermite-Interpolation
    float read (int ch, float d) const
    {
        d = juce::jlimit (2.0f, maxDelay, d);
        const float pos = (float) w - d;
        const int i = (int) std::floor (pos);
        const float f = pos - (float) i;
        const auto& b = buf[ch];
        const float xm1 = b[(size_t) ((i - 1) & mask)], x0 = b[(size_t) (i & mask)];
        const float x1  = b[(size_t) ((i + 1) & mask)], x2 = b[(size_t) ((i + 2) & mask)];
        const float c1 = 0.5f * (x1 - xm1);
        const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        return ((c3 * f + c2) * f + c1) * f + x0;
    }

    std::vector<float> buf[2];
    int mask = 0, w = 0;
    double fs = 44100.0;
    float base = 240.0f, delay = 240.0f, oldDelay = 0.0f, maxDelay = 1.0f;
    float rate = 1.0f, xfade = 0.0f;
    float stopStep = 0.0001f, startStep = 0.0001f, xfadeStep = 0.001f;
};

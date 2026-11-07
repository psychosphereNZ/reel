#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    struct Preset
    {
        const char* name;
        int speed;  // 0 = 7.5 IPS, 1 = 15 IPS, 2 = 30 IPS
        float input, sat, bump, tone, hiss, wow, flutter, wear, mix, output;
    };

    const Preset presets[] =
    {
        //  name              spd  in    sat    bump  tone   hiss  wow   flut  wear  mix   out
        { "Studio 15",        1,  0.f, 0.35f, 0.50f,  0.0f, 0.10f, 0.10f, 0.15f, 0.0f, 1.0f,  0.f },
        { "Hot 30 IPS",       2,  4.f, 0.60f, 0.40f,  0.1f, 0.05f, 0.05f, 0.10f, 0.0f, 1.0f, -2.f },
        { "Cassette Deck",    0,  0.f, 0.45f, 0.30f, -0.3f, 0.35f, 0.35f, 0.40f, 0.2f, 1.0f,  0.f },
        { "Lo-Fi Memory",     0,  2.f, 0.50f, 0.40f, -0.6f, 0.45f, 0.60f, 0.50f, 0.5f, 1.0f, -1.f },
        { "Warped VHS",       0,  0.f, 0.40f, 0.20f, -0.5f, 0.40f, 0.90f, 0.30f, 0.4f, 1.0f,  0.f },
        { "Mastering Glue",   2,  0.f, 0.25f, 0.30f, 0.05f, 0.00f, 0.00f, 0.05f, 0.0f, 1.0f,  0.f },
        { "Broken Walkman",   0,  0.f, 0.70f, 0.10f, -0.8f, 0.60f, 0.80f, 0.90f, 0.8f, 1.0f, -2.f },
        { "Init (Clean)",     2,  0.f, 0.00f, 0.00f,  0.0f, 0.00f, 0.00f, 0.00f, 0.0f, 1.0f,  0.f },
    };
    constexpr int numPresets = (int) (sizeof (presets) / sizeof (presets[0]));

    juce::String pct (float v) { return juce::String (juce::roundToInt (v * 100.f)) + " %"; }
    juce::String db  (float v) { return (v > 0 ? "+" : "") + juce::String (v, 1) + " dB"; }

    // Geschwindigkeit: Head-Bump-Frequenz und Höhengrenze
    constexpr float bumpHz[3] = { 45.0f, 75.0f, 120.0f };
    constexpr float hfHz[3]   = { 9000.0f, 15000.0f, 19500.0f };
}

//==============================================================================
Reel15AudioProcessor::Reel15AudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pSpeed   = apvts.getRawParameterValue ("speed");
    pInput   = apvts.getRawParameterValue ("input");
    pSat     = apvts.getRawParameterValue ("saturate");
    pBump    = apvts.getRawParameterValue ("bump");
    pTone    = apvts.getRawParameterValue ("tone");
    pHiss    = apvts.getRawParameterValue ("hiss");
    pWow     = apvts.getRawParameterValue ("wow");
    pFlutter = apvts.getRawParameterValue ("flutter");
    pWear    = apvts.getRawParameterValue ("wear");
    pMix     = apvts.getRawParameterValue ("mix");
    pOutput  = apvts.getRawParameterValue ("output");
    pStop    = apvts.getRawParameterValue ("stop");
}

juce::AudioProcessorValueTreeState::ParameterLayout Reel15AudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using A = juce::AudioParameterFloatAttributes;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    const auto& d = presets[0];

    auto pctAttr = A().withStringFromValueFunction ([] (float v, int) { return pct (v); });
    auto dbAttr  = A().withStringFromValueFunction ([] (float v, int) { return db (v); });
    auto unit    = [] { return juce::NormalisableRange<float> (0.f, 1.f, 0.001f); };

    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "speed", 1 }, "Tape Speed",
                                                         juce::StringArray { "7.5 IPS", "15 IPS", "30 IPS" }, d.speed));
    l.add (std::make_unique<P> (juce::ParameterID { "input", 1 }, "Input",
                                juce::NormalisableRange<float> (-18.f, 18.f, 0.1f), d.input, dbAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "saturate", 1 }, "Saturate", unit(), d.sat, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "bump", 1 }, "Head Bump", unit(), d.bump, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "tone", 1 }, "Tone",
                                juce::NormalisableRange<float> (-1.f, 1.f, 0.001f), d.tone,
                                A().withStringFromValueFunction ([] (float v, int)
                                {
                                    const int p = juce::roundToInt (v * 100.f);
                                    return p == 0 ? juce::String ("neutral") : (p > 0 ? "hell +" : "dunkel ") + juce::String (std::abs (p));
                                })));
    l.add (std::make_unique<P> (juce::ParameterID { "hiss", 1 }, "Hiss", unit(), d.hiss, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "wow", 1 }, "Wow", unit(), d.wow, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "flutter", 1 }, "Flutter", unit(), d.flutter, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "wear", 1 }, "Wear", unit(), d.wear, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "mix", 1 }, "Mix", unit(), d.mix, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "output", 1 }, "Output",
                                juce::NormalisableRange<float> (-24.f, 12.f, 0.1f), d.output, dbAttr));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "stop", 1 }, "Tape Stop", false));
    return l;
}

//==============================================================================
bool Reel15AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void Reel15AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;
    const auto numCh = (juce::uint32) juce::jmax (1, getTotalNumOutputChannels());

    transport.prepare (fs, 6.0f);   // 6 ms Grundlatenz als Spielraum für Wow/Flutter
    transport.setTimes (0.9f, 0.35f);

    const int latency = (int) std::round (transport.getBaseDelay());
    setLatencySamples (latency);

    dryWet.prepare ({ fs, (juce::uint32) samplesPerBlock, numCh });
    dryWet.setMixingRule (juce::dsp::DryWetMixingRule::linear);
    dryWet.setWetLatency ((float) latency);

    for (auto* b : { &preEmph, &deEmph, &headBump, &lowLoss, &hfLoss1, &hfLoss2, &hissShape })
        b->reset();
    hissShape.highpass (fs, 1500.0, 0.5);

    inGainSm.reset (fs, 0.03);  inGainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pInput->load()));
    outGainSm.reset (fs, 0.03); outGainSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    satSm.reset (fs, 0.05);     satSm.setCurrentAndTargetValue (pSat->load());

    dropGain = dropTarget = 1.0f;
    dropHold = 0;
    updateFilters();
}

void Reel15AudioProcessor::updateFilters()
{
    const int speed = juce::jlimit (0, 2, (int) pSpeed->load());
    const float sat  = pSat->load();
    const float wear = pWear->load();

    // Pre-/De-Emphasis: Höhen werden stärker gesättigt, wie auf echtem Band
    const float emph = sat * 9.0f;
    preEmph.highShelf (fs, 2500.0, emph);
    deEmph.highShelf  (fs, 2500.0, -emph);

    headBump.peak (fs, bumpHz[speed], 1.1, pBump->load() * 6.0);
    lowLoss.highpass (fs, bumpHz[speed] * 0.45, 0.7);

    const float toneMul = std::pow (2.0f, pTone->load() * 1.3f);
    const float cutoff  = hfHz[speed] * toneMul * (1.0f - wear * 0.55f);
    hfLoss1.lowpass (fs, cutoff, 0.6);
    hfLoss2.lowpass (fs, cutoff * 1.4, 0.5);
}

void Reel15AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), getTotalNumOutputChannels());
    const int n = buffer.getNumSamples();
    if (numCh == 0 || n == 0)
        return;

    updateFilters();

    auto block = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numCh);
    dryWet.setWetMixProportion (pMix->load());
    dryWet.pushDrySamples (block);

    inGainSm.setTargetValue  (juce::Decibels::decibelsToGain (pInput->load()));
    outGainSm.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    satSm.setTargetValue (pSat->load());

    const float wow = pWow->load(), flutter = pFlutter->load(), wear = pWear->load();
    const float hissAmp = std::pow (pHiss->load(), 2.0f) * 0.05f;
    const bool stopped = pStop->load() > 0.5f;

    // Modulationstiefen in Samples
    const float wowDepth = std::pow (wow, 1.5f) * 0.004f * (float) fs;       // bis 4 ms
    const float flDepth  = std::pow (flutter, 1.5f) * 0.00012f * (float) fs;  // bis 0,12 ms
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double wowInc = twoPi * 0.55 / fs, wowInc2 = twoPi * 0.23 / fs;
    const double flInc  = twoPi * 7.8 / fs,  flInc2  = twoPi * 13.3 / fs;
    const float driftCoef = 1.0f / (0.4f * (float) fs);
    const float dropProbPerSample = wear * wear * 2.5f / (float) fs;
    const float dropSmooth = 1.0f - std::exp (-1.0f / (0.012f * (float) fs));

    auto* L = buffer.getWritePointer (0);
    auto* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;
    float peak = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float gIn = inGainSm.getNextValue();
        const float sat = satSm.getNextValue();
        const float k = 1.0f + sat * 6.0f;
        const float bias = 0.12f * sat;
        const float tb = std::tanh (k * bias);
        const float g0 = std::sqrt (k * (1.0f - tb * tb));

        float x[2] = { L[i] * gIn, (R != nullptr ? R[i] : L[i]) * gIn };

        // --- Aufnahme: Emphasis + Sättigung ---------------------------------
        for (int ch = 0; ch < 2; ++ch)
        {
            float s = preEmph.process (x[ch], ch);
            const float satd = (std::tanh (k * (s + bias)) - tb) / g0;
            s = s + sat * (satd - s);                                  // 0 % = linear
            x[ch] = deEmph.process (s, ch);
        }

        // --- Transport: Wow (zwei langsame LFOs + Zufallsdrift) und Flutter ---
        wowPhase += wowInc;   if (wowPhase  > twoPi) wowPhase  -= twoPi;
        wowPhase2 += wowInc2; if (wowPhase2 > twoPi) wowPhase2 -= twoPi;
        flPhase += flInc;     if (flPhase   > twoPi) flPhase   -= twoPi;
        flPhase2 += flInc2;   if (flPhase2  > twoPi) flPhase2  -= twoPi;
        if (rng.nextFloat() < 0.0005f) driftTarget = rng.nextFloat() * 2.0f - 1.0f;
        drift += (driftTarget - drift) * driftCoef;

        const float mod = wowDepth * (0.5f * (1.0f + (float) std::sin (wowPhase)) * 0.7f
                                      + 0.3f * (0.5f + 0.5f * drift)
                                      + 0.25f * (float) std::sin (wowPhase2))
                        + flDepth * (1.0f + (float) std::sin (flPhase) + 0.4f * (float) std::sin (flPhase2)
                                     + 0.3f * (rng.nextFloat() - 0.5f));

        float yl, yr;
        transport.process (x[0], x[1], mod, stopped, yl, yr);

        // --- Wiedergabe: Head Bump, Bassverlust, Rauschen, Höhenverlust ------
        float y[2] = { yl, yr };
        for (int ch = 0; ch < 2; ++ch)
        {
            float s = headBump.process (y[ch], ch);
            s = lowLoss.process (s, ch);
            if (hissAmp > 0.0f)
                s += hissShape.process ((rng.nextFloat() * 2.0f - 1.0f), ch) * hissAmp;
            s = hfLoss1.process (s, ch);
            s = hfLoss2.process (s, ch);
            y[ch] = s;
        }

        // --- Dropouts (abgenutztes Band) ------------------------------------
        if (dropHold > 0)
        {
            if (--dropHold == 0) dropTarget = 1.0f;
        }
        else if (dropProbPerSample > 0.0f && rng.nextFloat() < dropProbPerSample)
        {
            dropTarget = 1.0f - wear * (0.3f + 0.6f * rng.nextFloat());
            dropHold = (int) ((0.02f + 0.08f * rng.nextFloat()) * (float) fs);
        }
        dropGain += (dropTarget - dropGain) * dropSmooth;

        const float gOut = outGainSm.getNextValue() * dropGain;
        L[i] = y[0] * gOut;
        if (R != nullptr) R[i] = y[1] * gOut;
        peak = juce::jmax (peak, std::abs (L[i]));
    }

    reelRate.store (transport.getRate());
    meterLevel.store (peak);

    dryWet.mixWetSamples (block);
}

//==============================================================================
int Reel15AudioProcessor::getNumPrograms() { return numPresets; }

const juce::String Reel15AudioProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, numPresets) ? juce::String (presets[index].name) : juce::String();
}

void Reel15AudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, numPresets))
        return;

    currentProgram = index;
    const auto& p = presets[index];
    auto set = [this] (const char* id, float v)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (v));
    };
    set ("speed", (float) p.speed); set ("input", p.input);     set ("saturate", p.sat);
    set ("bump", p.bump);           set ("tone", p.tone);       set ("hiss", p.hiss);
    set ("wow", p.wow);             set ("flutter", p.flutter); set ("wear", p.wear);
    set ("mix", p.mix);             set ("output", p.output);
}

//==============================================================================
void Reel15AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void Reel15AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, numPresets - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* Reel15AudioProcessor::createEditor()
{
    return new Reel15AudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reel15AudioProcessor();
}

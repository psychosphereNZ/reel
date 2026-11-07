#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "TapeDSP.h"

//==============================================================================
// REEL 15 – Tape-Maschine
// Signalweg:
//   Input -> Pre-Emphasis -> Bandsättigung (asymmetrisch) -> De-Emphasis
//   -> Bandtransport (Wow, Flutter, Tape-Stop) -> Head Bump + Bass-Verlust
//   -> Bandrauschen -> Höhenverlust (abhängig von Geschwindigkeit, Tone, Wear)
//   -> Dropouts -> Output -> Dry/Wet
//==============================================================================
class Reel15AudioProcessor : public juce::AudioProcessor
{
public:
    Reel15AudioProcessor();
    ~Reel15AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Für die Spulen-Animation im Editor
    float getTransportRate() const { return reelRate.load(); }
    float getLevel() const         { return meterLevel.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

private:
    void updateFilters();

    double fs = 44100.0;
    int currentProgram = 0;

    TapeTransport transport;
    Biquad preEmph, deEmph, headBump, lowLoss, hfLoss1, hfLoss2, hissShape;
    juce::dsp::DryWetMixer<float> dryWet { 4096 };

    juce::SmoothedValue<float> inGainSm, outGainSm, satSm;
    juce::Random rng;

    // Wow/Flutter
    double wowPhase = 0.0, wowPhase2 = 0.0, flPhase = 0.0, flPhase2 = 0.0;
    float drift = 0.0f, driftTarget = 0.0f;

    // Dropouts
    float dropGain = 1.0f, dropTarget = 1.0f;
    int dropHold = 0;

    std::atomic<float> reelRate { 1.0f }, meterLevel { 0.0f };

    std::atomic<float>* pSpeed = nullptr;
    std::atomic<float>* pInput = nullptr;
    std::atomic<float>* pSat = nullptr;
    std::atomic<float>* pBump = nullptr;
    std::atomic<float>* pTone = nullptr;
    std::atomic<float>* pHiss = nullptr;
    std::atomic<float>* pWow = nullptr;
    std::atomic<float>* pFlutter = nullptr;
    std::atomic<float>* pWear = nullptr;
    std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pStop = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reel15AudioProcessor)
};

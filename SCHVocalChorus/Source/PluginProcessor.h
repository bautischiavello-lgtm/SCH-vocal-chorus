#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ChoirEngine.h"

class SCHVocalChorusAudioProcessor : public juce::AudioProcessor
{
public:
    SCHVocalChorusAudioProcessor();
    ~SCHVocalChorusAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SCH Vocal Chorus"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.2; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    sch::ChoirEngine engine;

    std::atomic<float>* pMix    = nullptr;
    std::atomic<float>* pDetune = nullptr;
    std::atomic<float>* pDelay  = nullptr;
    std::atomic<float>* pWidth  = nullptr;
    std::atomic<float>* pMode   = nullptr;
    std::atomic<float>* pMono   = nullptr;
    std::atomic<float>* pAmount = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SCHVocalChorusAudioProcessor)
};

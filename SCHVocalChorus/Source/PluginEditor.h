#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PixelLookAndFeel.h"

class SCHVocalChorusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit SCHVocalChorusAudioProcessorEditor (SCHVocalChorusAudioProcessor&);
    ~SCHVocalChorusAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    SCHVocalChorusAudioProcessor& processor;
    PixelLookAndFeel laf;

    juce::Slider mixKnob, detuneKnob, delayKnob, widthKnob, amountSlider;
    juce::ComboBox modeBox;
    juce::ToggleButton monoButton { "MONO" };

    int twinkle = 0;

    // attachments declared last so they are destroyed first
    std::unique_ptr<SliderAtt> mixAtt, detuneAtt, delayAtt, widthAtt, amountAtt;
    std::unique_ptr<ComboAtt>  modeAtt;
    std::unique_ptr<ButtonAtt> monoAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SCHVocalChorusAudioProcessorEditor)
};

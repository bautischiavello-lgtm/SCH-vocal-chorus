#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
SCHVocalChorusAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "detune", 1 }, "Detune", NormalisableRange<float> (-20.0f, 20.0f, 0.1f), 12.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "delay", 1 }, "Delay", NormalisableRange<float> (0.0f, 50.0f, 0.1f), 20.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "width", 1 }, "Width", NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<AudioParameterChoice> (
        ParameterID { "mode", 1 }, "Mode",
        StringArray { "Classic", "Gospel", "Kids", "Shout", "Custom" }, 0));

    params.push_back (std::make_unique<AudioParameterBool> (
        ParameterID { "mono", 1 }, "Mono", false));

    params.push_back (std::make_unique<AudioParameterInt> (
        ParameterID { "amount", 1 }, "Amount", 1, sch::kMaxVoices, 16));

    return { params.begin(), params.end() };
}

SCHVocalChorusAudioProcessor::SCHVocalChorusAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SCH_VOCAL_CHORUS", createParameterLayout())
{
    pMix    = apvts.getRawParameterValue ("mix");
    pDetune = apvts.getRawParameterValue ("detune");
    pDelay  = apvts.getRawParameterValue ("delay");
    pWidth  = apvts.getRawParameterValue ("width");
    pMode   = apvts.getRawParameterValue ("mode");
    pMono   = apvts.getRawParameterValue ("mono");
    pAmount = apvts.getRawParameterValue ("amount");
}

bool SCHVocalChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    // mono->mono, mono->stereo, stereo->stereo
    if (in == juce::AudioChannelSet::mono())   return true;
    if (in == juce::AudioChannelSet::stereo()) return out == juce::AudioChannelSet::stereo();
    return false;
}

void SCHVocalChorusAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // ALL memory (ring buffer, voices, filter states) is allocated here.
    engine.prepare (sampleRate);
}

void SCHVocalChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int n      = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, n);

    // mono in -> stereo out: duplicate the input into the second channel
    if (numIn == 1 && numOut >= 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);

    sch::Params p;
    p.mix         = pMix->load();
    p.detuneCents = pDetune->load();
    p.delayMs     = pDelay->load();
    p.width       = pWidth->load();
    p.mode        = (sch::ChoirMode) juce::jlimit (0, 4, (int) std::lround (pMode->load()));
    p.mono        = pMono->load() > 0.5f;
    p.voices      = juce::jlimit (1, sch::kMaxVoices, (int) std::lround (pAmount->load()));

    float* L = buffer.getWritePointer (0);
    float* R = numOut >= 2 ? buffer.getWritePointer (1) : nullptr;

    engine.process (L, R, n, p);
}

juce::AudioProcessorEditor* SCHVocalChorusAudioProcessor::createEditor()
{
    return new SCHVocalChorusAudioProcessorEditor (*this);
}

void SCHVocalChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SCHVocalChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SCHVocalChorusAudioProcessor();
}

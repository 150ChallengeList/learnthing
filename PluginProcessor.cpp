/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LearnthingAudioProcessor::LearnthingAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

LearnthingAudioProcessor::~LearnthingAudioProcessor()
{
}

//==============================================================================
const juce::String LearnthingAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LearnthingAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LearnthingAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LearnthingAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LearnthingAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LearnthingAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LearnthingAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LearnthingAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LearnthingAudioProcessor::getProgramName (int index)
{
    return {};
}

void LearnthingAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LearnthingAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dspFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 100.0f, 0.5f, 3.0f);


    // Prepare the filter for processing
    juce::dsp::ProcessSpec ProcessSpecThing;
    ProcessSpecThing.sampleRate = sampleRate;
    ProcessSpecThing.maximumBlockSize = samplesPerBlock;
    ProcessSpecThing.numChannels = getTotalNumOutputChannels();

    dspFilter.prepare(ProcessSpecThing);
}

void LearnthingAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LearnthingAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LearnthingAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    dspFilter.process(context);

    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
    }
}

//==============================================================================
bool LearnthingAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LearnthingAudioProcessor::createEditor()
{
    return new LearnthingAudioProcessorEditor (*this);
}

//==============================================================================
void LearnthingAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LearnthingAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LearnthingAudioProcessor();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NMAudioProcessor::NMAudioProcessor()
    : gainParameter(new juce::AudioParameterFloat("gain", "Gain", -12.0f, 12.0f, 0.0f))
{
    addParameter(gainParameter);
}

NMAudioProcessor::~NMAudioProcessor()
{
}

//==============================================================================
const juce::String NMAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NMAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NMAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NMAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NMAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NMAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NMAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NMAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NMAudioProcessor::getProgramName (int index)
{
    return {};
}

void NMAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NMAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NMAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NMAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NMAudioProcessor::applyLimiter(juce::AudioBuffer<float>& buffer)
{
    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();

    float thresholdDb = -6.0f;
    float ceilingDb = 0.0f;
    float linearThreshold = juce::Decibels::decibelsToGain(thresholdDb);
    float linearCeiling = juce::Decibels::decibelsToGain(ceilingDb);
    float sustainLevelDb = 1.0f;

    float attackCoeff = std::exp(-1.0f / (attackTimeMs * getSampleRate() * 0.001f));
    float releaseCoeff = std::exp(-1.0f / (releaseTimeMs * getSampleRate() * 0.001f));
    float sustainLevel = juce::Decibels::decibelsToGain(sustainLevelDb);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        float gainReduction = 1.0f;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputSample = channelData[sample];
            float inputLevel = std::fabs(inputSample);

            if (inputLevel > linearThreshold)
            {
                float targetReduction = std::max(linearThreshold / inputLevel, linearCeiling);
                gainReduction = (inputLevel > linearThreshold)
                    ? std::max(targetReduction, gainReduction * attackCoeff)
                    : gainReduction * releaseCoeff;

                channelData[sample] = inputSample * juce::jlimit(linearCeiling, 1.0f, gainReduction);
            }
            else
            {
                gainReduction = sustainLevel + (gainReduction - sustainLevel) * releaseCoeff;
                channelData[sample] = inputSample * juce::jlimit(linearCeiling, 1.0f, gainReduction);
            }
        }
    }
}

void NMAudioProcessor::applyGainToBuffer(juce::AudioBuffer<float>& buffer)
{
    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();
    auto dB = gainParameter->get();
    auto gain = dBToLinear(dB);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            channelData[sample] *= gain;
    }
}

float NMAudioProcessor::dBToLinear(float dB)
{
    return juce::Decibels::decibelsToGain(dB);
}

void NMAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    applyLimiter(buffer);
    applyGainToBuffer(buffer);
}

//==============================================================================
bool NMAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NMAudioProcessor::createEditor()
{
    return new NMAudioProcessorEditor (*this);
}

//==============================================================================
void NMAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NMAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NMAudioProcessor();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhatBassAudioProcessor::PhatBassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), parameters(*this, nullptr, "PARAMETERS",
        { std::make_unique<juce::AudioParameterFloat>("gain", "Gain", -60.0f, 24.0f, 0.0f) })
#endif
{
}

PhatBassAudioProcessor::~PhatBassAudioProcessor()
{
}

//==============================================================================
const juce::String PhatBassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhatBassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhatBassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhatBassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhatBassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhatBassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhatBassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhatBassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhatBassAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhatBassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhatBassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);

    lowPassFilter.setCutoffFrequency(120.0f);
    highPassFilter.setCutoffFrequency(2000.0f);

    lowBandCompressor.prepare(spec);
    midBandCompressor.prepare(spec);
    highBandCompressor.prepare(spec);

    lowBandCompressor.setThreshold(juce::Decibels::decibelsToGain(-35.0f));
    lowBandCompressor.setRatio(4.0f);
    lowBandCompressor.setAttack(0.51f);
    lowBandCompressor.setRelease(0.69f);

    midBandCompressor.setThreshold(juce::Decibels::decibelsToGain(-35.0f));
    midBandCompressor.setRatio(4.0f);
    midBandCompressor.setAttack(0.40f);
    midBandCompressor.setRelease(0.69f);

    highBandCompressor.setThreshold(juce::Decibels::decibelsToGain(-35.0f));
    highBandCompressor.setRatio(4.0f);
    highBandCompressor.setAttack(0.33f);
    highBandCompressor.setRelease(0.57f);

    this->sampleRate = sampleRate;
}

void PhatBassAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhatBassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

float PhatBassAudioProcessor::calculateEnvelope(float input)
{
    float inputLevel = fabsf(input);
    if (inputLevel > envelope)
        envelope = attack(inputLevel);
    else
        envelope = release(inputLevel);
    return envelope;
}

float PhatBassAudioProcessor::attack(float input)
{
    float attackCoeff = expf(-1.0f / (attackTime * sampleRate));
    return attackCoeff * envelope + (1.0f - attackCoeff) * input;
}

float PhatBassAudioProcessor::release(float input)
{
    float releaseCoeff = expf(-1.0f / (releaseTime * sampleRate));
    return releaseCoeff * envelope + (1.0f - releaseCoeff) * input;
}

float PhatBassAudioProcessor::applySustain(float input)
{
    return sustainLevel * input;
}

float PhatBassAudioProcessor::calculateGainReduction(float envelope)
{
    gainReduction = 1.0f / (1.0f + envelope);
    return gainReduction;
}

float PhatBassAudioProcessor::processSample(float inputSample)
{
    float env = calculateEnvelope(inputSample);
    env = applySustain(env);
    float gain = calculateGainReduction(env);
    return gain * inputSample;
}

// Gain processing function
float PhatBassAudioProcessor::applyGain(float inputSample)
{
    float gainFactor = juce::Decibels::decibelsToGain(gainInDecibels);

    return inputSample * gainFactor;
}

void PhatBassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    {
        juce::dsp::AudioBlock<float> lowBlock(buffer);
        lowPassFilter.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
        lowBandCompressor.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
    }

    {
        juce::dsp::AudioBlock<float> highBlock(buffer);
        highPassFilter.process(juce::dsp::ProcessContextReplacing<float>(highBlock));
        highBandCompressor.process(juce::dsp::ProcessContextReplacing<float>(highBlock));
    }

    {
        juce::dsp::AudioBlock<float> midBlock(buffer);
        midBandCompressor.process(juce::dsp::ProcessContextReplacing<float>(midBlock));
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        buffer.applyGain(channel, 0, buffer.getNumSamples(), 0.18f);
        buffer.applyGain(channel, 0, buffer.getNumSamples(), 0.59f);
        buffer.applyGain(channel, 0, buffer.getNumSamples(), 0.70f);

        auto* channelData = buffer.getWritePointer(channel);

        buffer.addFrom(channel, 0, lowBuffer, channel, 0, buffer.getNumSamples(), 1.00f);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = processSample(channelData[sample]);

            gainInDecibels = *parameters.getRawParameterValue("gain");

            channelData[sample] = applyGain(channelData[sample]);
        }
    }
}

//==============================================================================
bool PhatBassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhatBassAudioProcessor::createEditor()
{
    return new PhatBassAudioProcessorEditor (*this);
}

//==============================================================================
void PhatBassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PhatBassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhatBassAudioProcessor();
}

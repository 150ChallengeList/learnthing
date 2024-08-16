/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PhatBassAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PhatBassAudioProcessor();
    ~PhatBassAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    // Limiter parameters
    float attackTime = 0.11f;
    float releaseTime = 0.5f;
    float sustainLevel = 0.28f;

    float sampleRate = 44100.0;

    float envelope = 0.0f;
    float gainReduction = 1.0f;

    float gainInDecibels = 0.0f;

    float calculateEnvelope(float input);
    float attack(float input);
    float release(float input);
    float applySustain(float input);
    float calculateGainReduction(float envelope);
    float processSample(float inputSample);
    float applyGain(float inputSample);

    juce::dsp::LinkwitzRileyFilter<float> lowPassFilter;
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;

    juce::dsp::Compressor<float> lowBandCompressor;
    juce::dsp::Compressor<float> midBandCompressor;
    juce::dsp::Compressor<float> highBandCompressor;

    juce::AudioBuffer<float> lowBuffer, midBuffer, highBuffer;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhatBassAudioProcessor)
};

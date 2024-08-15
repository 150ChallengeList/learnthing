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
class NMAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NMAudioProcessor();
    ~NMAudioProcessor() override;

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

    juce::AudioParameterFloat* gainParameter;

private:
    juce::dsp::Gain<float> outputGain;
    juce::dsp::Compressor<float> limiter;
    float dBToLinear(float dB);

    float attackTimeMs = 11.0f;   // Attack time in milliseconds
    float releaseTimeMs = 50.0f;  // Release time in milliseconds
    float sustainLevel = 0.29f;   // Sustain level as a percentage (interpreted as 29% gain reduction)

    void applyLimiter(juce::AudioBuffer<float>& buffer);
    void applyGainToBuffer(juce::AudioBuffer<float>& buffer);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NMAudioProcessor)
};

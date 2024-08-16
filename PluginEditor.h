/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PhatBassAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PhatBassAudioProcessorEditor (PhatBassAudioProcessor&);
    ~PhatBassAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PhatBassAudioProcessor& audioProcessor;

    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhatBassAudioProcessorEditor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NMAudioProcessorEditor::NMAudioProcessorEditor (NMAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    enableFeatureButton.setButtonText("Bypass");
    enableFeatureButton.setToggleState(false, juce::dontSendNotification);

    // Add the ToggleButton to the editor
    addAndMakeVisible(enableFeatureButton);

    // Handle button click
    enableFeatureButton.onClick = [this]()
    {
        if (enableFeatureButton.getToggleState())
            DBG("Feature Enabled");
        else
            DBG("Feature Disabled");
    };

    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(-12.0, 12.0, 0.01);
    gainSlider.setValue(0.0);
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true);
    addAndMakeVisible(gainLabel);

    setSize (500, 450);
}

NMAudioProcessorEditor::~NMAudioProcessorEditor()
{
}

//==============================================================================
void NMAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NMAudioProcessorEditor::resized()
{
    enableFeatureButton.setBounds(20, 20, 150, 30);
    gainSlider.setBounds(40, 60, getWidth() - 80, 20);
}

void NMAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        audioProcessor.gainParameter->setValueNotifyingHost((float)slider->getValue());
    }
}

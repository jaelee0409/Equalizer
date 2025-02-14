/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomRotarySlider : public juce::Slider {
    public:
        CustomRotarySlider() : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {
    
        }
};

//==============================================================================
/**
*/
class EqualizerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EqualizerAudioProcessorEditor (EqualizerAudioProcessor&);
    ~EqualizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EqualizerAudioProcessor& audioProcessor;

    CustomRotarySlider peak1FrequencySlider;
    CustomRotarySlider peak1GainSlider;
    CustomRotarySlider peak1QualitySlider;
    CustomRotarySlider peak2FrequencySlider;
    CustomRotarySlider peak2GainSlider;
    CustomRotarySlider peak2QualitySlider;
    CustomRotarySlider lowCutFrequencySlider;
    CustomRotarySlider highCutFrequencySlider;

    juce::AudioProcessorValueTreeState::SliderAttachment peak1FrequencySliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment peak1GainSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment peak1QualitySliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment peak2FrequencySliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment peak2GainSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment peak2QualitySliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment lowCutFrequencySliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment highCutFrequencySliderAttachment;

    juce::Label peak1FrequencyLabel;
    juce::Label peak1GainLabel;
    juce::Label peak1QualityLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessorEditor)
};

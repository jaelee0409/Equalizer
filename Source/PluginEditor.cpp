/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EqualizerAudioProcessorEditor::EqualizerAudioProcessorEditor(EqualizerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    peak1FrequencySliderAttachment(audioProcessor.apvts, "Peak1Freq", peak1FrequencySlider),
    peak1GainSliderAttachment(audioProcessor.apvts, "Peak1Gain", peak1GainSlider),
    peak1QualitySliderAttachment(audioProcessor.apvts, "Peak1Q", peak1QualitySlider),
    peak2FrequencySliderAttachment(audioProcessor.apvts, "Peak2Freq", peak2FrequencySlider),
    peak2GainSliderAttachment(audioProcessor.apvts, "Peak2Gain", peak2GainSlider),
    peak2QualitySliderAttachment(audioProcessor.apvts, "Peak2Q", peak2QualitySlider),
    lowCutFrequencySliderAttachment(audioProcessor.apvts, "LowCutFreq", lowCutFrequencySlider),
    highCutFrequencySliderAttachment(audioProcessor.apvts, "HighCutFreq", highCutFrequencySlider)
{
    addAndMakeVisible(&peak1FrequencySlider);
    addAndMakeVisible(&peak1GainSlider);
    addAndMakeVisible(&peak1QualitySlider);
    addAndMakeVisible(&peak2FrequencySlider);
    addAndMakeVisible(&peak2GainSlider);
    addAndMakeVisible(&peak2QualitySlider);
    addAndMakeVisible(&lowCutFrequencySlider);
    addAndMakeVisible(&highCutFrequencySlider);

    addAndMakeVisible(peak1FrequencyLabel);
    peak1FrequencyLabel.setText("Peak 1 Frequency", juce::dontSendNotification);
    peak1FrequencyLabel.attachToComponent(&peak1FrequencySlider, false);

    addAndMakeVisible(peak1GainLabel);
    peak1GainLabel.setText("Peak 1 Gain", juce::dontSendNotification);
    peak1GainLabel.attachToComponent(&peak1GainSlider, false);

    addAndMakeVisible(peak1QualityLabel);
    peak1QualityLabel.setText("Peak 1 Quality", juce::dontSendNotification);
    peak1QualityLabel.attachToComponent(&peak1QualitySlider, false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 800);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour::fromRGBA(86, 91, 90, 255));

    //g.setColour (juce::Colours::white);
    //g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void EqualizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> topArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
    juce::Rectangle<int> middleArea = bounds.removeFromTop(bounds.getHeight() * 0.5f);
    juce::Rectangle<int> bottomArea = bounds;

    juce::Rectangle<int> lowCutArea = middleArea.removeFromLeft(middleArea.getWidth() * 0.5f);
    juce::Rectangle<int> lowCutFrequencyArea = lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.75f);
    juce::Rectangle<int> lowCutSlopeArea = lowCutArea;

    juce::Rectangle<int> highCutArea = middleArea;
    juce::Rectangle<int> highCutFrequencyArea = highCutArea.removeFromTop(highCutArea.getHeight() * 0.75f);
    juce::Rectangle<int> highCutSlopeArea = highCutArea;

    juce::Rectangle<int> peak1Area = bottomArea.removeFromLeft(bottomArea.getWidth() * 0.5f);
    juce::Rectangle<int> peak1FrequencyArea = peak1Area.removeFromTop(peak1Area.getHeight() * 0.33f);
    juce::Rectangle<int> peak1GainArea = peak1Area.removeFromTop(peak1Area.getHeight() * 0.5f);
    juce::Rectangle<int> peak1QualityArea = peak1Area;

    juce::Rectangle<int> peak2Area = bottomArea;
    juce::Rectangle<int> peak2FrequencyArea = peak2Area.removeFromTop(peak2Area.getHeight() * 0.33f);
    juce::Rectangle<int> peak2GainArea = peak2Area.removeFromTop(peak2Area.getHeight() * 0.5f);
    juce::Rectangle<int> peak2QualityArea = peak2Area;

    lowCutFrequencySlider.setBounds(lowCutFrequencyArea);
    highCutFrequencySlider.setBounds(highCutFrequencyArea);

    peak1FrequencySlider.setBounds(peak1FrequencyArea);
    peak1GainSlider.setBounds(peak1GainArea);
    peak1QualitySlider.setBounds(peak1QualityArea);
    peak2FrequencySlider.setBounds(peak2FrequencyArea);
    peak2GainSlider.setBounds(peak2GainArea);
    peak2QualitySlider.setBounds(peak2QualityArea);
}

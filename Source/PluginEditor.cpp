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
    highCutFrequencySliderAttachment(audioProcessor.apvts, "HighCutFreq", highCutFrequencySlider),
    parametersChanged(false)
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

	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
		param->addListener(this);

	startTimerHz(60);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(1200, 1000);
}

EqualizerAudioProcessorEditor::~EqualizerAudioProcessorEditor()
{
	 auto& params = audioProcessor.getParameters();
	for (auto param : params) {
		param->removeListener(this);
	}
}

//==============================================================================
void EqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour::fromRGBA(86, 91, 90, 255));

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> topArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
    juce::Rectangle<int> responseArea = topArea.removeFromRight(topArea.getWidth() * 0.9f);

    int width = responseArea.getWidth();
    
    IIRFilter& peak1 = channelEQ.get<ChainPositions::PeakBand1>();
    IIRFilter& peak2 = channelEQ.get<ChainPositions::PeakBand2>();
    BandFilter& lowCut = channelEQ.get<ChainPositions::LowCut>();
    BandFilter& highCut = channelEQ.get<ChainPositions::HighCut>();

    double sampleRate = audioProcessor.getSampleRate();
    std::vector<double> magnitudes;
    magnitudes.resize(width);
    for (int i = 0; i < width; ++i) {
        double magnitude = 1.0;
        double frequency = juce::mapToLog10((double)i / (double)width, 20.0, 20000.0);

        if (!channelEQ.isBypassed<ChainPositions::PeakBand1>()) {
            magnitude *= peak1.coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        if (!channelEQ.isBypassed<ChainPositions::PeakBand2>()) {
            magnitude *= peak2.coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        if (!lowCut.isBypassed<0>()) {
            magnitude *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!lowCut.isBypassed<1>()) {
            magnitude *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!lowCut.isBypassed<2>()) {
            magnitude *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!lowCut.isBypassed<3>()) {
            magnitude *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        if (!highCut.isBypassed<0>()) {
            magnitude *= highCut.get<0>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!highCut.isBypassed<1>()) {
            magnitude *= highCut.get<1>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!highCut.isBypassed<2>()) {
            magnitude *= highCut.get<2>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
        if (!highCut.isBypassed<3>()) {
            magnitude *= highCut.get<3>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        magnitudes[i] = juce::Decibels::gainToDecibels(magnitude);
    }

    juce::Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input) {
        return juce::jmap<double>(input, -18.0, 18.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));
    for (int i = 0; i < magnitudes.size(); ++i) {
        responseCurve.lineTo(responseArea.getX() + i, map(magnitudes[i]));
    }

    g.setColour(juce::Colours::black);
    g.fillRect(responseArea);

    g.setColour(juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(1.f));
}

void EqualizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> topArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);
    juce::Rectangle<int> responseArea = topArea.removeFromRight(topArea.getWidth() * 0.33f);
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

void EqualizerAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);

}

void EqualizerAudioProcessorEditor::timerCallback() {
    if (parametersChanged.compareAndSetBool(false, true)) {
		const ChainSettings& chainSettings = getChainSettings(audioProcessor.apvts);
		//channelEQ.setBypassed<ChainPositions::LowCut>(audioProcessor.apvts.getRawParameterValue("LowCutBypass")->load() > 0.5);
		//channelEQ.setBypassed<ChainPositions::HighCut>(audioProcessor.apvts.getRawParameterValue("HighCutBypass")->load() > 0.5);
		//channelEQ.setBypassed<ChainPositions::PeakBand1>(audioProcessor.apvts.getRawParameterValue("Peak1Bypass")->load() > 0.5);
		//channelEQ.setBypassed<ChainPositions::PeakBand2>(audioProcessor.apvts.getRawParameterValue("Peak2Bypass")->load() > 0.5);
		auto [peak1Coefficients, peak2Coefficients] = makePeakFilters(chainSettings, audioProcessor.getSampleRate());
        channelEQ.get<ChainPositions::PeakBand1>().coefficients = *peak1Coefficients;
        channelEQ.get<ChainPositions::PeakBand2>().coefficients = *peak2Coefficients;

		//auto lowCutCoefficients = makeLowCutFilter(audioProcessor.apvts);
		//auto highCutCoefficients = makeHighCutFilter(audioProcessor.apvts);
		//updateLowCutFilter(channelEQ.get<ChainPositions::LowCut>(), lowCutCoefficients);
		//updateHighCutFilter(channelEQ.get<ChainPositions::HighCut>(), highCutCoefficients);
		repaint();
    }
}
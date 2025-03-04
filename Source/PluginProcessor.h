/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
using IIRFilter = juce::dsp::IIR::Filter<float>;
using BandFilter = juce::dsp::ProcessorChain<IIRFilter, IIRFilter, IIRFilter, IIRFilter>;
using ChannelEQ = juce::dsp::ProcessorChain<BandFilter, IIRFilter, IIRFilter, BandFilter>;

enum ChainPositions {
    LowCut,
    PeakBand1,
    PeakBand2,
    HighCut
};

enum Slope {
    Slope_12dB = 0,
    Slope_24dB,
    Slope_36dB,
    Slope_48dB
};

struct ChainSettings {
    float peak1Frequency{ 500 }, peak1GainInDecibels{ 0 }, peak1Quality{ 1.0f };
    float peak2Frequency{ 2000 }, peak2GainInDecibels{ 0 }, peak2Quality{ 1.0f };
    float lowCutFrequency{ 80 }, highCutFrequency{ 12000 };
    Slope lowCutSlope{ Slope_12dB }, highCutSlope{ Slope_12dB };
    float outputGain{ 0 };
    bool bypass{ false };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

std::pair<juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>,
    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>>
    makePeakFilters(const ChainSettings& chainSettings, double sampleRate);

void applyCutFilterCoefficients(BandFilter& filterChain, const juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coefficients, Slope slope);

template <int Stage>
void updateCutFilterStage(BandFilter& filterChain, const typename IIRFilter::CoefficientsPtr& coefficients);

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
        chainSettings.lowCutFrequency, sampleRate, 2 * (chainSettings.lowCutSlope + 1));
};

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
        chainSettings.highCutFrequency, sampleRate, 2 * (chainSettings.highCutSlope + 1));
};

//==============================================================================
/**
*/
class EqualizerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EqualizerAudioProcessor();
    ~EqualizerAudioProcessor() override;

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

    

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    
    
    void updateFilters();
    void updatePeakFilters(const ChainSettings& chainSettings);
    void updateCutFilters(const ChainSettings& chainSettings);

    void resetCutFilterBypass(BandFilter& filterChain);

    ChannelEQ leftEQ, rightEQ;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerAudioProcessor)
};

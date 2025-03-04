/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;

    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak1Freq"))
        settings.peak1Frequency = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak1Gain"))
        settings.peak1GainInDecibels = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak1Q"))
        settings.peak1Quality = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak2Freq"))
        settings.peak2Frequency = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak2Gain"))
        settings.peak2GainInDecibels = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Peak2Q"))
        settings.peak2Quality = param->load();

    if (std::atomic<float>* param = apvts.getRawParameterValue("LowCutFreq"))
        settings.lowCutFrequency = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("LowCutSlope"))
        settings.lowCutSlope = static_cast<Slope>(param->load());
    if (std::atomic<float>* param = apvts.getRawParameterValue("HighCutFreq"))
        settings.highCutFrequency = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("HighCutSlope"))
        settings.highCutSlope = static_cast<Slope>(param->load());

    if (std::atomic<float>* param = apvts.getRawParameterValue("OutputGain"))
        settings.outputGain = param->load();
    if (std::atomic<float>* param = apvts.getRawParameterValue("Bypass"))
        settings.bypass = static_cast<bool>(param->load());

    return settings;
}

//==============================================================================
EqualizerAudioProcessor::EqualizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{

}

EqualizerAudioProcessor::~EqualizerAudioProcessor()
{
}

//==============================================================================
const juce::String EqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EqualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EqualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EqualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EqualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EqualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EqualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EqualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftEQ.prepare(spec);
    rightEQ.prepare(spec);

    updateFilters();
}

void EqualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    const juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    updateFilters();

    const juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::AudioBlock<float> leftChannelBlock = audioBlock.getSingleChannelBlock(0);
    juce::dsp::AudioBlock<float> rightChannelBlock = audioBlock.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftChannelContext(leftChannelBlock);
    juce::dsp::ProcessContextReplacing<float> rightChannelContext(rightChannelBlock);

    leftEQ.process(leftChannelContext);
    rightEQ.process(rightChannelContext);
}

//==============================================================================
bool EqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EqualizerAudioProcessor::createEditor()
{
    return new EqualizerAudioProcessorEditor(*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EqualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void EqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
        updateFilters();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout EqualizerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Peak Filters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak1Freq", "Peak 1 Frequency",
        juce::NormalisableRange<float>(500.0f, 5000.0f, 0.1f, 0.5f), 500.0f,
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak1Gain", "Peak 1 Gain",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f, 1.0f), 0.0f,
        "dB",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak1Q", "Peak 1 Quality",
        juce::NormalisableRange<float>(0.5f, 5.0f, 0.05f, 1.0f), 0.5f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak2Freq", "Peak 2 Frequency",
        juce::NormalisableRange<float>(5000.0f, 10000.0f, 0.1f, 0.5f), 5000.0f,
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak2Gain", "Peak 2 Gain",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f, 1.0f), 0.0f,
        "dB",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak2Q", "Peak 2 Quality",
        juce::NormalisableRange<float>(0.5f, 5.0f, 0.05f, 1.0f), 0.5f
    ));

    // Low Cut (High-Pass Filter)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCutFreq",
        "Low Cut Frequency",
        juce::NormalisableRange<float>(20.0f, 500.0f, 0.1f, 0.5f), 20.0f,
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "LowCutSlope", "Low Cut Slope",
        juce::StringArray({ "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" }), 0
    ));

    // High Cut (Low-Pass Filter)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCutFreq", "High Cut Frequency",
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 0.1f, 0.5f), 20000.0f,
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "HighCutSlope", "High Cut Slope",
        juce::StringArray({ "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" }), 0
    ));

    // Output Gain
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "OutputGain", "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 0.0f,
        "dB",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; }
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        "Bypass", "Bypass", false
    ));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "FilterType", "Filter Type",
        juce::StringArray({ "Low Shelf", "High Shelf", "Bell", "Notch" }), 2
    ));

    return layout;
}

void EqualizerAudioProcessor::updateFilters() {
    const ChainSettings chainSettings = getChainSettings(apvts);

    updatePeakFilters(chainSettings);
    updateCutFilters(chainSettings);
}

std::pair<juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>,
          juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>>
    makePeakFilters(const ChainSettings& chainSettings, double sampleRate) {

    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>> peak1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak1Frequency,
        chainSettings.peak1Quality, juce::Decibels::decibelsToGain(chainSettings.peak1GainInDecibels));

    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>> peak2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peak2Frequency,
        chainSettings.peak2Quality, juce::Decibels::decibelsToGain(chainSettings.peak2GainInDecibels));

	return { peak1Coefficients, peak2Coefficients };
}

void EqualizerAudioProcessor::updatePeakFilters(const ChainSettings& chainSettings) {
    auto [peak1Coefficients, peak2Coefficients] = makePeakFilters(chainSettings, getSampleRate());

    *leftEQ.get<ChainPositions::PeakBand1>().coefficients = *peak1Coefficients;
    *rightEQ.get<ChainPositions::PeakBand1>().coefficients = *peak1Coefficients;

    *leftEQ.get<ChainPositions::PeakBand2>().coefficients = *peak2Coefficients;
    *rightEQ.get<ChainPositions::PeakBand2>().coefficients = *peak2Coefficients;
}

void EqualizerAudioProcessor::updateCutFilters(const ChainSettings& chainSettings) {
    const double sampleRate = getSampleRate();

	const juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> lowCutCoefficients = makeLowCutFilter(chainSettings, sampleRate);
    resetCutFilterBypass(leftEQ.get<ChainPositions::LowCut>());
    resetCutFilterBypass(rightEQ.get<ChainPositions::LowCut>());
    applyCutFilterCoefficients(leftEQ.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    applyCutFilterCoefficients(rightEQ.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);

    const juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> highCutCoefficients = makeHighCutFilter(chainSettings, sampleRate);
    resetCutFilterBypass(leftEQ.get<ChainPositions::HighCut>());
    resetCutFilterBypass(rightEQ.get<ChainPositions::HighCut>());
    applyCutFilterCoefficients(leftEQ.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
    applyCutFilterCoefficients(rightEQ.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void EqualizerAudioProcessor::resetCutFilterBypass(BandFilter& filterChain) {
    filterChain.template setBypassed<0>(true);
    filterChain.template setBypassed<1>(true);
    filterChain.template setBypassed<2>(true);
    filterChain.template setBypassed<3>(true);
}

void applyCutFilterCoefficients(BandFilter& filterChain, const juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>& coefficients, Slope slope) {
    switch (slope) {
        case Slope_48dB:
            updateCutFilterStage<3>(filterChain, coefficients[3]);
        case Slope_36dB:
            updateCutFilterStage<2>(filterChain, coefficients[2]);
        case Slope_24dB:
            updateCutFilterStage<1>(filterChain, coefficients[1]);
        case Slope_12dB:
            updateCutFilterStage<0>(filterChain, coefficients[0]);
            break;
        default:
            jassertfalse; // Invalid slope value
            break;
    }
}

template <int Stage>
void updateCutFilterStage(BandFilter& filterChain, const typename IIRFilter::CoefficientsPtr& coefficients) {
    filterChain.template get<Stage>().coefficients = coefficients;
    filterChain.template setBypassed<Stage>(false);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EqualizerAudioProcessor();
}

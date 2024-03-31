/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = exp(-1000.0f / (attackTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));
}

float EnvelopeFollower::process(float in)
{
	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
	return m_OutLast = m_AttackCoef * (m_OutLast - m_Out1Last) + m_Out1Last;
}

//==============================================================================

void BiquadLowPassFilter::set(float frequency, float Q)
{
	float frequencyLimited = fminf(frequency, 0.5f * (float)m_SampleRate);
	float norm;
	float K = tan(3.141593f * frequencyLimited / m_SampleRate);

	norm = 1 / (1 + K / Q + K * K);
	a0 = K * K * norm;
	a1 = 2 * a0;
	a2 = a0;
	b1 = 2 * (K * K - 1) * norm;
	b2 = (1 - K / Q + K * K) * norm;
}

float BiquadLowPassFilter::process(float in)
{
	float out = in * a0 + z1;
	z1 = in * a1 + z2 - b1 * out;
	z2 = in * a2 - b2 * out;
	return out;
}
//==============================================================================

const std::string DistortionAudioProcessor::paramsNames[] = { "Drive", "Dynamics", "Cutoff", "Resonance", "Mix", "Volume" };

//==============================================================================
DistortionAudioProcessor::DistortionAudioProcessor()
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
{
	driveParameter  = apvts.getRawParameterValue(paramsNames[0]);
	dynamicsParameter  = apvts.getRawParameterValue(paramsNames[1]);
	frequencyParameter = apvts.getRawParameterValue(paramsNames[2]);
	resonanceParameter = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter    = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[5]);;
}

DistortionAudioProcessor::~DistortionAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	m_lowPassFilter[0].init(sr);
	m_lowPassFilter[1].init(sr);

	m_inputEnvelope[0].init(sr);
	m_inputEnvelope[1].init(sr);

	m_outputEnvelope[0].init(sr);
	m_outputEnvelope[1].init(sr);

	static const float attack = 1.0f;
	static const float release = 10.0f;

	m_inputEnvelope[0].setCoef(attack, release);
	m_inputEnvelope[1].setCoef(attack, release);

	m_outputEnvelope[0].setCoef(attack, release);
	m_outputEnvelope[1].setCoef(attack, release);

}

void DistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto drive = driveParameter->load();
	const auto dynamics = dynamicsParameter->load();
	const auto frequency = frequencyParameter->load();
	const auto resonance = resonanceParameter->load() * 4.0f;
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	const auto mix = mixParameter->load();

	// Mics constants
	const float driveExponent = (drive >= 0.0f) ? 1.0f - (0.99f * drive) : 1.0f - 3.0f * drive;
	const float mixInverse = 1.0f - mix;
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	float gainComponsation = 0.0f;

	for (int channel = 0; channel < channels; ++channel)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& lowPassFilter = m_lowPassFilter[channel];
		auto& inputEnvelope = m_inputEnvelope[channel];
		auto& outputEnvelope = m_outputEnvelope[channel];

		// Set filter
		lowPassFilter.set(frequency, 0.707f + resonance);

		for (int sample = 0; sample < samples; ++sample)
		{
			// Get input
			const float in = channelBuffer[sample];

			// Get input loudness
			const float inputLoudness = inputEnvelope.process(in);

			// Distort
			const float sign = (in >= 0.0f) ? 1.0f : -1.0f;
			const float inDistorted = sign * powf(fabsf(in), driveExponent);

			// Low pass filter
			const float inFiltered = lowPassFilter.process(inDistorted);

			// Get output loundess
			const float outputLoudness = outputEnvelope.process(inFiltered);

			// Get gain compensation
			float gainComponesation = 1.0f;
			
			if (outputLoudness > 0.001f)
			{
				gainComponesation = inputLoudness / outputLoudness;

				if (gainComponesation < 1.0f)
				{
					const float delta = 1.0f - gainComponesation;
					gainComponesation = 1.0f - (delta * dynamics);
				}
				else
				{
					const float delta = gainComponesation - 1.0f;
					gainComponesation = 1.0f + (delta * dynamics);
				}
			}

			// Apply volume and mix
			const float inVolume = volume * mix * inFiltered * gainComponesation + mixInverse * in;

			// Clip to <-1.0, 1.0> range
			if (inVolume > 1.0f)
			{
				channelBuffer[sample] = 1.0f;
			}
			else if (inVolume < -1.0f)
			{
				channelBuffer[sample] = -1.0f;
			}
			else
			{
				channelBuffer[sample] = inVolume;
			}
		}
	}
}

//==============================================================================
bool DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionAudioProcessor::createEditor()
{
    return new DistortionAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void DistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout DistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -1.0f,     1.0f, 0.01f, 1.0f),      0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  0.0f,     1.0f, 0.01f, 1.0f),      0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 40.0f, 20000.0f,  1.0f, 0.4f), 200000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  0.0f,     1.0f, 0.01f, 1.0f),      0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  0.0f,     1.0f, 0.01f, 1.0f),      1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(-36.0f,    36.0f,  0.1f, 1.0f),      0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionAudioProcessor();
}

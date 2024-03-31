/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class EnvelopeFollower
{
public:
	EnvelopeFollower();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTime, float releaseTime);
	float process(float in);

protected:
	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;

	float m_OutLast = 0.0f;
	float m_Out1Last = 0.0f;
};

//==============================================================================
class  BiquadLowPassFilter
{
public:
	BiquadLowPassFilter() {};

	inline void init(int sampleRate) { m_SampleRate = sampleRate; }
	void set(float frequency, float Q);
	float process(float in);

private:
	int m_SampleRate = 48000;
	float a0 = 0.0f;
	float a1 = 0.0f;
	float a2 = 0.0f;
	float b1 = 0.0f;
	float b2 = 0.0f;
	float z1 = 0.0f;
	float z2 = 0.0f;
};

//==============================================================================
/**
*/
class DistortionAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    DistortionAudioProcessor();
    ~DistortionAudioProcessor() override;

	static const std::string paramsNames[];

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

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================

	std::atomic<float>* driveParameter = nullptr;
	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* autoGainReductionParameter = nullptr;

	BiquadLowPassFilter m_lowPassFilter[2] = {};
	EnvelopeFollower m_inputEnvelope[2] = {};
	EnvelopeFollower m_outputEnvelope[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessor)
};

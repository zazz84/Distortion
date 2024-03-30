/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class LowPassFilter
{
public:
	LowPassFilter() {};

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setFrequency(float frequency)
	{
		float warp = tan((frequency * 3.141593f) / m_SampleRate);
		m_OutLastCoef = (1 - warp) / (1 + warp);
		m_InCoef = warp / (1 + warp);
	}
	float process(float in)
	{
		m_OutLast = m_InCoef * (in + m_InLast) + m_OutLastCoef * m_OutLast;
		m_InLast = in;
		return m_OutLast;
	}

protected:
	int   m_SampleRate = 48000;
	float m_InCoef = 1.0f;
	float m_OutLastCoef = 0.0f;

	float m_OutLast = 0.0f;
	float m_InLast = 0.0f;
};

//==============================================================================
class LadderFilter
{
public:
	void init(int sampleRate)
	{
		m_SampleRate = sampleRate;
		m_lowPassFilter[0].init(sampleRate);
		m_lowPassFilter[1].init(sampleRate);
		m_lowPassFilter[2].init(sampleRate);
		m_lowPassFilter[3].init(sampleRate);
	}
	void setFrequency(float frequency)
	{
		m_lowPassFilter[0].setFrequency(frequency);
		m_lowPassFilter[1].setFrequency(frequency);
		m_lowPassFilter[2].setFrequency(frequency);
		m_lowPassFilter[3].setFrequency(frequency);
	}
	void setResonance(float resonance)
	{
		m_resonance = resonance;
	}
	float process(float in)
	{
		float lowPass = in - m_resonance * m_OutLast;

		lowPass = m_lowPassFilter[0].process(lowPass);
		lowPass = m_lowPassFilter[1].process(lowPass);
		lowPass = m_lowPassFilter[2].process(lowPass);
		lowPass = m_lowPassFilter[3].process(lowPass);

		m_OutLast = lowPass;
		return lowPass;
	}

protected:
	LowPassFilter m_lowPassFilter[4] = {};
	int   m_SampleRate = 48000;

	float m_OutLast = 0.0f;
	float m_resonance = 0.0f;
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
	inline float lerp(float x0, float x1, float y0, float y1, float t)
	{
		// Calculate the slope (m) of the line passing through (x0, y0) and (x1, y1)
		float slope = (y1 - y0) / (x1 - x0);

		// Compute the interpolated value (yp) at the point (xp)
		float yp = y0 + slope * (t - x0);

		return yp;
	}
	
	//==============================================================================

	std::atomic<float>* driveParameter = nullptr;
	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	LadderFilter m_lowPassFilter[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessor)
};

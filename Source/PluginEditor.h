/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class ZazzLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ZazzLookAndFeel(const float hue)
	{
		m_light = juce::Colour::fromHSV(hue, 0.5f, 0.6f, 1.0f);
		m_medium = juce::Colour::fromHSV(hue, 0.5f, 0.5f, 1.0f);
		m_dark = juce::Colour::fromHSV(hue, 0.5f, 0.4f, 1.0f);
	}
	
	juce::Colour m_light;
	juce::Colour m_medium;
	juce::Colour m_dark;

	int m_scale;
	static const int FONT_SIZE = 15;
	static const int SLIDER_FONT_SIZE = 20;
	
	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& s) override
	{
		auto radius = ((float)juce::jmin(width / 2, height / 2) - 4.0f) * 0.9f;
		auto centreX = (float)x + (float)width  * 0.5f;
		auto centreY = (float)y + (float)height * 0.5f;
		auto rx = centreX - radius;
		auto ry = centreY - radius;
		auto rw = radius * 2.0f;
		auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// outline
		const float lineThickness = 6.0f;

		g.setColour(m_medium);
		g.drawEllipse(rx, ry, rw, rw, lineThickness);

		juce::Path p;
		auto pointerLength = radius * 0.2f;
		auto pointerThickness = lineThickness;
		p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
		p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

		// pointer
		g.setColour(m_medium);
		g.fillPath(p);

		// slider style
		s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, SLIDER_FONT_SIZE);
	}

	juce::Label *createSliderTextBox(juce::Slider &) override
	{
		auto *l = new juce::Label();
		l->setJustificationType(juce::Justification::centred);
		l->setFont(juce::Font(FONT_SIZE));
		return l;
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override
	{
		auto buttonArea = button.getLocalBounds();

		g.setColour(backgroundColour);
		g.fillRect(buttonArea);
	}
};

//==============================================================================
/**
*/
class DistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor (DistortionAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~DistortionAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 6;
	static const int SLIDER_WIDTH = 140;

	static const int FONT_DIVISOR = 9;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

private:
    DistortionAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	ZazzLookAndFeel zazzLookAndFeel = { 0.6f };

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessorEditor)
};

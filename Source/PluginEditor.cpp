/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
	juce::Colour light  = juce::Colour::fromHSV(0.6f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(0.6f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark   = juce::Colour::fromHSV(0.6f, 0.5f, 0.4f, 1.0f);

	const int fonthHeight = (int)(SLIDER_WIDTH / FONT_DIVISOR);

	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(DistortionAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(fonthHeight, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setLookAndFeel(&zazzLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, DistortionAudioProcessor::paramsNames[i], slider));
	}

	// Canvas
	setResizable(true, true);
	const float width = SLIDER_WIDTH * N_SLIDERS;
	setSize(width, SLIDER_WIDTH);

	if (auto* constrainer = getConstrainer())
	{
		constrainer->setFixedAspectRatio(N_SLIDERS);
		constrainer->setSizeLimits(width * 0.7f, SLIDER_WIDTH * 0.7, width * 2.0f, SLIDER_WIDTH * 2.0f);
	}
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

//==============================================================================
void DistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(0.6f, 0.5f, 0.7f, 1.0f));
}

void DistortionAudioProcessorEditor::resized()
{
	const int width = (int)(getWidth() / N_SLIDERS);
	const int height = getHeight();
	const int fonthHeight = (int)(height / FONT_DIVISOR);
	const int labelOffset = (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5;

	// Sliders + Labels
	for (int i = 0; i < N_SLIDERS; ++i)
	{
		juce::Rectangle<int> rectangle;

		rectangle.setSize(width, height);
		rectangle.setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangle);

		rectangle.removeFromBottom(labelOffset);
		m_labels[i].setBounds(rectangle);

		m_labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
	}
}

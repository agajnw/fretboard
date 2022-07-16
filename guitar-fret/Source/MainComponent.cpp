#include "MainComponent.h"
#include "FretView.cpp"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (234 * 4, 108 * 4);

    setAudioChannels (2, 2);

    addAndMakeVisible (fretboardView);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    metronome.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    metronome.getNextAudioBlock (bufferToFill);
}

void MainComponent::releaseResources() {}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    fretboardView.setBounds (getLocalBounds());
}

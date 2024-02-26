#include "MainComponent.h"

MainComponent::MainComponent()
{
  setSize(900, 900);

  if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
  {
    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [&](bool granted) {setAudioChannels(granted ? 2 : 0, 2); });
  }
  else setAudioChannels(0, 2);

  formatManager.registerBasicFormats();

  // Main components
  addAndMakeVisible(deckGUI1);
  addAndMakeVisible(deckGUI2);
  addAndMakeVisible(playlistComponent);

  // Add-ons
  addAndMakeVisible(crossfadeSliderLabel);
  crossfadeSliderLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(crossfadeSlider);
  crossfadeSlider.addListener(this);
  
  crossfadeSlider.setRange(-100, 100, 1);
  int crossfadeDefaultValue = 0;
  crossfadeSlider.setValue(crossfadeDefaultValue);
  crossfadeSlider.setDoubleClickReturnValue(true, crossfadeDefaultValue);

  // calls timerCallback() at the bottom of this .cpp
  startTimer(1);
}

MainComponent::~MainComponent()
{
  stopTimer();
  shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
  player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
  mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
  mixerSource.addInputSource(&player1, false);
  mixerSource.addInputSource(&player2, false);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
  mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
  player1.releaseResources();
  player2.releaseResources();
  mixerSource.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
  // Background
  juce::Colour myBlack = juce::Colour::fromRGB(20, 20, 20);
  g.fillAll(myBlack);

  // ----- Slider ----- //
  // Knob
  crossfadeSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
  
  // Filled (left side)
  crossfadeSlider.setColour(juce::Slider::trackColourId, colour2);
  
  // Empty (right side)
  crossfadeSlider.setColour(juce::Slider::backgroundColourId, colour1);
}

void MainComponent::resized()
{
  double oneThirdHeight = getHeight() / static_cast<double>(3);
  double halfWidth = getWidth() / static_cast<double>(2);
  deckGUI1.setBounds(0, 0, halfWidth , oneThirdHeight * 2);
  deckGUI2.setBounds(deckGUI1.getX() + deckGUI1.getWidth(), deckGUI1.getY(), halfWidth, oneThirdHeight * 2);
  
  double cellHeight = (oneThirdHeight * 2) / 25; // Ensure this calculates the same as DeckGUI::resized()'s 'cellHeight' variable
  double textWidth = (crossfadeSliderLabel.getFont().getStringWidth(crossfadeSliderLabel.getText())) * 2;
  double crossfadeSliderWidth = getWidth() / static_cast<double>(4);
  crossfadeSliderLabel.setBounds(halfWidth - (textWidth / 2), (oneThirdHeight * 2) - (cellHeight * 1.75), textWidth, cellHeight);
  crossfadeSlider.setBounds(halfWidth - (crossfadeSliderWidth / 2), (oneThirdHeight * 2) - (cellHeight * 1.25), crossfadeSliderWidth, cellHeight);
  
  double margin = 10;
  playlistComponent.setBounds((margin * 1.5), oneThirdHeight * 2, getWidth() - (margin * 3), oneThirdHeight - (margin * 2));
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
  if (slider == &crossfadeSlider)
  {
    // Make max values = 1 for accurate setGain() adjustment
    double deck1Value = deckGUI1.volSlider.getValue() / 100;
    double deck2Value = deckGUI2.volSlider.getValue() / 100;
    double crossfadeSliderValue = slider->getValue() / 100;

    // Map gain amount
    double player1Gain = juce::jmap(crossfadeSliderValue, -1.0, 1.0, deck1Value, 0.0);
    double player2Gain = juce::jmap(crossfadeSliderValue, -1.0, 1.0, 0.0, deck2Value);

    // Set gain
    player1.setGain(player1Gain);
    player2.setGain(player2Gain);
  }
}

// Check if user is loading a file from playlistComponent to DeckGUI
void MainComponent::timerCallback()
{
  // If playlistComp's 'loadToDeck' bool variable is true, then do the following
  if (playlistComponent.loadToDeck)
  {
    // Depending on playlistComp's 'deckNumber' int variable, call corresponding deckGUI and its function
    if (playlistComponent.deckNumber == 1) deckGUI1.loadFromPlaylist(playlistComponent.fileURL);
    if (playlistComponent.deckNumber == 2) deckGUI2.loadFromPlaylist(playlistComponent.fileURL);

    // Reset bool variable
    playlistComponent.loadToDeck = false;
  }
}

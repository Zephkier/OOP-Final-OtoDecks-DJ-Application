#pragma once
#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"

class MainComponent : public juce::AudioAppComponent,
                      public juce::Slider::Listener,
                      public juce::Timer
{
public:
  MainComponent();
  ~MainComponent() override;

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  void paint(juce::Graphics& g) override;
  void resized() override;

  void sliderValueChanged(juce::Slider* slider) override;

  // Check if user is loading a file from playlistComponent to DeckGUI
  void timerCallback() override;

private:
  // Variables for MainComponent and DeckGUI to work
  juce::AudioFormatManager formatManager;
  juce::AudioThumbnailCache thumbnailCache{ 100 }; // Refers to saving '100' files in cache
  juce::MixerAudioSource mixerSource;

  // Deck 1 (left side)
  DJAudioPlayer player1{ formatManager };
  juce::Colour colour1 = juce::Colour::fromRGB(0, 120, 255);
  DeckGUI deckGUI1{ &player1, formatManager, thumbnailCache, colour1 };

  // Deck 2 (right side)
  DJAudioPlayer player2{ formatManager };
  juce::Colour colour2 = juce::Colour::fromRGB(255, 120, 0);
  DeckGUI deckGUI2{ &player2, formatManager, thumbnailCache, colour2 };

  // Crossfade slider below the deck
  juce::Label crossfadeSliderLabel{ "Crossfade", "Crossfade" };
  juce::Slider crossfadeSlider{ juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox };

  // PlaylistComponent below crossfade slider
  PlaylistComponent playlistComponent{ colour1, colour2 };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

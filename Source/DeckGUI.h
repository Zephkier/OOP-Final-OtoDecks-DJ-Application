#pragma once
#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "WaveformDisplayZoomedIn.h"
#include "customLookAndFeel.h"

class DeckGUI : public juce::Component,
                public juce::Button::Listener,
                public juce::Slider::Listener,
                public juce::Timer
{
public:
  DeckGUI(DJAudioPlayer* player,
          juce::AudioFormatManager& formatManagerToUse,
          juce::AudioThumbnailCache& cacheToUse,
          juce::Colour _incomingColour);
  ~DeckGUI() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  juce::String formatSecondsToMMSS(double seconds);
  void buttonClicked(juce::Button* button) override;
  void sliderValueChanged(juce::Slider* slider) override;

  /*
  This updates...
  1. zoomed-in waveform's position to create "scrolling" effect
  2. normal waveform's playhead (aka. the vertical line)
  3. timestamp text
  4. posSlider's value (aka. its width)
  5. loop button's functionality
  */
  void timerCallback() override;

  // Made public to be accessed in MainComponent
  juce::Slider volSlider{ juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::TextBoxAbove };
  void loadFromPlaylist(std::string fileURL);

private:
  // General variables
  juce::Colour incomingColour;
  DJAudioPlayer* player;
  CustomLookAndFeel customLookAndFeel;
  void resetValues();

  // Arrays to easily apply whatever is needed
  juce::Array<juce::Button*> buttons;
  juce::Array<juce::Slider*> sliders;
  juce::Array<juce::Label*> labels;

  // ----- Top half ----- //
  // Waveforms
  WaveformDisplay waveformDisplay;
  WaveformDisplayZoomedIn waveformDisplayZoomedIn;

  // Position (is transparent over 'waveformDisplay')
  juce::Slider posSlider{ juce::Slider::SliderStyle::LinearBar, juce::Slider::TextEntryBoxPosition::NoTextBox };
  double currentSliderPosition = 0.0;

  // ----- Bottom half ----- //
  // Row 1: title, timestamp (variables are initialised and declared here due to being used in resetValues())
  double currentTimestampInSeconds = 0;
  double totalTimestampInSeconds = 0;
  bool audioLoaded = false;
  juce::Label titleLabel;
  juce::Label timestampLabel;

  // Row 2~3: play, pause, stop
  juce::TextButton playButton{ "Play" };
  juce::TextButton pauseButton{ "Pause" };
  juce::TextButton stopButton{ "Stop" };

  // Row 4~5: backward, forward, loop (toggle)
  double skipLengthInSeconds = 5;
  juce::TextButton backwardButton{ "Backward\n(" + juce::String(skipLengthInSeconds) + "s)" };
  juce::TextButton forwardButton{ "Forward\n(" + juce::String(skipLengthInSeconds) + "s)" };
  juce::String loopStatusText = "Off";
  juce::TextButton loopButton{ "Loop\n(" + loopStatusText + ")" };

  // Row 6~7: low, mid, high labels
  juce::Label lowFilterSliderLabel{ "Low Pass", "Low Pass" };
  juce::Label midFilterSliderLabel{ "Mid Pass", "Mid Pass" };
  juce::Label highFilterSliderLabel{ "High Pass", "High Pass" };

  // Row 8~10: low, mid, high sliders
  juce::Slider lowFilterSlider{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxAbove };
  juce::Slider midFilterSlider{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxAbove };
  juce::Slider highFilterSlider{ juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::TextBoxAbove };

  // Initialised and declared here due to being used in resetValues()
  int lowFilterDefaultValue = 20'000;
  int midFilterDefaultValue = 1'000;
  int highFilterDefaultValue = 20;

  // Row 11~12: unload from deck, cue set, cue play (variables are initialised and declared here due to being used in resetValues())
  juce::TextButton unloadButton{ "Reset\nDeck" };
  int cueCounter1 = 0;
  int cueCounter2 = 0;
  double cueFromHereInSeconds1 = 0;
  double cueFromHereInSeconds2 = 0;
  juce::TextButton cueSetButton1{ "Cue\n" + juce::String(cueCounter1) };
  juce::TextButton cueSetButton2{ "Cue\n" + juce::String(cueCounter2) };
  juce::TextButton cuePlayButton1{ "Play\ncue" };
  juce::TextButton cuePlayButton2{ "Play\ncue" };

  // ----- Side ----- //
  // Volume
  juce::Label volSliderLabel{ "Volume", "Volume" };
  // 'volSlider' found above as public variable

  // Speed
  juce::Label speedSliderLabel{ "Speed", "Speed" };
  juce::Slider speedSlider{ juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::TextBoxAbove };

  // Initialised and declared here due to being used in resetValues()
  int volDefaultValue = 50;
  int speedSliderDefaultValue = 1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};

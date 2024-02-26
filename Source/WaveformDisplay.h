#pragma once
#include <JuceHeader.h>

class WaveformDisplay : public juce::Component,
                        public juce::ChangeListener
{
public:
  WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                  juce::AudioThumbnailCache& cacheToUse,
                  juce::Colour _incomingColour);
  ~WaveformDisplay() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void changeListenerCallback(juce::ChangeBroadcaster* source) override;
  void loadURL(juce::URL audioURL);

  // Set vertical line's position relative to width/entire waveform
  void setPositionRelative(double pos);

  juce::AudioThumbnail audioThumb;
  bool fileLoaded;
  double position;

private:
  juce::Colour incomingColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

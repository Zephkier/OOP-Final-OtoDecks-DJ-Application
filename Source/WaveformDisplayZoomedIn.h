#pragma once
#include "WaveformDisplay.h"

class WaveformDisplayZoomedIn : public WaveformDisplay
{
public:
  WaveformDisplayZoomedIn(juce::AudioFormatManager& formatManagerToUse,
                          juce::AudioThumbnailCache& cacheToUse,
                          juce::Colour _incomingColour);
  ~WaveformDisplayZoomedIn() override;

  void paint(juce::Graphics&) override;

  // Set waveform's start-and-end point's position relative to entire waveform
  void setPositionRelativeOfWaveform(double pos);

private:
  juce::Colour incomingColour;
};

#include <JuceHeader.h>
#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& formatManagerToUse,
                                 juce::AudioThumbnailCache& cacheToUse,
                                 juce::Colour _incomingColour)
  : audioThumb(1'000, formatManagerToUse, cacheToUse), // Refers to '1000' points in waveform
    fileLoaded(false),
    position(0),
    incomingColour(_incomingColour)
{
  audioThumb.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
}

void WaveformDisplay::paint(juce::Graphics& g)
{
  // Background
  juce::Colour myBlack = juce::Colour::fromRGB(20, 20, 20);
  g.fillAll(myBlack);
  
  if (fileLoaded)
  {
    // Static waveform
    g.setColour(incomingColour.darker());
    audioThumb.drawChannel(g, getLocalBounds(), 0, audioThumb.getTotalLength(), 0, 1.0f);
    
    // Moving vertical line
    g.setColour(juce::Colours::white);
    float lineThickness = 2.0f;
    g.fillRect((position * getWidth()) - (lineThickness / 2.0f), 0.0f, lineThickness, static_cast<float>(getHeight()));
  }
  else
  {
    // Ensure same as WaveformDisplayZoomedIn::paint()'s 'else{}' section
    g.setColour(juce::Colours::white.withAlpha(0.5f));

    float fontSize = 24.0f;
    g.setFont(fontSize);
    g.drawText("No audio loaded", getLocalBounds().withY(getLocalBounds().getY() - (fontSize / 2)), juce::Justification::centred, true);

    g.setFont(18.0f);
    g.drawText("Load a track or library into this deck!", getLocalBounds().withY(getLocalBounds().getY() + (fontSize / 2)), juce::Justification::centred, true);
  }

  // Border
  g.setColour(juce::Colours::white);
  g.drawRect(getLocalBounds(), 2);
}

void WaveformDisplay::resized()
{
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
  repaint();
}

void WaveformDisplay::loadURL(juce::URL audioURL)
{
  audioThumb.clear();
  fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
}

void WaveformDisplay::setPositionRelative(double pos)
{
  if (pos != position)
  {
    position = pos;
    repaint();
  }
}

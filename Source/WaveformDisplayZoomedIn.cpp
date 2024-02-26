#include "WaveformDisplayZoomedIn.h"

WaveformDisplayZoomedIn::WaveformDisplayZoomedIn(juce::AudioFormatManager& formatManagerToUse,
                                                 juce::AudioThumbnailCache& cacheToUse,
                                                 juce::Colour _incomingColour)
  : WaveformDisplay(formatManagerToUse, cacheToUse, _incomingColour),
    incomingColour(_incomingColour)
{
}

WaveformDisplayZoomedIn::~WaveformDisplayZoomedIn()
{
}

void WaveformDisplayZoomedIn::paint(juce::Graphics& g)
{
  // Background
  juce::Colour myBlack = juce::Colour::fromRGB(20, 20, 20);
  g.fillAll(myBlack);

  if (fileLoaded)
  {
    // Set zoom amount
    /*
    0.01 = very zoomed in
    0.50 = same as normal WaveformDisplay
    1.00 = original
    */
    double zoom = 0.05;
    
    // Set range
    double rangeStart = position - zoom;
    double rangeEnd = position + zoom;
    
    // Get range in terms of audioThumb
    double visibleStart = rangeStart * audioThumb.getTotalLength();
    double visibleEnd = rangeEnd * audioThumb.getTotalLength();
    
    // Moving waveform (via changing start-and-end points)
    g.setColour(incomingColour.brighter());
    audioThumb.drawChannel(g, getLocalBounds(), visibleStart, visibleEnd, 0, 1.0f);
    
    // Static vertical line (using 'drawRect' to get sharper line)
    g.setColour(juce::Colours::white);
    double lineThickness = 1;
    g.drawRect((getWidth() / static_cast<double>(2)) - (lineThickness / 2), 0, lineThickness, getHeight());
  }
  else
  {
    // Ensure same as WaveformDisplay::paint()'s 'else{}' section
    g.setColour(juce::Colours::white.withAlpha(0.5f));

    float fontSize = 24.0f;
    g.setFont(fontSize);
    g.drawText("No audio loaded", getLocalBounds().withY(getLocalBounds().getY() - (fontSize / 2)), juce::Justification::centred, true);

    g.setFont(18.0f);
    g.drawText("Load a track or library into this deck!", getLocalBounds().withY(getLocalBounds().getY() + (fontSize / 2)), juce::Justification::centred, true);

    // Draw vertical line (and using 'double' and 'drawRect' to get sharper line)
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    double lineThickness = 1;
    g.drawRect((getWidth() / static_cast<double>(2)) - (lineThickness / 2), 0, lineThickness, getHeight());
  }

  // Border
  g.setColour(juce::Colours::white);
  g.drawRect(getLocalBounds(), 2);
}

void WaveformDisplayZoomedIn::setPositionRelativeOfWaveform(double pos)
{
  if (pos != position)
  {
    position = pos;
    repaint();
  }
}

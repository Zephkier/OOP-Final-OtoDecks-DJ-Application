#include <JuceHeader.h>
#include "DeckGUI.h"

DeckGUI::DeckGUI(DJAudioPlayer* _player,
                 juce::AudioFormatManager& formatManagerToUse,
                 juce::AudioThumbnailCache& cacheToUse,
                 juce::Colour _incomingColour)
  : player(_player),
    waveformDisplay(formatManagerToUse, cacheToUse, _incomingColour),
    waveformDisplayZoomedIn(formatManagerToUse, cacheToUse, _incomingColour),
    incomingColour(_incomingColour)
{
  // Add variables to array
  buttons.add(&playButton);
  buttons.add(&pauseButton);
  buttons.add(&stopButton);
  buttons.add(&backwardButton);
  buttons.add(&forwardButton);
  buttons.add(&loopButton);
  buttons.add(&unloadButton);
  buttons.add(&cueSetButton1);
  buttons.add(&cuePlayButton1);
  buttons.add(&cueSetButton2);
  buttons.add(&cuePlayButton2);
  sliders.add(&posSlider);
  sliders.add(&lowFilterSlider);
  sliders.add(&midFilterSlider);
  sliders.add(&highFilterSlider);
  sliders.add(&volSlider);
  sliders.add(&speedSlider);
  labels.add(&titleLabel);
  labels.add(&timestampLabel);
  labels.add(&lowFilterSliderLabel);
  labels.add(&midFilterSliderLabel);
  labels.add(&highFilterSliderLabel);
  labels.add(&volSliderLabel);
  labels.add(&speedSliderLabel);

  // Strategically addAndMakeVisible/addListener variables from "bottom to top" layer
  addAndMakeVisible(waveformDisplay);
  addAndMakeVisible(waveformDisplayZoomedIn);

  for (auto button : buttons)
  {
    addAndMakeVisible(button);
    button->addListener(this);
  }

  for (auto slider : sliders)
  {
    addAndMakeVisible(slider);
    slider->addListener(this);
  }

  for (auto label : labels)
  {
    addAndMakeVisible(label);
    label->setJustificationType(juce::Justification::centred);
  }

  // Editing variables from "bottom to top" layer
  titleLabel.setText("No audio loaded", juce::NotificationType::dontSendNotification);
  timestampLabel.setText(juce::String(formatSecondsToMMSS(currentTimestampInSeconds)) + " / " + juce::String(formatSecondsToMMSS(totalTimestampInSeconds)), juce::NotificationType::dontSendNotification);
  titleLabel.setJustificationType(juce::Justification::left);
  timestampLabel.setJustificationType(juce::Justification::right);
  
  loopButton.setClickingTogglesState(true);
  
  posSlider.setRange(0, 1);
  
  lowFilterSlider.setRange(20, 20'000, 1);
  midFilterSlider.setRange(20, 5'000, 1);
  highFilterSlider.setRange(20, 5'000, 1);
  lowFilterSlider.setValue(lowFilterDefaultValue);
  midFilterSlider.setValue(midFilterDefaultValue);
  highFilterSlider.setValue(highFilterDefaultValue);
  lowFilterSlider.setDoubleClickReturnValue(true, lowFilterDefaultValue);
  midFilterSlider.setDoubleClickReturnValue(true, midFilterDefaultValue);
  highFilterSlider.setDoubleClickReturnValue(true, highFilterDefaultValue);
  lowFilterSlider.setTextValueSuffix("Hz");
  midFilterSlider.setTextValueSuffix("Hz");
  highFilterSlider.setTextValueSuffix("Hz");
  
  volSlider.setRange(0, 100, 1);
  speedSlider.setRange(0, 3, 0.01); // Refers to '3x' speed, which is already too fast, and any faster would be ridiculous
  volSlider.setValue(volDefaultValue);
  speedSlider.setValue(speedSliderDefaultValue);
  volSlider.setDoubleClickReturnValue(true, volDefaultValue);
  speedSlider.setDoubleClickReturnValue(true, speedSliderDefaultValue);
  volSlider.setTextValueSuffix("%");
  speedSlider.setTextValueSuffix("x");

  // calls timerCallback() at the bottom of this .cpp
  startTimer(1);
}

DeckGUI::~DeckGUI()
{
  stopTimer();
}

void DeckGUI::paint(juce::Graphics& g)
{
  // Background
  g.fillAll(incomingColour.brighter().withAlpha(0.25f));

  for (int i = 0; i < buttons.size(); ++i)
  {
    auto button = buttons[i];
    
    // ----- Font size (mainly for the pair of 'cueSet' and 'cuePlay' buttons) ----- //
    button->setLookAndFeel(&customLookAndFeel);
    
    // ----- Colour (button index 6 is 'unloadButton') ----- //
    // By default
    if (i == 6) button->setColour(juce::TextButton::buttonColourId, juce::Colours::red.withAlpha(0.75f));
    else button->setColour(juce::TextButton::buttonColourId, incomingColour.withAlpha(0.25f));

    // When hovered over
    if (button->isMouseOver())
    {
      if (i == 6) button->setColour(juce::TextButton::buttonColourId, juce::Colours::red.brighter());
      else button->setColour(juce::TextButton::buttonColourId, incomingColour);
    }

    // When toggled on
    button->setColour(juce::TextButton::buttonOnColourId, incomingColour);

    // ----- Edges ----- //
    // For buttons index 0~5...
    if (i < 6)
    {
      // Right side
      if (i % 3 == 0) button->setConnectedEdges(juce::TextButton::ConnectedOnRight);
      // Middle
      else if (i % 3 == 1) button->setConnectedEdges(juce::TextButton::ConnectedOnLeft | juce::TextButton::ConnectedOnRight);
      // Left side
      else button->setConnectedEdges(juce::TextButton::ConnectedOnLeft);
    }
    // Skip button index 6, resume for buttons index 7~10
    else if (i > 6)
    {
      // Left side (aka. 'cueSet')
      if (i % 2 == 0) button->setConnectedEdges(juce::TextButton::ConnectedOnLeft);
      // Right side (aka. 'cuePlay')
      else button->setConnectedEdges(juce::TextButton::ConnectedOnRight);
    }
  }

  // This includes posSlider that is transparent over the waveform
  for (auto slider : sliders)
  {
    // -----Knob ----- //
    slider->setColour(juce::Slider::thumbColourId, juce::Colours::white);

    // ----- Linear slider ----- //
    // Filled (bottom side)
    slider->setColour(juce::Slider::trackColourId, incomingColour);

    // Empty (top side)
    slider->setColour(juce::Slider::backgroundColourId, incomingColour.brighter().withAlpha(0.25f));

    // ----- Rotary slider ----- //
    // Filled (left side)
    slider->setColour(juce::Slider::rotarySliderFillColourId, incomingColour);

    // Empty (right side)
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, incomingColour.brighter().withAlpha(0.25f));

    // ----- posSlider (must be the top layer to prevent above code from applying to posSlider) ----- //
    if (slider == &posSlider)
    {
      slider->setColour(juce::Slider::trackColourId, juce::Colours::white.withAlpha(0.25f));
      slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    }
  }
}

void DeckGUI::resized()
{
  // ----- Top half ----- //
  double waveformDisplayHeight = getHeight() / static_cast<double>(5);
  
  // Waveforms
  waveformDisplayZoomedIn.setBounds(0, 0, getWidth(), waveformDisplayHeight);
  waveformDisplay.setBounds(0, waveformDisplayZoomedIn.getY() + waveformDisplayZoomedIn.getHeight(), getWidth(), waveformDisplayHeight);
  
  // Position (is transparent over 'waveformDisplay')
  posSlider.setBounds(0, waveformDisplayZoomedIn.getY() + waveformDisplayZoomedIn.getHeight(), getWidth(), waveformDisplayHeight);

  // ----- Bottom half ----- //
  double margin = 20; // Applied to left and right only
  double rowCount = 15;
  double colCount = 5;
  double cellWidth = (getWidth() / colCount) - (margin / colCount);
  double cellHeight = (((getHeight() / static_cast<double>(5)) * 3) / rowCount) * 2; // Ensure this calculates the same as MainComponent::resized()'s 'cellHeight' variable

  // Row 1: title, timestamp
  double y1 = waveformDisplay.getY() + waveformDisplay.getHeight();
  double titleWidth = (getWidth() / static_cast<double>(5)) * 3;
  titleLabel.setBounds(margin, y1, titleWidth - margin, cellHeight);
  timestampLabel.setBounds(titleLabel.getX() + titleLabel.getWidth(), y1, getWidth() - titleWidth - margin, cellHeight);

  // Row 2~3: play, pause, stop
  double y2 = titleLabel.getY() + titleLabel.getHeight();
  playButton.setBounds(margin, y2, cellWidth, cellHeight);
  pauseButton.setBounds(playButton.getX() + playButton.getWidth(), y2, cellWidth, cellHeight);
  stopButton.setBounds(pauseButton.getX() + pauseButton.getWidth(), y2, cellWidth, cellHeight);
  
  // Side
  volSliderLabel.setBounds(stopButton.getX() + stopButton.getWidth(), y2, cellWidth - (margin / 2), cellHeight / 2);
  speedSliderLabel.setBounds(volSliderLabel.getX() + volSliderLabel.getWidth(), y2, cellWidth - (margin / 2), cellHeight / 2);
  
  // Row 4~5: backward, forward, loop (toggle)
  double y3 = playButton.getY() + playButton.getHeight();
  backwardButton.setBounds(margin, y3, cellWidth, cellHeight);
  forwardButton.setBounds(backwardButton.getX() + backwardButton.getWidth(), y3, cellWidth, cellHeight);
  loopButton.setBounds(forwardButton.getX() + forwardButton.getWidth(), y3, cellWidth, cellHeight);
  
  // Side
  volSlider.setBounds(loopButton.getX() + loopButton.getWidth(), y2 + cellHeight / 2, cellWidth - (margin / 2), cellHeight * 5);
  speedSlider.setBounds(volSlider.getX() + volSlider.getWidth(), y2 + cellHeight / 2, cellWidth - (margin / 2), cellHeight * 5);
  
  // Row 6~7: low, mid, high labels
  double y4 = backwardButton.getY() + backwardButton.getHeight();
  lowFilterSliderLabel.setBounds(margin, y4, cellWidth, cellHeight / 2);
  midFilterSliderLabel.setBounds(lowFilterSliderLabel.getX() + lowFilterSliderLabel.getWidth(), y4, cellWidth, cellHeight / 2);
  highFilterSliderLabel.setBounds(midFilterSliderLabel.getX() + midFilterSliderLabel.getWidth(), y4, cellWidth, cellHeight / 2);

  // Row 8~10: low, mid, high sliders
  double y5 = lowFilterSliderLabel.getY() + lowFilterSliderLabel.getHeight();
  lowFilterSlider.setBounds(margin, y5, cellWidth, cellHeight * 2);
  midFilterSlider.setBounds(lowFilterSlider.getX() + lowFilterSlider.getWidth(), y5, cellWidth, cellHeight * 2);
  highFilterSlider.setBounds(midFilterSlider.getX() + midFilterSlider.getWidth(), y5, cellWidth, cellHeight * 2);

  // Row 11~12: unload from deck, cue set, cue play
  double y6 = lowFilterSlider.getY() + lowFilterSlider.getHeight();
  unloadButton.setBounds(margin, y6, cellWidth, cellHeight);
  cueSetButton1.setBounds(unloadButton.getX() + unloadButton.getWidth(), y6, cellWidth / 2, cellHeight);
  cuePlayButton1.setBounds(cueSetButton1.getX() + cueSetButton1.getWidth(), y6, cellWidth / 2, cellHeight);
  cueSetButton2.setBounds(cuePlayButton1.getX() + cuePlayButton1.getWidth(), y6, cellWidth / 2, cellHeight);
  cuePlayButton2.setBounds(cueSetButton2.getX() + cueSetButton2.getWidth(), y6, cellWidth / 2, cellHeight);
}

juce::String DeckGUI::formatSecondsToMMSS(double seconds)
{
  int totalSeconds = static_cast<int>(seconds);
  int minutes = totalSeconds / 60;
  int remainingSeconds = totalSeconds % 60;
  return juce::String::formatted("%02d:%02d", minutes, remainingSeconds);
}

void DeckGUI::buttonClicked(juce::Button* button)
{
  if (button == &playButton) player->start();

  if (button == &pauseButton) player->stop();

  if (button == &stopButton)
  {
    player->stop();
    player->setPosition(0);
  }

  if (button == &backwardButton) player->setPositionRelative((player->getCurrentLengthInSeconds() - skipLengthInSeconds) / player->getTotalLengthInSeconds());

  if (button == &forwardButton) player->setPositionRelative((player->getCurrentLengthInSeconds() + skipLengthInSeconds) / player->getTotalLengthInSeconds());

  if (button == &loopButton)
  {
    // Update text (loop functionality found at timerCallback() below)
    if (loopButton.getToggleState()) loopStatusText = "On";
    else loopStatusText = "Off";

    // Set updated text
    loopButton.setButtonText("Loop\n(" + loopStatusText + ")");
  }

  if (button == &unloadButton)
  {
    // Load into DJAudioPlayer
    player->loadURL(juce::URL{ "" });
    
    // Load waveforms
    waveformDisplayZoomedIn.loadURL(juce::URL{ "" });
    waveformDisplay.loadURL(juce::URL{ "" });
    
    // Set title
    titleLabel.setText("No audio loaded", juce::NotificationType::dontSendNotification);
    
    // Update and set timestamp
    totalTimestampInSeconds = 0;
    timestampLabel.setText(juce::String(formatSecondsToMMSS(currentTimestampInSeconds)) + " / " + juce::String(formatSecondsToMMSS(totalTimestampInSeconds)), juce::NotificationType::dontSendNotification);
    audioLoaded = false;
    
    // Reset cue and slider
    resetValues();
  }

  if (button == &cueSetButton1)
  {
    // Update and set text
    cueCounter1 += 1;
    cueSetButton1.setButtonText("Cue\n" + juce::String(cueCounter1));

    // Functionality
    cueFromHereInSeconds1 = player->getCurrentLengthInSeconds();
  }

  if (button == &cuePlayButton1)
  {
    player->setPosition(cueFromHereInSeconds1);
    player->start();
  }

  if (button == &cueSetButton2)
  {
    // Update and set text
    cueCounter2 += 1;
    cueSetButton2.setButtonText("Cue\n" + juce::String(cueCounter2));

    // Functionality
    cueFromHereInSeconds2 = player->getCurrentLengthInSeconds();
  }

  if (button == &cuePlayButton2)
  {
    player->setPosition(cueFromHereInSeconds2);
    player->start();
  }
}

void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
  if (slider == &posSlider) player->setPositionRelative(slider->getValue());

  if (slider == &volSlider) player->setGain(slider->getValue() / 100);

  if (slider == &speedSlider) player->setSpeed(slider->getValue());

  if (slider == &lowFilterSlider) player->setLowFilter(slider->getValue());

  if (slider == &midFilterSlider) player->setMidFilter(slider->getValue());

  if (slider == &highFilterSlider) player->setHighFilter(slider->getValue());
}

/*
This updates...
1. zoomed-in waveform's position to create "scrolling" effect
2. normal waveform's playhead (aka. the vertical line)
3. timestamp text
4. posSlider's value (aka. its width)
5. loop button's functionality
*/
void DeckGUI::timerCallback()
{
  // Update position of waveform's start and end (to create "scrolling" effect)
  waveformDisplayZoomedIn.setPositionRelativeOfWaveform(player->getPositionRelative());
  
  // Update position of playhead (aka. vertical line)
  waveformDisplay.setPositionRelative(player->getPositionRelative());

  // Update and set timestamp text, and update and set posSlider value (aka. its width)
  if (audioLoaded)
  {
    currentTimestampInSeconds = player->getCurrentLengthInSeconds();
    timestampLabel.setText(juce::String(formatSecondsToMMSS(currentTimestampInSeconds)) + " / " + juce::String(formatSecondsToMMSS(totalTimestampInSeconds)), juce::NotificationType::dontSendNotification);
  
    currentSliderPosition = player->getPositionRelative();
    posSlider.setValue(currentSliderPosition);
  }

  // Loop button's functionality
  if (loopButton.getToggleState())
  {
    if (player->getPositionRelative() >= 1)
    {
      player->setPositionRelative(0);
      player->start();
    }
  }
}

void DeckGUI::loadFromPlaylist(std::string fileURL)
{
  // Retrieve file from fileURL
  juce::File chosenFile = fileURL;
  
  // Load into DJAudioPlayer
  player->loadURL(juce::URL{ chosenFile });

  // Load waveforms
  waveformDisplayZoomedIn.loadURL(juce::URL{ chosenFile });
  waveformDisplay.loadURL(juce::URL{ chosenFile });

  // Set title
  titleLabel.setText(chosenFile.getFileNameWithoutExtension(), juce::NotificationType::dontSendNotification);

  // Update and set timestamp
  totalTimestampInSeconds = player->getTotalLengthInSeconds();
  timestampLabel.setText(juce::String(formatSecondsToMMSS(currentTimestampInSeconds)) + " / " + juce::String(formatSecondsToMMSS(totalTimestampInSeconds)), juce::NotificationType::dontSendNotification);
  audioLoaded = true;

  // Reset cue and slider
  resetValues();
}

void DeckGUI::resetValues()
{
  // Reset cue-related items
  cueCounter1 = 0;
  cueSetButton1.setButtonText("Cue\n" + juce::String(cueCounter1));
  cueCounter2 = 0;
  cueSetButton2.setButtonText("Cue\n" + juce::String(cueCounter2));

  // Reset sliders
  player->setPositionRelative(0.0);
  player->stop();
  lowFilterSlider.setValue(lowFilterDefaultValue);
  midFilterSlider.setValue(midFilterDefaultValue);
  highFilterSlider.setValue(highFilterDefaultValue);
  volSlider.setValue(volDefaultValue);
  speedSlider.setValue(speedSliderDefaultValue);
}

#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
  juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
};

#pragma once
#include <JuceHeader.h>

class DJAudioPlayer : public juce::AudioSource
{
public:
  DJAudioPlayer(juce::AudioFormatManager& _formatManager);
  ~DJAudioPlayer();

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  void loadURL(juce::URL audioURL);
  
  void start();
  void stop();
  void setGain(double gain);
  void setSpeed(double ratio);

  void setPosition(double posInSeconds);
  void setPositionRelative(double pos);
  double getPositionRelative();
  double getCurrentLengthInSeconds();
  double getTotalLengthInSeconds();

  void setMidFilter(double freq);
  void setLowFilter(double freq);
  void setHighFilter(double freq);

private:
  juce::AudioFormatManager& formatManager;
  juce::AudioTransportSource transportSource;
  juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  
  double lastSampleRate;
  juce::IIRFilterAudioSource lowFilterSource{ &resampleSource, false };
  juce::IIRFilterAudioSource midFilterSource{ &lowFilterSource, false };
  juce::IIRFilterAudioSource highFilterSource{ &midFilterSource, false };
};

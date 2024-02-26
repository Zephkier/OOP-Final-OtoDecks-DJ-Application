#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager)
  : formatManager(_formatManager),
    lastSampleRate(0.0)
{
}

DJAudioPlayer::~DJAudioPlayer()
{
}

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
  resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

  lastSampleRate = sampleRate;
  lowFilterSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
  midFilterSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
  highFilterSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
  highFilterSource.getNextAudioBlock(bufferToFill);
}

void DJAudioPlayer::releaseResources()
{
  resampleSource.releaseResources();
  lowFilterSource.releaseResources();
  midFilterSource.releaseResources();
  highFilterSource.releaseResources();
}

void DJAudioPlayer::loadURL(juce::URL audioURL)
{
  auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));
  if (reader != nullptr)
  {
    std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
    readerSource.reset(newSource.release());
  }
}

void DJAudioPlayer::start()
{
  transportSource.start();
}

void DJAudioPlayer::stop()
{
  transportSource.stop();
}

void DJAudioPlayer::setGain(double gain)
{
  if (gain < 0 || gain > 1.0) DBG("> DJAudioPlayer::setGain says: Gain should be between 0 and 1!\n");
  else transportSource.setGain(gain);
}

void DJAudioPlayer::setSpeed(double ratio)
{
  if (ratio < 0 || ratio > 100.0) DBG("> DJAudioPlayer::setSpeed says: Ratio should be between 0 and 100!\n");
  else resampleSource.setResamplingRatio(ratio);
}

void DJAudioPlayer::setPosition(double posInSeconds)
{
  transportSource.setPosition(posInSeconds);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
  if (pos < 0 || pos > 1) DBG("> DJAudioPlayer::setPositionRelative says: Relative Position should be between 0 and " << transportSource.getLengthInSeconds() << "!\n");
  else setPosition(transportSource.getLengthInSeconds() * pos);
}

double DJAudioPlayer::getPositionRelative()
{
  return transportSource.getCurrentPosition() / transportSource.getLengthInSeconds();
}

double DJAudioPlayer::getCurrentLengthInSeconds()
{
  return transportSource.getCurrentPosition();
}

double DJAudioPlayer::getTotalLengthInSeconds()
{
  return transportSource.getLengthInSeconds();
}

void DJAudioPlayer::setLowFilter(double freq)
{
  lowFilterSource.setCoefficients(juce::IIRCoefficients::makeLowPass(lastSampleRate, freq, 1.0 / juce::MathConstants<double>::sqrt2));
}

void DJAudioPlayer::setMidFilter(double freq)
{
  midFilterSource.setCoefficients(juce::IIRCoefficients::makeBandPass(lastSampleRate, freq, 1.0 / juce::MathConstants<double>::sqrt2));
}

void DJAudioPlayer::setHighFilter(double freq)
{
  highFilterSource.setCoefficients(juce::IIRCoefficients::makeHighPass(lastSampleRate, freq, 1.0 / juce::MathConstants<double>::sqrt2));
}

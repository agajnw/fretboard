/*
  ==============================================================================

    Metronome.h
    Created: 16 Jul 2022 1:30:53pm
    Author:  Aga

  ==============================================================================
*/

#pragma once

class Metronome
{
public:
    Metronome()
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        auto inputStream = std::make_unique<juce::MemoryInputStream> (BinaryData::tick_wav,
                                                                    BinaryData::tick_wavSize,
                                                                    false);

        if (auto* reader = formatManager.createReaderFor (std::move (inputStream)))
        {
            fileSampleRate = reader->sampleRate;
            readerSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
            resamplingSource = std::make_unique<juce::ResamplingAudioSource> (readerSource.get(),
                                                                              false,
                                                                              2);
        }
        else
        {
            jassertfalse;
        }
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRateIn)
    {
        sampleRate = sampleRateIn;
        resetInterval();

        if (resamplingSource != nullptr)
        {
            resamplingSource->setResamplingRatio (fileSampleRate / sampleRateIn);
            resamplingSource->prepareToPlay (samplesPerBlockExpected, sampleRateIn);
        }
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        if (! playing || resamplingSource == nullptr)
            return;

        samplesCount += bufferToFill.numSamples;

        if (interval > 0 && samplesCount / interval > 0)
        {
            const auto offset = samplesCount - interval;
            const auto numSamplesToFill = bufferToFill.numSamples - offset;

            readerSource->setNextReadPosition (0);
            resamplingSource->getNextAudioBlock ({ bufferToFill.buffer,
                                                   offset,
                                                   numSamplesToFill });

            samplesCount = samplesCount % interval;
        }
        else if (const auto p = readerSource->getNextReadPosition();
                 p != 0 && p < readerSource->getTotalLength())
        {
            resamplingSource->getNextAudioBlock (bufferToFill);
        }
    }

    void setPlaying (bool playingIn)
    {
        if (playing == playingIn)
            return;

        playing = playingIn;

        samplesCount = {};
    }

    bool isPlaying() const noexcept { return playing; }

    void setBpm (double val)
    {
        bpm = val;
        resetInterval();
    }

    double getBpm() const noexcept { return bpm; }

    static constexpr auto minBpm = 40;
    static constexpr auto maxBpm = 80;

private:
    void resetInterval()
    {
        interval = (int) (60.0 / bpm * sampleRate);
    }

    std::atomic<double> bpm = minBpm;
    std::atomic<int> interval;
    int samplesCount{};
    double fileSampleRate = 44100.0;
    double sampleRate{};
    bool playing{};

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
};

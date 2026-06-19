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
        if (! playing || interval <= 0 || resamplingSource == nullptr)
            return;

        auto position = bufferToFill.startSample;
        const auto endSample = bufferToFill.startSample + bufferToFill.numSamples;

        while (position < endSample)
        {
            if (samplesUntilNextTick <= 0)
            {
                readerSource->setNextReadPosition (0);
                samplesUntilNextTick = interval;
            }

            const auto samplesToEnd = endSample - position;
            const auto samplesThisStep = juce::jmin (samplesToEnd, samplesUntilNextTick);

            if (readerSource->getNextReadPosition() < readerSource->getTotalLength())
            {
                resamplingSource->getNextAudioBlock ({ bufferToFill.buffer,
                                                       position,
                                                       samplesThisStep });
            }

            samplesUntilNextTick -= samplesThisStep;
            position += samplesThisStep;
        }
    }

    void setPlaying (bool playingIn)
    {
        if (playing == playingIn)
            return;

        playing = playingIn;
        samplesUntilNextTick = interval;
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
        interval = sampleRate > 0.0 ? (int) std::round (60.0 / bpm * sampleRate) : 0;
        samplesUntilNextTick = interval;
    }

    std::atomic<double> bpm = minBpm;
    double fileSampleRate = 44100.0;
    double sampleRate{};
    int samplesUntilNextTick{};
    int interval{};
    bool playing{};

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
};

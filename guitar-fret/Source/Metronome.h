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
        auto formatReader = []
        {
            juce::AudioFormatManager formatManager;

            formatManager.registerBasicFormats();

            auto inputStream = std::make_unique<juce::MemoryInputStream> (BinaryData::tick_wav,
                                                                          BinaryData::tick_wavSize,
                                                                          false);

            return formatManager.createReaderFor (std::move (inputStream));
        }();

        if (formatReader == nullptr)
        {
            jassertfalse;
            return;
        }

        sampleReader = std::make_unique<juce::AudioFormatReaderSource> (formatReader, true);
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRateIn)
    {
        sampleRate = sampleRateIn;
        resetInterval();

        if (sampleReader != nullptr)
            sampleReader->prepareToPlay (samplesPerBlockExpected, sampleRateIn);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
    {
        if (! playing)
            return;

        samplesCount += bufferToFill.numSamples;

        if (samplesCount / interval > 0)
        {
            const auto offset = samplesCount - interval;
            const auto numSamplesToFill = bufferToFill.numSamples - offset;

            sampleReader->setNextReadPosition (0);
            sampleReader->getNextAudioBlock ({ bufferToFill.buffer,
                                               offset,
                                               numSamplesToFill });

            samplesCount = samplesCount % interval;
        }
        else if (const auto p = sampleReader->getNextReadPosition();
                 p != 0 && p < sampleReader->getTotalLength())
        {
            sampleReader->getNextAudioBlock (bufferToFill);
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
    double sampleRate{};
    bool playing{};

    std::unique_ptr <juce::AudioFormatReaderSource> sampleReader { nullptr };
};

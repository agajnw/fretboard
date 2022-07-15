/*
  ==============================================================================

    FretView.h
    Created: 15 Jul 2022 10:17:50pm
    Author:  Aga

  ==============================================================================
*/

#pragma once

struct FretView  : public juce::Component
{
    FretView (const juce::String& name = {})
        : label (name, name)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId,
                         juce::Colours::darkslategrey);

        addChildComponent (label);
        label.setVisible (name.isNotEmpty());
    }

    void setNoteName (const juce::String& name)
    {
        label.setText (name, juce::dontSendNotification);
        label.setVisible (name.isNotEmpty());
    }

    void resized() override
    {
        label.setBounds (getLocalBounds().withSizeKeepingCentre (circleSize,
                                                                 circleSize));
    }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds();

        const auto drawString = [&g, centreY = b.getCentreY()] (auto left, auto right)
        {
            g.fillRect (left, centreY, right - left, 1);
        };

        if (bgColour.has_value())
            g.fillAll (*bgColour);

        if (! label.isVisible())
        {
            g.setColour (lineColour);
            drawString (b.getX(), b.getRight());
        }
        else
        {
            const auto circleB = b.withSizeKeepingCentre (circleSize, circleSize);

            g.setColour (circleColour);
            g.fillEllipse (circleB.toFloat());

            g.setColour (lineColour);
            drawString (b.getX(), circleB.getX());
            drawString (circleB.getRight(), b.getRight());
        }

        g.fillRect (b.getRight() - 1, b.getY(), 1, b.getHeight());
    }

    juce::Label label;

    juce::Colour circleColour = juce::Colours::lightcyan;
    juce::Colour lineColour = juce::Colours::white;
    std::optional<juce::Colour> bgColour;

    static constexpr auto circleSize = 35;
};

struct StringView  : public juce::Component
{
    StringView (const juce::String& noteNameIn)
        : openNoteName (noteNameIn, noteNameIn)
    {
        openNoteName.setJustificationType (juce::Justification::centred);

        addAndMakeVisible (openNoteName);

        auto noteIndex = (int) std::distance (noteNames.begin(),
                                              std::find (noteNames.begin(),
                                                         noteNames.end(),
                                                         noteNameIn));

        for (auto& f : frets)
        {
            noteIndex = ++noteIndex % noteNames.size();
            f.setNoteName (noteNames [noteIndex]);

            addAndMakeVisible (f);
        }
    }

    void resized() override
    {
        auto b = getLocalBounds();

        openNoteName.setBounds (b.removeFromLeft (FretView::circleSize)
                                 .withSizeKeepingCentre (FretView::circleSize,
                                                         FretView::circleSize));

        juce::FlexBox box;

        for (auto& f : frets)
            box.items.add (juce::FlexItem { f }.withFlex (1.0f));

        box.performLayout (b);
    }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds();

        g.setColour (juce::Colours::white);
        g.fillRect (b.getX() + FretView::circleSize - 1, b.getY(), 2, b.getHeight());
    }

    static constexpr auto numFrets = 12;

    std::array<juce::String, 12> noteNames { "C", "C#", "D", "D#", "E", "F",
                                             "F#", "G", "G#", "A", "A#", "B" };

    std::array<FretView, numFrets> frets;
    juce::Label openNoteName;
};

struct FretboardView  : public juce::Component
{
    FretboardView()
    {
        for (auto& s : strings)
            addAndMakeVisible (s);
    }

    void resized() override
    {
        juce::FlexBox box;
        box.flexDirection = juce::FlexBox::Direction::columnReverse;

        for (auto& s : strings)
            box.items.add (juce::FlexItem { s }.withFlex (1.0f));

        box.performLayout (getLocalBounds());
    }

    static constexpr auto numStrings = 6;
    std::array<StringView, numStrings> strings { juce::String ("E"),
                                                 { "A" },
                                                 { "D" },
                                                 { "G" },
                                                 { "B" },
                                                 { "E" } };
};

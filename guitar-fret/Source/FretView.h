/*
  ==============================================================================

    FretView.h
    Created: 15 Jul 2022 10:17:50pm
    Author:  Aga

  ==============================================================================
*/

#pragma once

struct NoteData
{
    juce::String name;
    juce::Colour colour;
    size_t index{};
};

struct FretView  : public juce::Component
{
    FretView (const NoteData& d = {})
        : label (d.name, d.name),
          data (d)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId,
                         juce::Colours::darkgrey);

        addChildComponent (label);
        label.setVisible (data.name.isNotEmpty());
    }

    void setData (const NoteData& d)
    {
        data = d;

        label.setText (data.name, juce::dontSendNotification);
        label.setVisible (data.name.isNotEmpty());
    }

    void setBgColour (juce::Colour col)
    {
        bgColour = col;
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
            g.fillAll (bgColour->withAlpha (0.2f));

        if (! label.isVisible())
        {
            g.setColour (lineColour);
            drawString (b.getX(), b.getRight());
        }
        else
        {
            const auto circleB = b.withSizeKeepingCentre (circleSize,
                                                          circleSize);

            g.setColour (data.colour.interpolatedWith (juce::Colours::white, 0.6f));
            g.fillEllipse (circleB.toFloat());

            g.setColour (lineColour);
            drawString (b.getX(), circleB.getX());
            drawString (circleB.getRight(), b.getRight());
        }

        g.fillRect (b.getRight() - 1, b.getY(), 1, b.getHeight());
    }

    static constexpr auto circleSize = 35;

private:
    juce::Label label;

    NoteData data;

    juce::Colour lineColour = juce::Colours::white;
    std::optional<juce::Colour> bgColour;
};

struct StringView  : public juce::Component
{
    StringView (const juce::String& noteNameIn)
        : openNoteName (noteNameIn, noteNameIn)
    {
        openNoteName.setJustificationType (juce::Justification::centred);

        addAndMakeVisible (openNoteName);

        auto noteIndex = (int) std::distance (noteData.begin(),
                                              std::find_if (noteData.begin(),
                                                            noteData.end(),
                                                            [&noteNameIn] (const auto& n)
                                                            { return n.name == noteNameIn; }));

        for (auto& f : frets)
        {
            noteIndex = ++noteIndex % noteData.size();

            f.setData (noteData[noteIndex]);

            addAndMakeVisible (f);
        }

        for (auto i : { 3, 5, 7, 9, 12 })
            frets[i - 1].setBgColour (fretHighlight);
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

private:
    static constexpr auto numFrets = 12;
    static constexpr auto numNaturals = 7;

    std::array<NoteData, numNaturals> noteData {{ { "C", juce::Colours::crimson, 0 },
                                                  { "D", juce::Colours::cyan, 1 },
                                                  { "E", juce::Colours::green, 2 },
                                                  { "F", juce::Colours::magenta, 3 },
                                                  { "G", juce::Colours::orange, 4 },
                                                  { "A", juce::Colours::yellowgreen, 5 },
                                                  { "B", juce::Colours::blueviolet, 6 } }};

    juce::Colour fretHighlight = juce::Colours::white;

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

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
    }

private:
    static constexpr auto numStrings = 6;
    std::array<StringView, numStrings> strings { juce::String ("E"),
                                                 { "A" },
                                                 { "D" },
                                                 { "G" },
                                                 { "B" },
                                                 { "E" } };
};

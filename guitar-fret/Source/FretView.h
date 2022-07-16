/*
  ==============================================================================

    FretView.h
    Created: 15 Jul 2022 10:17:50pm
    Author:  Aga

  ==============================================================================
*/

#pragma once

#include "Metronome.h"

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
        setLabelVisible (data.name.isNotEmpty());
    }

    size_t getNoteIndex() const { return data.index; }

    void setBgColour (juce::Colour col)
    {
        bgColour = col;
    }

    void setLabelVisible (bool vis)
    {
        label.setVisible (vis && data.name.isNotEmpty());
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

    void showFrets (const std::vector<size_t>& indices)
    {
        const auto showAll = indices.empty();

        for (auto& f : frets)
        {
            f.setLabelVisible (showAll || std::find (indices.begin(),
                                                     indices.end(),
                                                     f.getNoteIndex()) != indices.end());
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

private:
    static constexpr auto numFrets = 12;

    std::array<NoteData, numFrets> noteData {{ { "C", juce::Colours::crimson, 0 },
                                               { "", juce::Colours::transparentBlack, 1 },
                                               { "D", juce::Colours::cyan, 2 },
                                               { "", juce::Colours::transparentBlack, 3 },
                                               { "E", juce::Colours::green, 4 },
                                               { "F", juce::Colours::magenta, 5 },
                                               { "", juce::Colours::transparentBlack, 6 },
                                               { "G", juce::Colours::orange, 7 },
                                               { "", juce::Colours::transparentBlack, 8 },
                                               { "A", juce::Colours::yellowgreen, 9 },
                                               { "", juce::Colours::blueviolet, 10 },
                                               { "B", juce::Colours::transparentBlack, 11 }, }};

    juce::Colour fretHighlight = juce::Colours::white;

    std::array<FretView, numFrets> frets;
    juce::Label openNoteName;
};

//=====================================================================================
struct ToggleButton  : public juce::Button
{
    ToggleButton (const juce::String& name = {},
                  juce::Colour colourIn = juce::Colours::white)
        : juce::Button (name),
          label (name, name),
          colour (colourIn)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId,
                         juce::Colours::darkgrey);
        label.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (label);

        setClickingTogglesState (true);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        auto b = getLocalBounds();
        auto colourToSet = [&]
        {
            auto col = getToggleState() ? colour.interpolatedWith (juce::Colours::white, 0.6f)
                                        : juce::Colours::grey;

            if (shouldDrawButtonAsHighlighted)
                col = col.brighter (0.3f);

            if (shouldDrawButtonAsDown)
                col = col.darker (0.5f);

            return col;
        }();

        g.setColour (colourToSet);
        g.fillEllipse (b.toFloat());
    }

    void resized() override
    {
        label.setBounds (getLocalBounds());
    }

    juce::Label label;
    juce::Colour colour;

    static constexpr auto height = 20;
    static constexpr auto width = 30;
};

struct NoteConfigItem  : public ToggleButton
{
    NoteConfigItem (const NoteData& d = {})
        : ToggleButton (d.name, d.colour),
          data (d)
    {}

    void setData (const NoteData& d)
    {
        data = d;
        colour = data.colour;

        label.setText (data.name, juce::dontSendNotification);
    }

    NoteData data;
};

struct ConfigView  : public juce::Component
{
    ConfigView()
    {
        for (size_t i = 0; i < noteItems.size(); ++i)
        {
            auto& item = noteItems[i];
            item.setData (noteData[i]);
            addAndMakeVisible (item);
        }

        bpm.setSliderSnapsToMousePosition (false);

        addAndMakeVisible (bpm);
        addAndMakeVisible (bpmToggle);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        juce::FlexBox box;
        box.alignItems = juce::FlexBox::AlignItems::center;

        box.items.add (juce::FlexItem{}.withFlex (1.0f));

        for (auto& n : noteItems)
        {
            box.items.addArray ({ juce::FlexItem { NoteConfigItem::width,
                                                   NoteConfigItem::height,
                                                   n },
                                  juce::FlexItem{}.withFlex (0.1f) });
        }

        box.items.add (juce::FlexItem{}.withFlex (0.9f));

        box.performLayout (b);

        juce::FlexBox metronomeBox;

        metronomeBox.alignItems = juce::FlexBox::AlignItems::center;
        metronomeBox.items.addArray ({ juce::FlexItem{}.withFlex (1.0f),
                                       juce::FlexItem { NoteConfigItem::width,
                                                        NoteConfigItem::height,
                                                        bpmToggle },
                                       juce::FlexItem{}.withWidth (margin),
                                       juce::FlexItem { NoteConfigItem::width * 4,
                                                        NoteConfigItem::height,
                                                        bpm },
                                       juce::FlexItem{}.withFlex (1.0f) });

        metronomeBox.performLayout (box.items.getLast().currentBounds);
    }

    //=================================================================================
    static constexpr auto numNaturals = 7;
    std::array<NoteConfigItem, numNaturals> noteItems;

    juce::Slider bpm { juce::Slider::IncDecButtons, juce::Slider::TextBoxRight };
    ToggleButton bpmToggle { "M", juce::Colours::orange };

    std::array<NoteData, numNaturals> noteData {{ { "C", juce::Colours::crimson, 0 },
                                                  { "D", juce::Colours::cyan, 2 },
                                                  { "E", juce::Colours::green, 4 },
                                                  { "F", juce::Colours::magenta, 5 },
                                                  { "G", juce::Colours::orange, 7 },
                                                  { "A", juce::Colours::yellowgreen, 9 },
                                                  { "B", juce::Colours::blueviolet, 11 } }};

    static constexpr auto margin = 6;
    static constexpr auto height = NoteConfigItem::height * 2 + 2 * margin;
};

//=====================================================================================
struct FretboardView  : public juce::Component
{
    FretboardView()
    {
        for (auto& s : strings)
            addAndMakeVisible (s);

        addAndMakeVisible (config);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        config.setBounds (b.removeFromBottom (ConfigView::height));

        juce::FlexBox box;
        box.flexDirection = juce::FlexBox::Direction::columnReverse;

        for (auto& s : strings)
            box.items.add (juce::FlexItem { s }.withFlex (1.0f));

        box.performLayout (b);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
    }

    static constexpr auto numStrings = 6;
    std::array<StringView, numStrings> strings { juce::String ("E"),
                                                 { "A" },
                                                 { "D" },
                                                 { "G" },
                                                 { "B" },
                                                 { "E" } };

    ConfigView config;
};

//=====================================================================================
class FretboardController
{
public:
    FretboardController (Metronome& metronome,
                         FretboardView& viewIn)
        : view (viewIn),
          config (view.config)
    {
        for (auto& c : config.noteItems)
            c.onClick = [this] { updateSelection(); };

        config.bpm.setRange (Metronome::minBpm, Metronome::maxBpm, 1.0);
        config.bpm.setValue (metronome.getBpm());

        config.bpmToggle.setToggleState (metronome.isPlaying(),
                                         juce::dontSendNotification);

        config.bpm.onValueChange = [this, &metronome]
        {
            metronome.setBpm (config.bpm.getValue());
        };

        config.bpmToggle.onClick = [this, &metronome]
        {
            metronome.setPlaying (config.bpmToggle.getToggleState());
        };
    }

    ~FretboardController()
    {
        for (auto& c : config.noteItems)
            c.onClick = nullptr;

        config.bpm.onValueChange = nullptr;
        config.bpmToggle.onClick = nullptr;
    }

private:
    void updateSelection()
    {
        auto& configItems = view.config.noteItems;

        std::vector<size_t> selectedItems;

        for (auto& c : configItems)
        {
            if (c.getToggleState())
                selectedItems.emplace_back (c.data.index);
        }

        showItems (selectedItems);
    }

    void showItems (const std::vector<size_t>& indices)
    {
        for (auto& s : view.strings)
            s.showFrets (indices);
    }

    FretboardView& view;
    ConfigView& config;
};

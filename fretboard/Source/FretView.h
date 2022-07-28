/*
  ==============================================================================

    FretView.h
    Created: 15 Jul 2022 10:17:50pm
    Author:  Aga

  ==============================================================================
*/

#pragma once

#include "Metronome.h"

namespace Styling
{
    namespace Colours
    {
        static inline const juce::Colour text { 0xffdce2f2 };
        static inline const juce::Colour string { 0xFF60666C };
        static inline const juce::Colour fret { 0xFF59636D };
        static inline const juce::Colour metronome { 0xFF8ABC62 };
        static inline const juce::Colour deselected { 0xFF6D758B };
        static inline const juce::Colour background { 0xFF161C22 };
        static inline const juce::Colour bgRaised { 0xFF363E55 };
        static inline const juce::Colour bgHalfRaised = bgRaised.withAlpha (0.4f);
    }

    inline void styleLabel (juce::Label& label)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId,
                         Styling::Colours::text);
        label.setInterceptsMouseClicks (false, false);
    }
}

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
        Styling::styleLabel (label);

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

    void setDrawLineUp (bool up) { drawLineUp = up; }
    void setDrawLineDown (bool down) { drawLineDown = down; }

    void resized() override
    {
        label.setBounds (getLocalBounds().withSizeKeepingCentre (circleSize,
                                                                 circleSize));
    }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds();
        const auto centreY = b.getCentreY();

        const auto drawString = [&] (auto left, auto right)
        {
            g.fillRect (left, centreY, right - left, 1);
        };

        if (bgColour.has_value())
            g.fillAll (*bgColour);

        if (! label.isVisible())
        {
            g.setColour (Styling::Colours::string);
            drawString (b.getX(), b.getRight());
        }
        else
        {
            const auto circleB = b.withSizeKeepingCentre (circleSize, circleSize);

            g.setColour (data.colour);
            g.fillEllipse (circleB.toFloat());

            g.setColour (Styling::Colours::string);
            drawString (b.getX(), circleB.getX());
            drawString (circleB.getRight(), b.getRight());
        }

        if (drawLineUp)
            g.fillRect (b.getX(), b.getY(), 1, b.getHeight() / 2 + 2);

        if (drawLineDown)
            g.fillRect (b.getX(), centreY - 1, 1, b.getHeight() / 2 + 1);
    }

    static constexpr auto circleSize = 35;

private:
    juce::Label label;

    NoteData data;

    std::optional<juce::Colour> bgColour;

    bool drawLineUp = true;
    bool drawLineDown = true;
};

struct StringView  : public juce::Component
{
    StringView (const juce::String& noteNameIn)
        : openNoteName (noteNameIn, noteNameIn)
    {
        Styling::styleLabel (openNoteName);

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
            frets[i - 1].setBgColour (Styling::Colours::bgHalfRaised);
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

    void setDrawLineUp (bool up)
    {
        for (auto& f : frets)
            f.setDrawLineUp (up);
    }

    void setDrawLineDown (bool down)
    {
        for (auto& f : frets)
            f.setDrawLineDown (down);
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

private:
    static constexpr auto numFrets = 12;

    std::array<NoteData, numFrets> noteData {{ { "C", juce::Colour { 0xFF499271 }, 0 },
                                               { "", juce::Colour { 0x00000000 }, 1 },
                                               { "D", juce::Colour { 0xFF50819B }, 2 },
                                               { "", juce::Colour { 0x00000000 }, 3 },
                                               { "E", juce::Colour { 0xFF6A519D }, 4 },
                                               { "F", juce::Colour { 0xFF8F528E }, 5 },
                                               { "", juce::Colour { 0x00000000 }, 6 },
                                               { "G", juce::Colour { 0xFF9C4E60 }, 7 },
                                               { "", juce::Colour { 0x00000000 }, 8 },
                                               { "A", juce::Colour { 0xFF94874C }, 9 },
                                               { "", juce::Colour { 0x00000000 }, 10 },
                                               { "B", juce::Colour { 0xFFAC6F4F }, 11 }, }};

    std::array<FretView, numFrets> frets;
    juce::Label openNoteName;
};

//=====================================================================================
struct ToggleButton  : public juce::Button
{
    ToggleButton (const juce::String& name = {},
                  juce::Colour colourIn = juce::Colours::black)
        : juce::Button (name),
          label (name, name),
          colour (colourIn)
    {
        Styling::styleLabel (label);
        label.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (label);

        setClickingTogglesState (true);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        auto b = getLocalBounds();

        g.setColour (getBgColour (shouldDrawButtonAsHighlighted,
                                  shouldDrawButtonAsDown));

        g.fillEllipse (b.toFloat());
    }

    void resized() override
    {
        label.setBounds (getLocalBounds());
    }

    juce::Colour getBgColour (bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) const
    {
        auto col = getToggleState() ? colour : Styling::Colours::deselected;

        if (shouldDrawButtonAsHighlighted)
            col = col.brighter (0.3f);

        if (shouldDrawButtonAsDown)
            col = col.darker (0.5f);

        return col;
    }

    juce::Label label;
    juce::Colour colour;

    static constexpr auto height = 30;
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

struct BpmControl  : public ToggleButton,
                     private juce::Slider::Listener
{
    BpmControl()
        : ToggleButton ("M", Styling::Colours::metronome)
    {
        Styling::styleLabel (value);

        slider.setSliderSnapsToMousePosition (false);
        slider.setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_Vertical);

        label.setColour (juce::Label::ColourIds::textColourId,
                         Styling::Colours::text);
        slider.setColour (juce::TextButton::textColourOnId,
                          juce::Colours::transparentBlack);

        addAndMakeVisible (slider);
        addAndMakeVisible (value);

        slider.addListener (this);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        auto b = getLocalBounds();

        g.setColour (getBgColour (shouldDrawButtonAsHighlighted,
                                  shouldDrawButtonAsDown));

        g.fillRoundedRectangle (b.toFloat(), 12.0f);
    }

    void resized() override
    {
        juce::FlexBox box;
        box.alignItems = juce::FlexBox::AlignItems::center;
        box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

        const auto margin = 4.0f;
        const auto height = getHeight() - 2 * margin;

        box.items.addArray ({ juce::FlexItem (25, height, label),
                              juce::FlexItem (30, height, slider),
                              juce::FlexItem (25, height, value) });

        box.performLayout (getLocalBounds());
    }

    void sliderValueChanged (juce::Slider*) override
    {
        value.setText (juce::String ((int) slider.getValue()),
                       juce::dontSendNotification);
    }

    juce::Label value { {}, "40" };
    juce::Slider slider { juce::Slider::IncDecButtons, juce::Slider::NoTextBox };

    static constexpr auto height = 30;
    static constexpr auto width = 83;
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

        addAndMakeVisible (bpm);
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
                                       juce::FlexItem{}.withWidth (margin),
                                       juce::FlexItem { BpmControl::width,
                                                        BpmControl::height,
                                                        bpm },
                                       juce::FlexItem{}.withFlex (1.0f) });

        metronomeBox.performLayout (box.items.getLast().currentBounds);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Styling::Colours::bgRaised);
    }

    //=================================================================================
    static constexpr auto numNaturals = 7;
    std::array<NoteConfigItem, numNaturals> noteItems;

    BpmControl bpm;

    std::array<NoteData, numNaturals> noteData {{  { "C", juce::Colour { 0xFF499271 }, 0 },
                                                   { "D", juce::Colour { 0xFF50819B }, 2 },
                                                   { "E", juce::Colour { 0xFF6A519D }, 4 },
                                                   { "F", juce::Colour { 0xFF8F528E }, 5 },
                                                   { "G", juce::Colour { 0xFF9C4E60 }, 7 },
                                                   { "A", juce::Colour { 0xFF94874C }, 9 },
                                                   { "B", juce::Colour { 0xFFAC6F4F }, 11 } }};

    static constexpr auto margin = 6;
    static constexpr auto height = NoteConfigItem::height * 2 + 2 * margin;
};

//=====================================================================================
struct NumbersView  : public juce::Component
{
    NumbersView()
    {
        for (auto i = 0; i < 12; ++i)
            box.items.add (juce::FlexItem{}.withFlex (1.0f));

        for (auto& n : numbers)
            addAndMakeVisible (n);
    }

    void resized() override
    {
        box.performLayout (getLocalBounds());

        for (auto& n : numbers)
            n.setBounds (box.items[n.index - 1].currentBounds.toNearestInt());
    }

    struct Number  : public juce::Component
    {
        Number (int i)
            : label (juce::String (i), juce::String (i)),
              index (i)
        {
            Styling::styleLabel (label);

            addAndMakeVisible (label);
        }

        void resized() override
        {
            label.setBounds (getLocalBounds());
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (Styling::Colours::bgHalfRaised);
        }

        juce::Label label;
        int index{};
    };

    juce::FlexBox box;
    std::array<Number, 5> numbers = {{ { 3 },
                                       { 5 },
                                       { 7 },
                                       { 9 },
                                       { 12 } }};
};

//=====================================================================================
struct FretboardView  : public juce::Component
{
    FretboardView()
    {
        for (auto& s : strings)
            addAndMakeVisible (s);

        strings[0].setDrawLineDown (false);
        strings[strings.size() - 1].setDrawLineUp (false);

        addAndMakeVisible (config);
        addAndMakeVisible (numbers);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        config.setBounds (b.removeFromBottom (ConfigView::height));
        numbers.setBounds (b.removeFromTop (ConfigView::height / 2)
                            .withTrimmedLeft (FretView::circleSize));

        juce::FlexBox box;
        box.flexDirection = juce::FlexBox::Direction::columnReverse;

        for (auto& s : strings)
            box.items.add (juce::FlexItem { s }.withFlex (1.0f));

        box.performLayout (b);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour { Styling::Colours::background });

        const auto getCentreY = [this] (auto& c)
        {
            return getLocalArea (&c, c.getLocalBounds()).getCentreY();
        };

        g.setColour (Styling::Colours::fret);

        auto b = getLocalBounds();
        const auto topY = getCentreY (strings.back());
        const auto bottomY = getCentreY (strings[0]);

        g.fillRect (b.getX() + FretView::circleSize - 1,
                    topY,
                    2,
                    bottomY - topY);
    }

    static constexpr auto numStrings = 6;
    std::array<StringView, numStrings> strings { juce::String ("E"),
                                                 { "A" },
                                                 { "D" },
                                                 { "G" },
                                                 { "B" },
                                                 { "E" } };
    NumbersView numbers;
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

        config.bpm.slider.setRange (Metronome::minBpm, Metronome::maxBpm, 1.0);
        config.bpm.slider.setValue (metronome.getBpm());

        config.bpm.setToggleState (metronome.isPlaying(),
                                   juce::dontSendNotification);

        config.bpm.slider.onValueChange = [this, &metronome]
        {
            metronome.setBpm (config.bpm.slider.getValue());
        };

        config.bpm.onClick = [this, &metronome]
        {
            metronome.setPlaying (config.bpm.getToggleState());
        };
    }

    ~FretboardController()
    {
        for (auto& c : config.noteItems)
            c.onClick = nullptr;

        config.bpm.slider.onValueChange = nullptr;
        config.bpm.onClick = nullptr;
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

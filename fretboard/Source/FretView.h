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
        inline const juce::Colour text { 0xffdce2f2 };
        inline const juce::Colour string { 0xFF60666C };
        inline const juce::Colour fret { 0xFF59636D };
        inline const juce::Colour metronome { 0xFF8ABC62 };
        inline const juce::Colour deselected { 0xFF6D758B };
        inline const juce::Colour background { 0xFF161C22 };
        inline const juce::Colour bgRaised { 0xFF363E55 };
        inline const juce::Colour bgHalfRaised = bgRaised.withAlpha (0.4f);
    }

    void styleLabel (juce::Label& label);
}

struct NoteData
{
    juce::String name;
    juce::Colour colour;
    size_t index{};
};

struct FretView  : public juce::Component
{
    explicit FretView (const NoteData& d = {});

    void setData (const NoteData& d);
    size_t getNoteIndex() const;
    void setBgColour (juce::Colour col);
    void setLabelVisible (bool vis);
    void setDrawLineUp (bool up);
    void setDrawLineDown (bool down);

    void resized() override;
    void paint (juce::Graphics& g) override;

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
    explicit StringView (const juce::String& noteNameIn);

    void showFrets (const std::vector<size_t>& indices);
    void setDrawLineUp (bool up);
    void setDrawLineDown (bool down);

    void resized() override;

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
                                                { "B", juce::Colour { 0xFFAC6F4F }, 11 } }};

    std::array<FretView, numFrets> frets;
    juce::Label openNoteName;
};

struct ToggleButton  : public juce::Button
{
    ToggleButton (const juce::String& name = {},
                  juce::Colour colourIn = juce::Colours::black);

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

    void resized() override;

    juce::Colour getBgColour (bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) const;

    virtual bool isHighlighted() const;

    juce::Label label;
    juce::Colour colour;

    static constexpr auto height = 30;
    static constexpr auto width = 30;
};

struct NoteConfigItem  : public ToggleButton
{
    explicit NoteConfigItem (const NoteData& d = {});

    void setData (const NoteData& d);
    bool isHighlighted() const override;

    NoteData data;
    bool showingAll{};
};

struct BpmControl  : public ToggleButton,
                     private juce::Slider::Listener
{
    BpmControl();

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

    void resized() override;

    juce::Label value { {}, "40" };
    juce::Slider slider { juce::Slider::IncDecButtons, juce::Slider::NoTextBox };

    static constexpr auto height = 30;
    static constexpr auto width = 83;

private:
    void sliderValueChanged (juce::Slider*) override;
};

struct ConfigView  : public juce::Component
{
    ConfigView();

    void resized() override;
    void paint (juce::Graphics& g) override;

    enum class NoteOrder { ascending, random };

    void updateNoteOrder (NoteOrder noteOrder);
    void updateHighlight();

    static constexpr auto numNaturals = 7;

    std::array<NoteConfigItem, numNaturals> noteItems;
    std::vector<int> noteItemsOrder;

    ToggleButton defaultOrderButton { "Default" };
    ToggleButton randomOrderButton { "Random" };
    BpmControl bpm;

    std::array<NoteData, numNaturals> noteData {{ { "C", juce::Colour { 0xFF499271 }, 0 },
                                                  { "D", juce::Colour { 0xFF50819B }, 2 },
                                                  { "E", juce::Colour { 0xFF6A519D }, 4 },
                                                  { "F", juce::Colour { 0xFF8F528E }, 5 },
                                                  { "G", juce::Colour { 0xFF9C4E60 }, 7 },
                                                  { "A", juce::Colour { 0xFF94874C }, 9 },
                                                  { "B", juce::Colour { 0xFFAC6F4F }, 11 } }};

    static constexpr auto margin = 6;
    static constexpr auto height = NoteConfigItem::height * 2 + 2 * margin;
};

struct NumbersView  : public juce::Component
{
    NumbersView();

    void resized() override;

    struct Number  : public juce::Component
    {
        explicit Number (int i);

        void resized() override;
        void paint (juce::Graphics& g) override;

        juce::Label label;
        int index{};
    };

    juce::FlexBox box;
    std::array<Number, 5> numbers;
};

struct FretboardView  : public juce::Component
{
    FretboardView();

    void resized() override;
    void paint (juce::Graphics& g) override;

    static constexpr auto numStrings = 6;

    std::array<StringView, numStrings> strings;
    NumbersView numbers;
    ConfigView config;
};

class FretboardController
{
public:
    FretboardController (Metronome& metronome, FretboardView& viewIn);
    ~FretboardController();

private:
    void updateSelection();
    void showItems (const std::vector<size_t>& indices);
    void saveBpmProperty (double value);

    static juce::PropertiesFile::Options getPropertiesFileOptions();

    FretboardView& view;
    ConfigView& config;

    juce::PropertiesFile propertiesFile { getPropertiesFileOptions() };

    const juce::String bpmPropertyId { "bpm" };
};

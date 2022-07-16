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
        setLabelVisible (data.name.isNotEmpty());
    }

    size_t getNoteIndex() const { return data.index; }

    void setBgColour (juce::Colour col)
    {
        bgColour = col;
    }

    void setLabelVisible (bool vis)
    {
        label.setVisible (vis);
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
        for (auto& f : frets)
        {
            f.setLabelVisible (std::find (indices.begin(),
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

//=====================================================================================
struct NoteConfigItem  : public juce::Button
{
    NoteConfigItem (const NoteData& d = {})
        : juce::Button (d.name),
          label (d.name, d.name),
          data (d)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId,
                         juce::Colours::darkgrey);
        label.setInterceptsMouseClicks (false, false);

        addAndMakeVisible (label);

        setClickingTogglesState (true);
    }

    void setData (const NoteData& d)
    {
        data = d;

        label.setText (data.name, juce::dontSendNotification);
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        auto b = getLocalBounds();
        auto colour = [&]
        {
            auto col = getToggleState() ? data.colour.interpolatedWith (juce::Colours::white, 0.6f)
                                        : juce::Colours::grey;

            if (shouldDrawButtonAsHighlighted)
                col = col.brighter (0.3f);

            if (shouldDrawButtonAsDown)
                col = col.darker (0.5f);

            return col;
        }();

        g.setColour (colour);
        g.fillEllipse (b.toFloat());
    }

    void resized() override
    {
        label.setBounds (getLocalBounds());
    }

    juce::Label label;

    NoteData data;

    static constexpr auto height = 20;
    static constexpr auto width = 30;
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
    }

    void resized() override
    {
        auto b = getLocalBounds();

        juce::FlexBox box;
        box.alignItems = juce::FlexBox::AlignItems::center;

        box.items.add (juce::FlexItem{}.withFlex (1.0f));

        for (auto& n : noteItems)
        {
            box.items.addArray ({ juce::FlexItem { NoteConfigItem::width, NoteConfigItem::height, n },
                                  juce::FlexItem{}.withFlex (0.1f) });
        }

        box.items.add (juce::FlexItem{}.withFlex (0.9f));

        box.performLayout (b);
    }

    //=================================================================================
    static constexpr auto numNaturals = 7;
    std::array<NoteConfigItem, numNaturals> noteItems;

    std::array<NoteData, numNaturals> noteData {{ { "C", juce::Colours::crimson, 0 },
                                                  { "D", juce::Colours::cyan, 1 },
                                                  { "E", juce::Colours::green, 2 },
                                                  { "F", juce::Colours::magenta, 3 },
                                                  { "G", juce::Colours::orange, 4 },
                                                  { "A", juce::Colours::yellowgreen, 5 },
                                                  { "B", juce::Colours::blueviolet, 6 } }};

    static constexpr auto margin = 6;
    static constexpr auto height = NoteConfigItem::height + 2 * margin;
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
    FretboardController (FretboardView& viewIn)
        : view (viewIn),
          configItems (view.config.noteItems)
    {
        for (auto& c : configItems)
            c.onClick = [this] { updateSelection(); };
    }

    ~FretboardController()
    {
        for (auto& c : configItems)
            c.onClick = nullptr;
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
    std::array<NoteConfigItem, ConfigView::numNaturals>& configItems;
};

/*
  ==============================================================================

    FretView.cpp
    Created: 15 Jul 2022 10:17:50pm
    Author:  Aga

  ==============================================================================
*/

#include "FretView.h"

namespace Styling
{
    void styleLabel (juce::Label& label)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::ColourIds::textColourId, Colours::text);
        label.setInterceptsMouseClicks (false, false);
    }
}

//==============================================================================
FretView::FretView (const NoteData& d)
    : label (d.name, d.name),
      data (d)
{
    Styling::styleLabel (label);

    addChildComponent (label);
    label.setVisible (data.name.isNotEmpty());
}

void FretView::setData (const NoteData& d)
{
    data = d;

    label.setText (data.name, juce::dontSendNotification);
    setLabelVisible (data.name.isNotEmpty());
}

size_t FretView::getNoteIndex() const
{
    return data.index;
}

void FretView::setBgColour (juce::Colour col)
{
    bgColour = col;
}

void FretView::setLabelVisible (bool vis)
{
    label.setVisible (vis && data.name.isNotEmpty());
}

void FretView::setDrawLineUp (bool up)
{
    drawLineUp = up;
}

void FretView::setDrawLineDown (bool down)
{
    drawLineDown = down;
}

void FretView::resized()
{
    label.setBounds (getLocalBounds().withSizeKeepingCentre (circleSize, circleSize));
}

void FretView::paint (juce::Graphics& g)
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

//==============================================================================
StringView::StringView (const juce::String& noteNameIn)
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
        noteIndex = ++noteIndex % (int) noteData.size();

        f.setData (noteData[(size_t) noteIndex]);

        addAndMakeVisible (f);
    }

    for (auto i : { 3, 5, 7, 9, 12 })
        frets[i - 1].setBgColour (Styling::Colours::bgHalfRaised);
}

void StringView::showFrets (const std::vector<size_t>& indices)
{
    const auto showAll = indices.empty();

    for (auto& f : frets)
    {
        f.setLabelVisible (showAll || std::find (indices.begin(),
                                                 indices.end(),
                                                 f.getNoteIndex()) != indices.end());
    }
}

void StringView::setDrawLineUp (bool up)
{
    for (auto& f : frets)
        f.setDrawLineUp (up);
}

void StringView::setDrawLineDown (bool down)
{
    for (auto& f : frets)
        f.setDrawLineDown (down);
}

void StringView::resized()
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

//==============================================================================
ToggleButton::ToggleButton (const juce::String& name, juce::Colour colourIn)
    : juce::Button (name),
      label (name, name),
      colour (colourIn)
{
    Styling::styleLabel (label);
    label.setInterceptsMouseClicks (false, false);

    addAndMakeVisible (label);

    setClickingTogglesState (true);
}

void ToggleButton::paintButton (juce::Graphics& g,
                                bool shouldDrawButtonAsHighlighted,
                                bool shouldDrawButtonAsDown)
{
    const auto b = getLocalBounds();

    g.setColour (getBgColour (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown));
    g.fillEllipse (b.toFloat());
}

void ToggleButton::resized()
{
    label.setBounds (getLocalBounds());
}

juce::Colour ToggleButton::getBgColour (bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown) const
{
    auto col = isHighlighted() ? colour : Styling::Colours::deselected;

    if (shouldDrawButtonAsHighlighted)
        col = col.brighter (0.3f);

    if (shouldDrawButtonAsDown)
        col = col.darker (0.5f);

    return col;
}

bool ToggleButton::isHighlighted() const
{
    return getToggleState();
}

//==============================================================================
NoteConfigItem::NoteConfigItem (const NoteData& d)
    : ToggleButton (d.name, d.colour),
      data (d)
{}

void NoteConfigItem::setData (const NoteData& d)
{
    data = d;
    colour = data.colour;

    label.setText (data.name, juce::dontSendNotification);
}

bool NoteConfigItem::isHighlighted() const
{
    return getToggleState() || showingAll;
}

//==============================================================================
BpmControl::BpmControl()
    : ToggleButton ("M", Styling::Colours::metronome)
{
    Styling::styleLabel (value);

    slider.setSliderSnapsToMousePosition (false);
    slider.setIncDecButtonsMode (juce::Slider::incDecButtonsDraggable_Vertical);

    label.setColour (juce::Label::ColourIds::textColourId, Styling::Colours::text);
    slider.setColour (juce::TextButton::textColourOnId, juce::Colours::transparentBlack);

    addAndMakeVisible (slider);
    addAndMakeVisible (value);

    slider.addListener (this);
}

void BpmControl::paintButton (juce::Graphics& g,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown)
{
    const auto b = getLocalBounds();

    g.setColour (getBgColour (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown));
    g.fillRoundedRectangle (b.toFloat(), 12.0f);
}

void BpmControl::resized()
{
    juce::FlexBox box;
    box.alignItems = juce::FlexBox::AlignItems::center;
    box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;

    const auto margin = 4.0f;
    const auto controlHeight = (float) getHeight() - 2.0f * margin;

    box.items.addArray ({ juce::FlexItem (25.0f, controlHeight, label),
                          juce::FlexItem (30.0f, controlHeight, slider),
                          juce::FlexItem (25.0f, controlHeight, value) });

    box.performLayout (getLocalBounds());
}

void BpmControl::sliderValueChanged (juce::Slider*)
{
    value.setText (juce::String ((int) slider.getValue()), juce::dontSendNotification);
}

//==============================================================================
ConfigView::ConfigView()
{
    for (size_t i = 0; i < noteItems.size(); ++i)
    {
        auto& item = noteItems[i];
        item.setData (noteData[i]);
        addAndMakeVisible (item);
    }

    for (auto* c : { &defaultOrderButton, &randomOrderButton })
    {
        c->setClickingTogglesState (false);
        addAndMakeVisible (c);
    }

    addAndMakeVisible (bpm);

    defaultOrderButton.onClick = [this] { updateNoteOrder (NoteOrder::ascending); };
    randomOrderButton.onClick = [this] { updateNoteOrder (NoteOrder::random); };

    updateNoteOrder (NoteOrder::ascending);
    updateHighlight();
}

void ConfigView::resized()
{
    auto b = getLocalBounds();

    juce::FlexBox box;
    box.alignItems = juce::FlexBox::AlignItems::center;

    box.items.add (juce::FlexItem{}.withFlex (1.0f));

    for (auto i = 0; i < numNaturals; ++i)
    {
        box.items.addArray ({ juce::FlexItem { NoteConfigItem::width,
                                               NoteConfigItem::height,
                                               noteItems[noteItemsOrder[(size_t) i]] },
                              juce::FlexItem{}.withFlex (0.1f) });
    }

    box.items.add (juce::FlexItem{}.withFlex (0.9f));

    box.performLayout (b);

    juce::FlexBox metronomeBox;

    metronomeBox.alignItems = juce::FlexBox::AlignItems::center;
    metronomeBox.items.addArray ({ juce::FlexItem{}.withFlex (1.0f),
                                   juce::FlexItem{}.withWidth ((float) margin),
                                   juce::FlexItem { BpmControl::width,
                                                    BpmControl::height,
                                                    bpm },
                                   juce::FlexItem{}.withFlex (1.0f) });

    metronomeBox.performLayout (box.items.getLast().currentBounds);

    juce::FlexBox orderBox;

    orderBox.alignItems = juce::FlexBox::AlignItems::center;
    orderBox.items.addArray ({ juce::FlexItem{}.withFlex (1.0f),
                               juce::FlexItem { BpmControl::width,
                                                BpmControl::height,
                                                defaultOrderButton },
                               juce::FlexItem{}.withWidth ((float) margin),
                               juce::FlexItem { BpmControl::width,
                                                BpmControl::height,
                                                randomOrderButton },
                               juce::FlexItem{}.withFlex (1.0f) });
    orderBox.performLayout (box.items.getFirst().currentBounds);
}

void ConfigView::paint (juce::Graphics& g)
{
    g.fillAll (Styling::Colours::bgRaised);
}

void ConfigView::updateNoteOrder (NoteOrder noteOrder)
{
    std::vector<int> defaultIndices;

    for (auto i = 0; i < numNaturals; ++i)
        defaultIndices.emplace_back (i);

    noteItemsOrder.clear();

    if (noteOrder == NoteOrder::ascending)
    {
        noteItemsOrder = defaultIndices;
    }
    else
    {
        juce::Random random;

        for (auto i = numNaturals; i > 0; --i)
        {
            const auto ind = random.nextInt (i);
            const auto picked = defaultIndices[(size_t) ind];

            noteItemsOrder.emplace_back (picked);

            defaultIndices.erase (std::remove (defaultIndices.begin(),
                                               defaultIndices.end(),
                                               picked),
                                  defaultIndices.end());
        }
    }

    resized();
}

void ConfigView::updateHighlight()
{
    const auto firstToggledOn = noteItems.front().getToggleState();
    const auto highlightAll = std::all_of (noteItems.begin(),
                                           noteItems.end(),
                                           [firstToggledOn] (const auto& it)
                                           { return it.getToggleState() == firstToggledOn; });

    for (auto& it : noteItems)
    {
        it.showingAll = highlightAll;
        it.repaint();
    }
}

//==============================================================================
NumbersView::NumbersView()
    : numbers { Number { 3 },
                Number { 5 },
                Number { 7 },
                Number { 9 },
                Number { 12 } }
{
    for (auto i = 0; i < 12; ++i)
        box.items.add (juce::FlexItem{}.withFlex (1.0f));

    for (auto& n : numbers)
        addAndMakeVisible (n);
}

void NumbersView::resized()
{
    box.performLayout (getLocalBounds());

    for (auto& n : numbers)
        n.setBounds (box.items[n.index - 1].currentBounds.toNearestInt());
}

NumbersView::Number::Number (int i)
    : label (juce::String (i), juce::String (i)),
      index (i)
{
    Styling::styleLabel (label);
    addAndMakeVisible (label);
}

void NumbersView::Number::resized()
{
    label.setBounds (getLocalBounds());
}

void NumbersView::Number::paint (juce::Graphics& g)
{
    g.fillAll (Styling::Colours::bgHalfRaised);
}

//==============================================================================
FretboardView::FretboardView()
    : strings { StringView { "E" },
                StringView { "A" },
                StringView { "D" },
                StringView { "G" },
                StringView { "B" },
                StringView { "E" } }
{
    for (auto& s : strings)
        addAndMakeVisible (s);

    strings.front().setDrawLineDown (false);
    strings.back().setDrawLineUp (false);

    addAndMakeVisible (config);
    addAndMakeVisible (numbers);
}

void FretboardView::resized()
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

void FretboardView::paint (juce::Graphics& g)
{
    g.fillAll (Styling::Colours::background);

    const auto getCentreY = [this] (const juce::Component& c)
    {
        return getLocalArea (&c, c.getLocalBounds()).getCentreY();
    };

    const auto b = getLocalBounds();
    const auto topY = getCentreY (strings.back());
    const auto bottomY = getCentreY (strings.front());

    g.setColour (Styling::Colours::fret);
    g.fillRect (b.getX() + FretView::circleSize - 1, topY, 2, bottomY - topY);
}

//==============================================================================
FretboardController::FretboardController (Metronome& metronome, FretboardView& viewIn)
    : view (viewIn),
      config (view.config)
{
    if (auto savedValue = propertiesFile.getDoubleValue (bpmPropertyId); savedValue != 0.0)
        metronome.setBpm (savedValue);

    for (auto& c : config.noteItems)
        c.onClick = [this] { updateSelection(); };

    config.bpm.slider.setRange (Metronome::minBpm, Metronome::maxBpm, 1.0);
    config.bpm.slider.setValue (metronome.getBpm());

    config.bpm.setToggleState (metronome.isPlaying(), juce::dontSendNotification);

    config.bpm.slider.onValueChange = [this, &metronome]
    {
        const auto value = config.bpm.slider.getValue();
        metronome.setBpm (value);
        saveBpmProperty (value);
    };

    config.bpm.onClick = [this, &metronome]
    {
        metronome.setPlaying (config.bpm.getToggleState());
    };
}

FretboardController::~FretboardController()
{
    propertiesFile.saveIfNeeded();

    for (auto& c : config.noteItems)
        c.onClick = nullptr;

    config.bpm.slider.onValueChange = nullptr;
    config.bpm.onClick = nullptr;
}

void FretboardController::updateSelection()
{
    auto& configItems = view.config.noteItems;

    std::vector<size_t> selectedItems;

    for (auto& c : configItems)
    {
        if (c.getToggleState())
            selectedItems.emplace_back (c.data.index);
    }

    showItems (selectedItems);
    view.config.updateHighlight();
}

void FretboardController::showItems (const std::vector<size_t>& indices)
{
    for (auto& s : view.strings)
        s.showFrets (indices);
}

void FretboardController::saveBpmProperty (double value)
{
    propertiesFile.setValue (bpmPropertyId, value);
    propertiesFile.saveIfNeeded();
}

juce::PropertiesFile::Options FretboardController::getPropertiesFileOptions()
{
    juce::PropertiesFile::Options options;

    static const auto name = "Fretboard";

    options.applicationName = name;
    options.osxLibrarySubFolder = "Application Support/" + juce::String (name);
    options.filenameSuffix = ".settings";

    return options;
}

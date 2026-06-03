#include "CommandBar.h"
#include "CustomLookAndFeel.h"
#include "Utils.h"

using LF = UltraLookAndFeel;

namespace
{
    struct KeyDef { int n; const char* label; };
    static const KeyDef defs[] = {
        { 1, "Help" },
        { 2, "Menu" },
        { 3, "View" },
        { 4, "Edit" },
        { 5, "Copy" },
        { 6, "Move" },
        { 7, "MkDir" },
        { 8, "Delete" },
        { 9, "Rename" },
        { 10, "Quit" }
    };
}

CommandBar::CommandBar()
{
    for (auto& d : defs)
    {
        auto* k = new FKey { d.n, juce::String (d.label), std::make_unique<juce::TextButton>() };
        ownedKeys.add (k);
        keys.add (k);

        auto* b = k->button.get();
        addAndMakeVisible (*b);
        b->setButtonText (juce::String ("F") + juce::String (d.n) + "  " + d.label);
        b->setColour (juce::TextButton::buttonColourId, LF::kButtonBg);
        b->setColour (juce::TextButton::textColourOffId, LF::kTextNormal);
        const int n = d.n;
        b->onClick = [this, n] { if (onCommand) onCommand (n); };
    }
}

void CommandBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    juce::ColourGradient grad (LF::kFooterBg.brighter (0.05f), 0.0f, r.getY(),
                               LF::kFooterBg.darker (0.2f), 0.0f, r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRect (r);

    g.setColour (LF::kAccent.withAlpha (0.7f));
    g.fillRect (r.removeFromTop (1.0f));
}

void CommandBar::resized()
{
    auto r = getLocalBounds().reduced (6, 4);
    int n = keys.size();
    int w = r.getWidth() / n;
    for (int i = 0; i < n; ++i)
    {
        auto cell = r.removeFromLeft (i == n - 1 ? r.getWidth() : w).reduced (2, 0);
        keys[i]->button->setBounds (cell);
    }
}

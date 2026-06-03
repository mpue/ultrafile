#pragma once

#include <JuceHeader.h>
#include "Volumes.h"
#include "CustomLookAndFeel.h"
#include "Utils.h"

//==============================================================================
/** Horizontal bar of the machine's drives / mounted volumes.

    - Left-click a volume button -> the active panel navigates to that root.
    - Removable / network mounts get a small eject button (⏏) that safely
      unmounts the media.

    A low-frequency timer re-enumerates volumes so hot-plugged or ejected
    media appear / disappear without manual refresh.

    Header-only so it needs no entry in the generated build projects. */
class DriveBar : public juce::Component,
                 private juce::Timer
{
public:
    using LF = UltraLookAndFeel;

    DriveBar()
    {
        rebuild (UF::getVolumes());
        startTimer (2000);
    }

    /** Active panel should navigate to this root. */
    std::function<void (const juce::File&)> onDriveChosen;

    /** A volume was ejected / the set changed — panels may need refreshing. */
    std::function<void()> onVolumesChanged;

    /** Force a re-enumeration and rebuild now. */
    void refresh() { rebuild (UF::getVolumes()); }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        juce::ColourGradient grad (LF::kHeaderBg.brighter (0.04f), 0.0f, r.getY(),
                                   LF::kHeaderBg.darker (0.18f),  0.0f, r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (r);

        g.setColour (LF::kPanelBorder);
        g.fillRect (r.removeFromBottom (1.0f));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6, 4);
        const int charW = 8; // monospace estimate

        for (auto* e : entries)
        {
            int navW = juce::jmax (46, e->info.name.length() * charW + 16);
            e->nav->setBounds (r.removeFromLeft (navW).reduced (2, 0));

            if (e->eject != nullptr)
                e->eject->setBounds (r.removeFromLeft (24).reduced (1, 0));

            r.removeFromLeft (4);
        }
    }

private:
    //==============================================================================
    struct Entry
    {
        UF::VolumeInfo info;
        std::unique_ptr<juce::TextButton> nav;
        std::unique_ptr<juce::TextButton> eject;
    };

    juce::OwnedArray<Entry> entries;
    juce::String signature;

    //==============================================================================
    void timerCallback() override
    {
        auto vols = UF::getVolumes();
        if (UF::volumesSignature (vols) != signature)
            rebuild (vols);
    }

    void rebuild (const juce::Array<UF::VolumeInfo>& vols)
    {
        entries.clear();
        signature = UF::volumesSignature (vols);

        for (auto& v : vols)
        {
            auto* e = entries.add (new Entry());
            e->info = v;

            e->nav = std::make_unique<juce::TextButton>();
            addAndMakeVisible (*e->nav);
            e->nav->setButtonText (v.name);
            e->nav->setWantsKeyboardFocus (false);
            e->nav->setColour (juce::TextButton::buttonColourId, LF::kButtonBg);
            e->nav->setColour (juce::TextButton::textColourOffId,
                               v.isNetwork ? LF::kTextDim
                                           : (v.isRemovable ? LF::kTextSelected : LF::kTextNormal));
            auto root = v.root;
            e->nav->onClick = [this, root] { if (onDriveChosen) onDriveChosen (root); };

            if (v.ejectable)
            {
                e->eject = std::make_unique<juce::TextButton>();
                addAndMakeVisible (*e->eject);
                e->eject->setButtonText (juce::String::fromUTF8 ("\xE2\x8F\x8F")); // ⏏
                e->eject->setWantsKeyboardFocus (false);
                e->eject->setColour (juce::TextButton::buttonColourId, LF::kButtonBg);
                e->eject->setColour (juce::TextButton::textColourOffId, LF::kAccent);
                e->eject->setTooltip ("Eject " + v.name);
                auto info = v;
                e->eject->onClick = [this, info] { doEject (info); };
            }
        }

        resized();
        repaint();
    }

    void doEject (const UF::VolumeInfo& info)
    {
        juce::String err;
        if (! UF::ejectVolume (info, err))
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                "Eject failed", "Could not eject '" + info.name + "':\n" + err);
            return;
        }

        if (onVolumesChanged) onVolumesChanged();
        refresh();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DriveBar)
};

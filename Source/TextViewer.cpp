#include "TextViewer.h"
#include "CustomLookAndFeel.h"
#include "Utils.h"

using LF = UltraLookAndFeel;

void TextViewerComponent::showFor (const juce::File& file, bool editable, juce::Component* parentForCentering)
{
    juce::DialogWindow::LaunchOptions opts;
    opts.dialogTitle = (editable ? juce::String ("Edit: ") : juce::String ("View: ")) + file.getFileName();
    opts.dialogBackgroundColour = LF::kBackground;
    opts.escapeKeyTriggersCloseButton = false; // we handle Esc ourselves
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    opts.content.setOwned (new TextViewerComponent (file, editable));
    opts.content->setSize (900, 640);

    auto* dw = opts.launchAsync();
    if (parentForCentering != nullptr && dw != nullptr)
        dw->centreAroundComponent (parentForCentering, 900, 640);
}

TextViewerComponent::TextViewerComponent (const juce::File& f, bool ed)
    : file (f), editable (ed)
{
    addAndMakeVisible (header);
    header.setFont (UF::getMonoBoldFont (12.0f));
    header.setColour (juce::Label::textColourId, LF::kAccent);
    header.setText ((editable ? "EDIT " : "VIEW ") + file.getFullPathName(), juce::dontSendNotification);

    addAndMakeVisible (editor);
    editor.setMultiLine (true, false);
    editor.setReturnKeyStartsNewLine (true);
    editor.setReadOnly (! editable);
    editor.setCaretVisible (editable);
    editor.setScrollbarsShown (true);
    editor.setFont (UF::getMonoFont (13.5f));
    editor.setColour (juce::TextEditor::backgroundColourId, LF::kPanelBg);
    editor.setColour (juce::TextEditor::textColourId, LF::kTextNormal);
    editor.setColour (juce::TextEditor::outlineColourId, LF::kPanelBorder);
    editor.setColour (juce::TextEditor::focusedOutlineColourId, LF::kAccent);

    auto content = file.loadFileAsString();
    editor.setText (content, false);

    editor.onTextChange = [this] { dirty = editable; };
    editor.addKeyListener (this);

    if (editable)
    {
        addAndMakeVisible (saveButton);
        saveButton.onClick = [this] { save(); };
    }

    addAndMakeVisible (closeButton);
    closeButton.onClick = [this]
    {
        promptCloseIfDirty ([this] (bool proceed)
        {
            if (! proceed) return;
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState (0);
        });
    };

    setWantsKeyboardFocus (true);
}

TextViewerComponent::~TextViewerComponent() = default;

void TextViewerComponent::resized()
{
    auto r = getLocalBounds().reduced (10);
    auto top = r.removeFromTop (22);
    header.setBounds (top);
    r.removeFromTop (6);

    auto bottom = r.removeFromBottom (32);
    if (editable)
    {
        saveButton.setBounds (bottom.removeFromLeft (120));
        bottom.removeFromLeft (8);
    }
    closeButton.setBounds (bottom.removeFromLeft (120));

    r.removeFromBottom (6);
    editor.setBounds (r);
}

void TextViewerComponent::paint (juce::Graphics& g)
{
    g.fillAll (LF::kBackground);

    auto r = getLocalBounds().toFloat().reduced (4.0f);
    g.setColour (LF::kAccent.withAlpha (0.25f));
    g.drawRoundedRectangle (r, 8.0f, 1.0f);
}

bool TextViewerComponent::keyPressed (const juce::KeyPress& key)
{
    return keyPressed (key, this);
}

bool TextViewerComponent::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress::escapeKey)
    {
        closeButton.triggerClick();
        return true;
    }
    if (editable && key == juce::KeyPress::F2Key)
    {
        save();
        return true;
    }
    return false;
}

void TextViewerComponent::save()
{
    if (! editable) return;
    if (! file.replaceWithText (editor.getText()))
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Save failed",
                                                "Could not write to " + file.getFullPathName());
        return;
    }
    dirty = false;
    header.setText ("EDIT " + file.getFullPathName() + "   [saved]", juce::dontSendNotification);
}

void TextViewerComponent::promptCloseIfDirty (std::function<void(bool)> onDecision)
{
    if (! dirty) { onDecision (true); return; }

    juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon,
        "Unsaved changes",
        "Discard changes to " + file.getFileName() + "?",
        "Discard", "Keep editing",
        nullptr,
        juce::ModalCallbackFunction::create ([onDecision] (int result) { onDecision (result == 1); }));
}

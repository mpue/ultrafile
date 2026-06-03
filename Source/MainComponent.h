#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "FilePanel.h"
#include "CommandBar.h"
#include "DriveBar.h"

class MainComponent  : public juce::Component,
                       public juce::KeyListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // KeyListener — installed on tables so we get keys before the table does
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyPressed (const juce::KeyPress& key) override; // own focus

private:
    void setActivePanel (FilePanel* p);
    FilePanel* otherPanel (FilePanel* p) const;
    FilePanel* active() const { return activePanel; }

    // commands
    void cmdView();
    void cmdEdit();
    void cmdCopy();
    void cmdMove();
    void cmdMkDir();
    void cmdDelete();
    void cmdRename();
    void cmdHelp();
    void cmdQuit();
    void cmdSelectByWildcard (bool select);
    void switchPanel();

    void handleFKey (int n);

    /** Focus the named text editor inside the AlertWindow after it becomes modal,
        select all its text, and wire ESC -> cancel(0) and Return -> accept(acceptResult).
        TextEditor normally swallows ESC, so we route it back to the dialog explicitly. */
    static void focusEditorAndSelectAll (juce::AlertWindow* aw,
                                         const juce::String& editorName,
                                         int acceptResult = 1);

    UltraLookAndFeel lf;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    DriveBar driveBar;
    FilePanel leftPanel  { "Left Panel" };
    FilePanel rightPanel { "Right Panel" };
    CommandBar commandBar;

    FilePanel* activePanel = &leftPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

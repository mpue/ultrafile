#pragma once

#include <JuceHeader.h>
#include "FileEntry.h"

class FilePanel : public juce::Component,
                  public juce::TableListBoxModel
{
public:
    FilePanel (const juce::String& panelName);
    ~FilePanel() override;

    void setDirectory (const juce::File& dir);
    juce::File getDirectory() const           { return currentDir; }

    /** File under the cursor (may be the ".." entry). */
    juce::File getCursorFile() const;

    /** Files marked with Insert/Space. Empty if none marked. */
    juce::Array<juce::File> getMarkedFiles() const;

    /** Marked files, or — if none — the cursor file (excluding ".."). */
    juce::Array<juce::File> getActionFiles() const;

    void refresh();
    void setActive (bool shouldBeActive);
    bool isActive() const                     { return active; }

    void selectAll (bool select);
    void invertSelection();
    void toggleSelectionAtCursor();

    /** Norton-style group select/unselect via wildcard, e.g. "*.txt" or "report?.*".
        Only files are affected (directories and the ".." entry are skipped). */
    void selectByWildcard (const juce::String& pattern, bool select);

    void setSortColumn (SortColumn col, bool ascending);

    // Callbacks
    std::function<void()> onPanelActivated;
    std::function<void (const juce::File&)> onFileChosen;   // Enter on file (not dir)
    std::function<void (const juce::File&)> onDirChanged;
    std::function<void()> onRequestFocus;                   // requests main to give us focus

    //==============================================================================
    // TableListBoxModel
    int getNumRows() override;
    void paintRowBackground (juce::Graphics&, int row, int width, int height,
                             bool isSelected) override;
    void paintCell (juce::Graphics&, int row, int columnId,
                    int width, int height, bool isSelected) override;
    void cellClicked (int row, int columnId, const juce::MouseEvent&) override;
    void cellDoubleClicked (int row, int columnId, const juce::MouseEvent&) override;
    void backgroundClicked (const juce::MouseEvent&) override;
    void sortOrderChanged (int newSortColumnId, bool isForwards) override;
    void returnKeyPressed (int lastRowSelected) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::TableListBox& getTable() { return table; }

private:
    void loadDirectory();
    void sortEntries();
    void updateHeader();
    void updateFooter();
    void enterCursor();

    juce::String panelName;
    juce::File currentDir;
    juce::Array<FileEntry> entries;

    juce::Label titleLabel;     // PANEL NAME at top
    juce::Label pathLabel;      // current path breadcrumb
    juce::TableListBox table;
    juce::Label statusLabel;    // selection summary
    juce::Label footerLabel;    // free space / total

    bool active = false;
    SortColumn sortCol = SortColumn::Name;
    bool sortAscending = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePanel)
};

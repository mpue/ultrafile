#include "FilePanel.h"
#include "CustomLookAndFeel.h"
#include "Utils.h"

using LF = UltraLookAndFeel;

FilePanel::FilePanel (const juce::String& name)
    : panelName (name)
{
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (UF::getMonoBoldFont (12.5f));
    titleLabel.setText (panelName.toUpperCase(), juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, LF::kAccent);
    titleLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (pathLabel);
    pathLabel.setFont (UF::getMonoBoldFont (13.0f));
    pathLabel.setColour (juce::Label::textColourId, LF::kTextNormal);
    pathLabel.setJustificationType (juce::Justification::centredLeft);
    pathLabel.setInterceptsMouseClicks (false, false);

    addAndMakeVisible (statusLabel);
    statusLabel.setFont (UF::getMonoFont (11.5f));
    statusLabel.setColour (juce::Label::textColourId, LF::kTextSelected);
    statusLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (footerLabel);
    footerLabel.setFont (UF::getMonoFont (11.5f));
    footerLabel.setColour (juce::Label::textColourId, LF::kTextDim);
    footerLabel.setJustificationType (juce::Justification::centredRight);

    addAndMakeVisible (table);
    table.setModel (this);
    table.setHeaderHeight (22);
    table.setRowHeight (20);
    table.getHeader().addColumn ("Name",    (int) SortColumn::Name,      260, 80, -1,
        juce::TableHeaderComponent::defaultFlags | juce::TableHeaderComponent::sortable);
    table.getHeader().addColumn ("Ext",     (int) SortColumn::Extension,  56, 40, -1,
        juce::TableHeaderComponent::defaultFlags | juce::TableHeaderComponent::sortable);
    table.getHeader().addColumn ("Size",    (int) SortColumn::Size,       80, 60, -1,
        juce::TableHeaderComponent::defaultFlags | juce::TableHeaderComponent::sortable);
    table.getHeader().addColumn ("Date",    (int) SortColumn::Date,      130, 90, -1,
        juce::TableHeaderComponent::defaultFlags | juce::TableHeaderComponent::sortable);
    table.getHeader().setSortColumnId ((int) sortCol, sortAscending);
    table.setMultipleSelectionEnabled (false);
    table.setColour (juce::ListBox::backgroundColourId, LF::kPanelBg);
    table.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);

    auto home = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    setDirectory (home);
}

FilePanel::~FilePanel() = default;

void FilePanel::setDirectory (const juce::File& dir)
{
    auto target = dir;
    if (! target.isDirectory())
        target = juce::File::getSpecialLocation (juce::File::userHomeDirectory);

    auto previousDir = currentDir;
    currentDir = target;
    loadDirectory();

    // When stepping up to the parent, place the cursor on the directory we came from
    // instead of the default first row.
    if (previousDir != currentDir && previousDir.getParentDirectory() == currentDir)
    {
        for (int i = 0; i < entries.size(); ++i)
        {
            if (entries.getReference (i).file == previousDir)
            {
                table.selectRow (i);
                break;
            }
        }
    }

    if (onDirChanged) onDirChanged (currentDir);
}

void FilePanel::loadDirectory()
{
    entries.clear();

    if (currentDir.getParentDirectory() != currentDir)
    {
        FileEntry up;
        up.file = currentDir.getParentDirectory();
        up.displayName = "..";
        up.isDirectory = true;
        up.isParentLink = true;
        entries.add (up);
    }

    juce::Array<juce::File> kids;
    currentDir.findChildFiles (kids,
                               juce::File::findFilesAndDirectories,
                               false);

    for (auto& f : kids)
    {
        FileEntry e;
        e.file = f;
        e.isDirectory = f.isDirectory();
        e.isSymlink = f.isSymbolicLink();
        e.isHidden = f.isHidden();
        e.displayName = f.getFileName();
        if (e.isDirectory)
        {
            e.size = -1;
        }
        else
        {
            e.size = f.getSize();
            e.extension = f.getFileExtension().trimCharactersAtStart (".").toLowerCase();
        }
        e.modTime = f.getLastModificationTime();
        entries.add (e);
    }

    sortEntries();
    table.updateContent();
    table.selectRow (0);
    updateHeader();
    updateFooter();
    repaint();
}

void FilePanel::sortEntries()
{
    auto cmp = [this] (const FileEntry& a, const FileEntry& b) -> int
    {
        if (a.isParentLink != b.isParentLink) return a.isParentLink ? -1 : 1;
        if (a.isDirectory != b.isDirectory) return a.isDirectory ? -1 : 1;

        int result = 0;
        switch (sortCol)
        {
            case SortColumn::Name:
                result = a.displayName.compareIgnoreCase (b.displayName);
                break;
            case SortColumn::Extension:
                result = a.extension.compareIgnoreCase (b.extension);
                if (result == 0) result = a.displayName.compareIgnoreCase (b.displayName);
                break;
            case SortColumn::Size:
                result = (a.size < b.size) ? -1 : (a.size > b.size ? 1 : 0);
                if (result == 0) result = a.displayName.compareIgnoreCase (b.displayName);
                break;
            case SortColumn::Date:
                result = (a.modTime < b.modTime) ? -1 : (a.modTime > b.modTime ? 1 : 0);
                if (result == 0) result = a.displayName.compareIgnoreCase (b.displayName);
                break;
        }
        return sortAscending ? result : -result;
    };

    struct Comparator
    {
        std::function<int (const FileEntry&, const FileEntry&)> fn;
        int compareElements (const FileEntry& a, const FileEntry& b) const { return fn (a, b); }
    };
    Comparator c { cmp };
    entries.sort (c, true);
}

int FilePanel::getNumRows() { return entries.size(); }

void FilePanel::paintRowBackground (juce::Graphics& g, int row, int width, int height, bool isSelected)
{
    if (! juce::isPositiveAndBelow (row, entries.size())) return;

    if (isSelected)
    {
        auto bg = active ? LF::kCursorBg : LF::kCursorBg.withSaturation (0.2f).darker (0.2f);
        juce::ColourGradient grad (bg.brighter (0.15f), 0.0f, 0.0f,
                                   bg.darker (0.1f), 0.0f, (float) height, false);
        g.setGradientFill (grad);
        g.fillRect (0, 0, width, height);

        if (active)
        {
            g.setColour (LF::kAccent);
            g.fillRect (0, 0, 3, height);
        }
    }
    else
    {
        g.setColour ((row % 2 == 0) ? LF::kPanelBg : LF::kPanelBgAlt);
        g.fillRect (0, 0, width, height);
    }
}

void FilePanel::paintCell (juce::Graphics& g, int row, int columnId,
                           int width, int height, bool isSelected)
{
    if (! juce::isPositiveAndBelow (row, entries.size())) return;
    const auto& e = entries.getReference (row);

    juce::Colour text;
    if (e.selected)        text = LF::kTextSelected;
    else if (e.isDirectory) text = LF::kTextDir;
    else                    text = LF::kTextNormal;

    if (e.isHidden) text = text.withAlpha (0.55f);
    if (isSelected && active) text = juce::Colours::white;

    g.setColour (text);
    g.setFont (e.isDirectory ? UF::getMonoBoldFont (13.0f) : UF::getMonoFont (13.0f));

    auto cell = juce::Rectangle<int> (width, height).reduced (6, 0);

    switch ((SortColumn) columnId)
    {
        case SortColumn::Name:
        {
            juce::String prefix;
            if (e.isParentLink)     prefix = "^ ";
            else if (e.isDirectory) prefix = "/ ";
            else                    prefix = "  ";
            g.drawText (prefix + e.displayName, cell, juce::Justification::centredLeft, true);
            break;
        }
        case SortColumn::Extension:
            g.drawText (e.isDirectory ? juce::String() : e.extension,
                        cell, juce::Justification::centredLeft, true);
            break;
        case SortColumn::Size:
            g.drawText (e.isDirectory ? juce::String ("<DIR>") : UF::formatSize (e.size),
                        cell, juce::Justification::centredRight, true);
            break;
        case SortColumn::Date:
            g.drawText (UF::formatTime (e.modTime),
                        cell, juce::Justification::centredRight, true);
            break;
    }

    // selection marker
    if (e.selected && columnId == (int) SortColumn::Name)
    {
        g.setColour (LF::kTextSelected);
        g.fillRect (cell.removeFromLeft (3));
    }
}

void FilePanel::cellClicked (int row, int /*col*/, const juce::MouseEvent&)
{
    table.selectRow (row);
    if (onPanelActivated) onPanelActivated();
}

void FilePanel::cellDoubleClicked (int row, int /*col*/, const juce::MouseEvent&)
{
    table.selectRow (row);
    enterCursor();
}

void FilePanel::backgroundClicked (const juce::MouseEvent&)
{
    if (onPanelActivated) onPanelActivated();
}

void FilePanel::sortOrderChanged (int newSortColumnId, bool isForwards)
{
    sortCol = (SortColumn) newSortColumnId;
    sortAscending = isForwards;
    auto cursor = getCursorFile();
    sortEntries();
    table.updateContent();
    // try to retain cursor
    for (int i = 0; i < entries.size(); ++i)
    {
        if (entries.getReference (i).file == cursor)
        {
            table.selectRow (i);
            break;
        }
    }
}

void FilePanel::returnKeyPressed (int /*lastRowSelected*/)
{
    enterCursor();
}

void FilePanel::enterCursor()
{
    auto row = table.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, entries.size())) return;
    const auto& e = entries.getReference (row);

    if (e.isDirectory)
        setDirectory (e.file);
    else if (onFileChosen)
        onFileChosen (e.file);
}

juce::File FilePanel::getCursorFile() const
{
    auto row = table.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, entries.size())) return {};
    return entries.getReference (row).file;
}

juce::Array<juce::File> FilePanel::getMarkedFiles() const
{
    juce::Array<juce::File> result;
    for (auto& e : entries)
        if (e.selected && ! e.isParentLink)
            result.add (e.file);
    return result;
}

juce::Array<juce::File> FilePanel::getActionFiles() const
{
    auto marked = getMarkedFiles();
    if (! marked.isEmpty()) return marked;

    auto row = table.getSelectedRow();
    if (juce::isPositiveAndBelow (row, entries.size()))
    {
        const auto& e = entries.getReference (row);
        if (! e.isParentLink)
            return { e.file };
    }
    return {};
}

void FilePanel::refresh()
{
    auto cursor = getCursorFile();
    auto originalRow = table.getSelectedRow();

    loadDirectory();

    // Try to keep the cursor on the same file.
    for (int i = 0; i < entries.size(); ++i)
    {
        if (entries.getReference (i).file == cursor)
        {
            table.selectRow (i);
            return;
        }
    }

    // Original file is gone (e.g. after delete/move) — stay at the same row index
    // so the next entry slides into the cursor position. Clamp to valid range.
    if (entries.size() > 0)
    {
        auto target = juce::jlimit (0, entries.size() - 1, originalRow);
        table.selectRow (target);
    }
}

void FilePanel::setActive (bool shouldBeActive)
{
    if (active == shouldBeActive) return;
    active = shouldBeActive;
    if (active && onPanelActivated) onPanelActivated();
    repaint();
}

void FilePanel::selectAll (bool select)
{
    for (auto& e : entries)
        if (! e.isParentLink && ! e.isDirectory)
            e.selected = select;
    updateFooter();
    table.repaint();
}

void FilePanel::invertSelection()
{
    for (auto& e : entries)
        if (! e.isParentLink && ! e.isDirectory)
            e.selected = ! e.selected;
    updateFooter();
    table.repaint();
}

void FilePanel::toggleSelectionAtCursor()
{
    auto row = table.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, entries.size())) return;
    auto& e = entries.getReference (row);
    if (e.isParentLink) return;
    e.selected = ! e.selected;
    if (row + 1 < entries.size()) table.selectRow (row + 1);
    updateFooter();
    table.repaint();
}

void FilePanel::selectByWildcard (const juce::String& pattern, bool select)
{
    auto pat = pattern.trim();
    if (pat.isEmpty()) pat = "*";

    for (auto& e : entries)
    {
        if (e.isParentLink || e.isDirectory) continue;
        if (e.displayName.matchesWildcard (pat, true))
            e.selected = select;
    }
    updateFooter();
    table.repaint();
}

void FilePanel::setSortColumn (SortColumn col, bool ascending)
{
    sortCol = col;
    sortAscending = ascending;
    table.getHeader().setSortColumnId ((int) col, ascending);
}

void FilePanel::updateHeader()
{
    pathLabel.setText (currentDir.getFullPathName(), juce::dontSendNotification);
}

void FilePanel::updateFooter()
{
    juce::int64 totalMarked = 0;
    int markedCount = 0;
    int totalFiles = 0;
    int totalDirs = 0;
    juce::int64 totalSize = 0;
    for (auto& e : entries)
    {
        if (e.isParentLink) continue;
        if (e.selected) { ++markedCount; totalMarked += juce::jmax<juce::int64> (0, e.size); }
        if (e.isDirectory) ++totalDirs;
        else { ++totalFiles; totalSize += e.size; }
    }

    juce::String left;
    if (markedCount > 0)
        left = juce::String (markedCount) + " marked  " + UF::formatSize (totalMarked);
    else
        left = juce::String (totalFiles) + " file(s)  " + juce::String (totalDirs) + " dir(s)  "
             + UF::formatSize (totalSize);

    statusLabel.setText (left, juce::dontSendNotification);

    auto free = currentDir.getBytesFreeOnVolume();
    auto vol  = currentDir.getVolumeTotalSize();
    juce::String right = UF::formatSize (free) + " free / " + UF::formatSize (vol);
    footerLabel.setText (right, juce::dontSendNotification);
}

void FilePanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    // Panel body
    g.setColour (LF::kPanelBg);
    g.fillRoundedRectangle (r, 8.0f);

    // Active glow
    if (active)
    {
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (LF::kAccent.withAlpha (0.07f * i));
            g.drawRoundedRectangle (r.reduced ((float) (4 - i) * 0.6f), 8.0f, 1.5f);
        }
        g.setColour (LF::kPanelBorderActive);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.5f);
    }
    else
    {
        g.setColour (LF::kPanelBorder);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);
    }

    // Title bar gradient strip
    auto strip = r.reduced (1.0f).withHeight (24.0f);
    juce::Path p;
    p.addRoundedRectangle (strip.getX(), strip.getY(), strip.getWidth(), strip.getHeight(),
                           8.0f, 8.0f, true, true, false, false);
    juce::ColourGradient stripGrad (active ? LF::kAccent.withAlpha (0.22f) : LF::kHeaderBg,
                                    strip.getX(), strip.getY(),
                                    active ? LF::kAccent.withAlpha (0.08f) : LF::kHeaderBg.darker (0.2f),
                                    strip.getX(), strip.getBottom(), false);
    g.setGradientFill (stripGrad);
    g.fillPath (p);

    // Separator under path
    g.setColour (active ? LF::kAccent.withAlpha (0.7f) : LF::kPanelBorder);
    g.drawLine (r.getX() + 6.0f, r.getY() + 48.0f, r.getRight() - 6.0f, r.getY() + 48.0f, 1.0f);

    // Separator above footer
    auto footerY = r.getBottom() - 22.0f;
    g.setColour (LF::kPanelBorder);
    g.drawLine (r.getX() + 6.0f, footerY, r.getRight() - 6.0f, footerY, 1.0f);
}

void FilePanel::resized()
{
    auto r = getLocalBounds().reduced (8, 8);

    // Title strip
    auto titleRow = r.removeFromTop (18);
    titleLabel.setBounds (titleRow);

    // Path
    auto pathRow = r.removeFromTop (22);
    pathLabel.setBounds (pathRow.reduced (4, 0));

    r.removeFromTop (4); // spacer for separator

    // Footer split into status (left) + free space (right)
    auto footerRow = r.removeFromBottom (20);
    auto leftHalf = footerRow.removeFromLeft (footerRow.getWidth() / 2);
    statusLabel.setBounds (leftHalf);
    footerLabel.setBounds (footerRow);

    r.removeFromBottom (2);

    table.setBounds (r);
}

#include "MainComponent.h"
#include "Utils.h"
#include "FileOperations.h"
#include "TextViewer.h"

using LF = UltraLookAndFeel;

MainComponent::MainComponent()
{
    juce::LookAndFeel::setDefaultLookAndFeel (&lf);

    addAndMakeVisible (titleLabel);
    titleLabel.setFont (UF::getMonoBoldFont (18.0f));
    titleLabel.setColour (juce::Label::textColourId, LF::kAccent);
    titleLabel.setText ("ULTRAFILE", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (subtitleLabel);
    subtitleLabel.setFont (UF::getMonoFont (11.0f));
    subtitleLabel.setColour (juce::Label::textColourId, LF::kTextDim);
    subtitleLabel.setText ("dual-pane file commander", juce::dontSendNotification);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (leftPanel);
    addAndMakeVisible (rightPanel);
    addAndMakeVisible (commandBar);

    leftPanel.onPanelActivated  = [this] { setActivePanel (&leftPanel); };
    rightPanel.onPanelActivated = [this] { setActivePanel (&rightPanel); };

    leftPanel.onFileChosen  = [this] (const juce::File&) { cmdView(); };
    rightPanel.onFileChosen = [this] (const juce::File&) { cmdView(); };

    commandBar.onCommand = [this] (int n) { handleFKey (n); };

    // Install key listener on tables so we intercept F-keys, Tab, Insert, etc.
    leftPanel.getTable().addKeyListener (this);
    rightPanel.getTable().addKeyListener (this);
    addKeyListener (this);

    setWantsKeyboardFocus (true);
    setActivePanel (&leftPanel);

    setSize (1280, 800);
}

MainComponent::~MainComponent()
{
    leftPanel.getTable().removeKeyListener (this);
    rightPanel.getTable().removeKeyListener (this);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

void MainComponent::paint (juce::Graphics& g)
{
    // Layered background
    auto r = getLocalBounds().toFloat();
    juce::ColourGradient grad (LF::kBackground.brighter (0.04f), r.getCentreX(), 0.0f,
                               LF::kBackground.darker (0.25f),   r.getCentreX(), r.getBottom(),
                               false);
    g.setGradientFill (grad);
    g.fillAll();

    // Subtle vertical scanlines
    g.setColour (juce::Colour (0xff0c1330).withAlpha (0.35f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, (float) getWidth());

    // Header underline
    g.setColour (LF::kAccent.withAlpha (0.35f));
    g.drawLine (8.0f, 44.0f, getWidth() - 8.0f, 44.0f, 1.0f);
}

void MainComponent::resized()
{
    auto r = getLocalBounds();

    // Top header
    auto header = r.removeFromTop (44);
    auto left = header.removeFromLeft (260).reduced (12, 6);
    titleLabel.setBounds (left.removeFromTop (24));
    subtitleLabel.setBounds (left);

    // Bottom command bar
    commandBar.setBounds (r.removeFromBottom (34));

    // Two panels side by side
    auto body = r.reduced (8, 6);
    int gap = 8;
    int halfW = (body.getWidth() - gap) / 2;
    leftPanel.setBounds (body.removeFromLeft (halfW));
    body.removeFromLeft (gap);
    rightPanel.setBounds (body);
}

void MainComponent::setActivePanel (FilePanel* p)
{
    if (p == nullptr || p == activePanel) {
        if (p != nullptr) p->getTable().grabKeyboardFocus();
        return;
    }
    if (activePanel) activePanel->setActive (false);
    activePanel = p;
    activePanel->setActive (true);
    activePanel->getTable().grabKeyboardFocus();
}

FilePanel* MainComponent::otherPanel (FilePanel* p) const
{
    return p == &leftPanel ? const_cast<FilePanel*> (&rightPanel)
                            : const_cast<FilePanel*> (&leftPanel);
}

void MainComponent::switchPanel()
{
    setActivePanel (activePanel == &leftPanel ? &rightPanel : &leftPanel);
}

void MainComponent::focusEditorAndSelectAll (juce::AlertWindow* aw,
                                             const juce::String& editorName,
                                             int acceptResult)
{
    if (aw == nullptr) return;
    auto* ed = aw->getTextEditor (editorName);
    if (ed == nullptr) return;

    juce::Component::SafePointer<juce::AlertWindow> safeAw (aw);
    ed->onEscapeKey = [safeAw] { if (auto* a = safeAw.getComponent()) a->exitModalState (0); };
    ed->onReturnKey = [safeAw, acceptResult] { if (auto* a = safeAw.getComponent()) a->exitModalState (acceptResult); };

    juce::Component::SafePointer<juce::TextEditor> safe (ed);
    juce::MessageManager::callAsync ([safe]
    {
        if (auto* e = safe.getComponent())
        {
            e->grabKeyboardFocus();
            e->selectAll();
        }
    });
}

void MainComponent::handleFKey (int n)
{
    switch (n)
    {
        case 1:  cmdHelp(); break;
        case 3:  cmdView(); break;
        case 4:  cmdEdit(); break;
        case 5:  cmdCopy(); break;
        case 6:  cmdMove(); break;
        case 7:  cmdMkDir(); break;
        case 8:  cmdDelete(); break;
        case 9:  cmdRename(); break;
        case 10: cmdQuit(); break;
        default: break;
    }
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    return keyPressed (key, this);
}

bool MainComponent::keyPressed (const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    if (key == juce::KeyPress::tabKey || key == juce::KeyPress (juce::KeyPress::tabKey, juce::ModifierKeys::shiftModifier, 0))
    {
        switchPanel();
        return true;
    }
    if (key == juce::KeyPress::F1Key)  { cmdHelp();   return true; }
    if (key == juce::KeyPress::F3Key)  { cmdView();   return true; }
    if (key == juce::KeyPress::F4Key)  { cmdEdit();   return true; }
    if (key == juce::KeyPress::F5Key)  { cmdCopy();   return true; }
    if (key == juce::KeyPress::F6Key)  { cmdMove();   return true; }
    if (key == juce::KeyPress::F7Key)  { cmdMkDir();  return true; }
    if (key == juce::KeyPress::F8Key || key == juce::KeyPress::deleteKey)
                                       { cmdDelete(); return true; }
    if (key == juce::KeyPress::F9Key)  { cmdRename(); return true; }
    if (key == juce::KeyPress::F10Key) { cmdQuit();   return true; }

    if (key.getKeyCode() == juce::KeyPress::insertKey
        || (key.getTextCharacter() == ' ' && ! key.getModifiers().isAnyModifierKeyDown()))
    {
        if (active()) active()->toggleSelectionAtCursor();
        return true;
    }

    // Norton-style group selection
    if (key.getKeyCode() == juce::KeyPress::numberPadMultiply
        || key.getTextCharacter() == '*')
    {
        if (active()) active()->invertSelection();
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::numberPadAdd
        || (key.getTextCharacter() == '+' && ! key.getModifiers().isCtrlDown()
                                          && ! key.getModifiers().isCommandDown()))
    {
        cmdSelectByWildcard (true);
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::numberPadSubtract
        || (key.getTextCharacter() == '-' && ! key.getModifiers().isCtrlDown()
                                          && ! key.getModifiers().isCommandDown()))
    {
        cmdSelectByWildcard (false);
        return true;
    }

    if (key == juce::KeyPress::backspaceKey)
    {
        if (active())
        {
            auto cur = active()->getDirectory();
            auto parent = cur.getParentDirectory();
            if (parent != cur) active()->setDirectory (parent);
        }
        return true;
    }

    if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown())
    {
        if (key.getKeyCode() == 'A') { if (active()) active()->selectAll (true); return true; }
        if (key.getKeyCode() == 'D') { if (active()) active()->selectAll (false); return true; }
        if (key.getKeyCode() == 'R') { if (active()) active()->refresh(); return true; }
        // Norton-style sort shortcuts
        if (key == juce::KeyPress (juce::KeyPress::F3Key, juce::ModifierKeys::ctrlModifier, 0))
        { if (active()) active()->setSortColumn (SortColumn::Name, true); return true; }
        if (key == juce::KeyPress (juce::KeyPress::F4Key, juce::ModifierKeys::ctrlModifier, 0))
        { if (active()) active()->setSortColumn (SortColumn::Extension, true); return true; }
        if (key == juce::KeyPress (juce::KeyPress::F5Key, juce::ModifierKeys::ctrlModifier, 0))
        { if (active()) active()->setSortColumn (SortColumn::Date, false); return true; }
        if (key == juce::KeyPress (juce::KeyPress::F6Key, juce::ModifierKeys::ctrlModifier, 0))
        { if (active()) active()->setSortColumn (SortColumn::Size, false); return true; }
    }

    return false;
}

//==============================================================================
// Commands
//==============================================================================

void MainComponent::cmdView()
{
    if (! active()) return;
    auto f = active()->getCursorFile();
    if (! f.existsAsFile()) return;
    TextViewerComponent::showFor (f, false, this);
}

void MainComponent::cmdEdit()
{
    if (! active()) return;
    auto f = active()->getCursorFile();
    if (f == juce::File()) return;
    if (! f.existsAsFile())
    {
        if (! f.create()) return;
    }
    TextViewerComponent::showFor (f, true, this);
}

void MainComponent::cmdCopy()
{
    if (! active()) return;
    auto files = active()->getActionFiles();
    if (files.isEmpty()) return;

    auto* dest = otherPanel (active());
    juce::String msg = files.size() == 1
        ? "Copy '" + files.getFirst().getFileName() + "' to:\n" + dest->getDirectory().getFullPathName()
        : "Copy " + juce::String (files.size()) + " items to:\n" + dest->getDirectory().getFullPathName();

    auto* aw = new juce::AlertWindow ("Copy", msg, juce::AlertWindow::NoIcon);
    aw->addTextEditor ("dest", dest->getDirectory().getFullPathName(), "Destination:");
    aw->addButton ("Copy", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, files] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result != 1) return;

            juce::File destDir (aw->getTextEditorContents ("dest"));
            if (! destDir.isDirectory())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Copy", "Destination is not a directory:\n" + destDir.getFullPathName());
                return;
            }

            juce::StringArray errors;
            FileOps::runTransfer (FileOps::Mode::Copy, files, destDir, errors);

            if (! errors.isEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Copy completed with errors", errors.joinIntoString ("\n"));

            leftPanel.refresh();
            rightPanel.refresh();
        }), false);
    focusEditorAndSelectAll (aw, "dest");
}

void MainComponent::cmdMove()
{
    if (! active()) return;
    auto files = active()->getActionFiles();
    if (files.isEmpty()) return;

    auto* dest = otherPanel (active());
    juce::String msg = files.size() == 1
        ? "Move '" + files.getFirst().getFileName() + "' to:\n" + dest->getDirectory().getFullPathName()
        : "Move " + juce::String (files.size()) + " items to:\n" + dest->getDirectory().getFullPathName();

    auto* aw = new juce::AlertWindow ("Move", msg, juce::AlertWindow::NoIcon);
    aw->addTextEditor ("dest", dest->getDirectory().getFullPathName(), "Destination:");
    aw->addButton ("Move", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, files] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result != 1) return;

            juce::File destDir (aw->getTextEditorContents ("dest"));
            if (! destDir.isDirectory())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Move", "Destination is not a directory:\n" + destDir.getFullPathName());
                return;
            }

            juce::StringArray errors;
            FileOps::runTransfer (FileOps::Mode::Move, files, destDir, errors);

            if (! errors.isEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Move completed with errors", errors.joinIntoString ("\n"));

            leftPanel.refresh();
            rightPanel.refresh();
        }), false);
    focusEditorAndSelectAll (aw, "dest");
}

void MainComponent::cmdMkDir()
{
    if (! active()) return;
    auto here = active()->getDirectory();

    auto* aw = new juce::AlertWindow ("Make Directory",
        "Create new directory in:\n" + here.getFullPathName(),
        juce::AlertWindow::NoIcon);
    aw->addTextEditor ("name", "", "Name:");
    aw->addButton ("Create", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, here] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result != 1) return;

            auto name = aw->getTextEditorContents ("name").trim();
            if (name.isEmpty()) return;

            auto target = here.getChildFile (name);
            auto res = target.createDirectory();
            if (! res.wasOk())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Make Directory failed", res.getErrorMessage());
                return;
            }
            leftPanel.refresh();
            rightPanel.refresh();
            if (active())
            {
                // place cursor on the new directory
                active()->refresh();
            }
        }), false);
    focusEditorAndSelectAll (aw, "name");
}

void MainComponent::cmdDelete()
{
    if (! active()) return;
    auto items = active()->getActionFiles();
    if (items.isEmpty()) return;

    juce::String msg;
    if (items.size() == 1)
        msg = "Delete '" + items.getFirst().getFileName() + "'?";
    else
        msg = "Delete " + juce::String (items.size()) + " items?";

    bool anyDir = false;
    for (auto& it : items) if (it.isDirectory()) { anyDir = true; break; }
    if (anyDir) msg += "\nWARNING: includes directories (recursive delete)";

    auto* aw = new juce::AlertWindow ("Delete", msg, juce::AlertWindow::WarningIcon);
    aw->addButton ("Move to Trash", 2, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Delete permanently", 1);
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, items] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result == 0) return;

            bool trash = (result == 2);
            juce::StringArray errors;
            FileOps::runDelete (items, trash, errors);

            if (! errors.isEmpty())
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Delete completed with errors", errors.joinIntoString ("\n"));

            leftPanel.refresh();
            rightPanel.refresh();
        }), false);
}

void MainComponent::cmdRename()
{
    if (! active()) return;
    auto f = active()->getCursorFile();
    if (f == juce::File()) return;
    if (f.getFileName() == "..") return;

    auto* aw = new juce::AlertWindow ("Rename",
        "Rename '" + f.getFileName() + "' in:\n" + f.getParentDirectory().getFullPathName(),
        juce::AlertWindow::NoIcon);
    aw->addTextEditor ("name", f.getFileName(), "New name:");
    aw->addButton ("Rename", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, f] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result != 1) return;

            auto newName = aw->getTextEditorContents ("name").trim();
            if (newName.isEmpty() || newName == f.getFileName()) return;

            auto target = f.getParentDirectory().getChildFile (newName);
            if (target.exists())
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Rename", "Target already exists:\n" + target.getFullPathName());
                return;
            }
            if (! f.moveFileTo (target))
            {
                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                    "Rename failed", "Could not rename file.");
                return;
            }
            leftPanel.refresh();
            rightPanel.refresh();
        }), false);
    focusEditorAndSelectAll (aw, "name");
}

void MainComponent::cmdHelp()
{
    juce::String help;
    help << "ULTRAFILE  -  Norton-style dual-pane file manager\n\n"
         << "  Tab / Shift+Tab   switch active panel\n"
         << "  Enter             enter directory / open file\n"
         << "  Backspace         parent directory\n"
         << "  Insert / Space    mark/unmark file\n"
         << "  *                 invert selection\n"
         << "  +                 select by wildcard (e.g. *.txt)\n"
         << "  -                 unselect by wildcard\n"
         << "  Cmd/Ctrl+A        mark all     Cmd/Ctrl+D unmark all\n"
         << "  Cmd/Ctrl+R        refresh\n\n"
         << "  F3   View         F4   Edit\n"
         << "  F5   Copy         F6   Move\n"
         << "  F7   MkDir        F8/Del Delete\n"
         << "  F9   Rename       F10  Quit\n\n"
         << "  Click a column header to sort.\n"
         << "  Cmd/Ctrl + F3/F4/F5/F6 = sort by name/ext/date/size.";

    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::NoIcon, "Help", help, "OK");
}

void MainComponent::cmdQuit()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void MainComponent::cmdSelectByWildcard (bool select)
{
    if (! active()) return;

    auto* aw = new juce::AlertWindow (select ? "Select group" : "Unselect group",
                                       select ? "Select files matching pattern:"
                                              : "Unselect files matching pattern:",
                                       juce::AlertWindow::NoIcon);
    aw->addTextEditor ("pattern", "*", "Wildcard:");
    aw->addButton (select ? "Select" : "Unselect", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw, select] (int result)
        {
            std::unique_ptr<juce::AlertWindow> owner (aw);
            if (result != 1) return;
            auto pattern = aw->getTextEditorContents ("pattern");
            if (active()) active()->selectByWildcard (pattern, select);
        }), false);

    focusEditorAndSelectAll (aw, "pattern");
}

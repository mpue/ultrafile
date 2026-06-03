#pragma once

#include <JuceHeader.h>

class TextViewerComponent : public juce::Component,
                            public juce::KeyListener
{
public:
    /** Opens the given file in a modal DialogWindow.
        @param editable  false for F3 (view), true for F4 (edit). */
    static void showFor (const juce::File& file, bool editable, juce::Component* parentForCentering);

    TextViewerComponent (const juce::File& file, bool editable);
    ~TextViewerComponent() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    bool keyPressed (const juce::KeyPress&) override;
    bool keyPressed (const juce::KeyPress&, juce::Component*) override;

private:
    void save();
    void promptCloseIfDirty (std::function<void(bool)> onDecision);

    juce::File file;
    bool editable;
    juce::TextEditor editor;
    juce::Label header;
    juce::TextButton saveButton { "Save (F2)" };
    juce::TextButton closeButton { "Close (Esc)" };
    bool dirty = false;
};

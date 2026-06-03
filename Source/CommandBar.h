#pragma once

#include <JuceHeader.h>

class CommandBar : public juce::Component
{
public:
    CommandBar();

    /** Called with the F-key number (1..10) when a button is clicked. */
    std::function<void (int)> onCommand;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct FKey
    {
        int number;
        juce::String label;
        std::unique_ptr<juce::TextButton> button;
    };
    juce::Array<FKey*> keys;
    juce::OwnedArray<FKey> ownedKeys;
};

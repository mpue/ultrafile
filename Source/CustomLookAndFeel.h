#pragma once

#include <JuceHeader.h>

class UltraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Palette — deep midnight blue with cyan accents
    static const juce::Colour kBackground;       // window bg
    static const juce::Colour kPanelBg;          // panel background
    static const juce::Colour kPanelBgAlt;       // alternating row
    static const juce::Colour kPanelBorder;      // inactive border
    static const juce::Colour kPanelBorderActive;// active border (glowing cyan)
    static const juce::Colour kAccent;           // cyan
    static const juce::Colour kAccentDim;
    static const juce::Colour kTextNormal;
    static const juce::Colour kTextDir;          // directory entries
    static const juce::Colour kTextSelected;     // marked files (yellow)
    static const juce::Colour kTextDim;
    static const juce::Colour kCursorBg;         // current row
    static const juce::Colour kHeaderBg;
    static const juce::Colour kFooterBg;
    static const juce::Colour kButtonBg;
    static const juce::Colour kButtonBgHover;
    static const juce::Colour kFKeyNumber;       // amber

    UltraLookAndFeel();

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour&, bool isOver, bool isDown) override;
    void drawScrollbar (juce::Graphics&, juce::ScrollBar&,
                        int x, int y, int width, int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;
    void drawTableHeaderBackground (juce::Graphics&, juce::TableHeaderComponent&) override;
    void drawTableHeaderColumn (juce::Graphics&, juce::TableHeaderComponent&,
                                const juce::String& columnName,
                                int columnId, int width, int height,
                                bool isMouseOver, bool isMouseDown, int columnFlags) override;
    void drawAlertBox (juce::Graphics&, juce::AlertWindow&,
                       const juce::Rectangle<int>& textArea, juce::TextLayout&) override;
    void drawProgressBar (juce::Graphics&, juce::ProgressBar&,
                          int width, int height, double progress,
                          const juce::String& textToShow) override;
};

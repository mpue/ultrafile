#include "CustomLookAndFeel.h"
#include "Utils.h"

using juce::Colour;

const Colour UltraLookAndFeel::kBackground        { 0xff0a0e1a };
const Colour UltraLookAndFeel::kPanelBg           { 0xff111733 };
const Colour UltraLookAndFeel::kPanelBgAlt        { 0xff141a3a };
const Colour UltraLookAndFeel::kPanelBorder       { 0xff2a3358 };
const Colour UltraLookAndFeel::kPanelBorderActive { 0xff00e5ff };
const Colour UltraLookAndFeel::kAccent            { 0xff00e5ff };
const Colour UltraLookAndFeel::kAccentDim         { 0xff2599ad };
const Colour UltraLookAndFeel::kTextNormal        { 0xffe6ecff };
const Colour UltraLookAndFeel::kTextDir           { 0xffffffff };
const Colour UltraLookAndFeel::kTextSelected      { 0xffffd24a };
const Colour UltraLookAndFeel::kTextDim           { 0xff8794b8 };
const Colour UltraLookAndFeel::kCursorBg          { 0xff1f4dff };
const Colour UltraLookAndFeel::kHeaderBg          { 0xff1a2247 };
const Colour UltraLookAndFeel::kFooterBg          { 0xff0d1228 };
const Colour UltraLookAndFeel::kButtonBg          { 0xff1a2247 };
const Colour UltraLookAndFeel::kButtonBgHover     { 0xff253067 };
const Colour UltraLookAndFeel::kFKeyNumber        { 0xffffb84d };

UltraLookAndFeel::UltraLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, kBackground);
    setColour (juce::DocumentWindow::backgroundColourId, kBackground);
    setColour (juce::Label::textColourId, kTextNormal);
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    setColour (juce::TextEditor::backgroundColourId, kPanelBg);
    setColour (juce::TextEditor::textColourId, kTextNormal);
    setColour (juce::TextEditor::highlightColourId, kAccent.withAlpha (0.4f));
    setColour (juce::TextEditor::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextEditor::outlineColourId, kPanelBorder);
    setColour (juce::TextEditor::focusedOutlineColourId, kAccent);
    setColour (juce::CaretComponent::caretColourId, kAccent);

    setColour (juce::TextButton::buttonColourId, kButtonBg);
    setColour (juce::TextButton::buttonOnColourId, kAccent);
    setColour (juce::TextButton::textColourOffId, kTextNormal);
    setColour (juce::TextButton::textColourOnId, kBackground);

    setColour (juce::ComboBox::backgroundColourId, kPanelBg);
    setColour (juce::ComboBox::textColourId, kTextNormal);
    setColour (juce::ComboBox::outlineColourId, kPanelBorder);
    setColour (juce::ComboBox::arrowColourId, kAccent);
    setColour (juce::ComboBox::buttonColourId, kPanelBg);

    setColour (juce::PopupMenu::backgroundColourId, kPanelBg);
    setColour (juce::PopupMenu::textColourId, kTextNormal);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kCursorBg);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    setColour (juce::TableHeaderComponent::backgroundColourId, kHeaderBg);
    setColour (juce::TableHeaderComponent::outlineColourId, kPanelBorder);
    setColour (juce::TableHeaderComponent::textColourId, kAccent);
    setColour (juce::TableHeaderComponent::highlightColourId, kCursorBg);

    setColour (juce::ListBox::backgroundColourId, kPanelBg);
    setColour (juce::ListBox::outlineColourId, kPanelBorder);
    setColour (juce::ListBox::textColourId, kTextNormal);

    setColour (juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId, kAccentDim);
    setColour (juce::ScrollBar::trackColourId, kPanelBgAlt);

    setColour (juce::AlertWindow::backgroundColourId, kPanelBg);
    setColour (juce::AlertWindow::textColourId, kTextNormal);
    setColour (juce::AlertWindow::outlineColourId, kAccent);

    setColour (juce::ProgressBar::backgroundColourId, kPanelBgAlt);
    setColour (juce::ProgressBar::foregroundColourId, kAccent);

    setDefaultSansSerifTypefaceName (UF::getMonospaceFontName());
}

juce::Font UltraLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return UF::getMonoBoldFont (juce::jmin (15.0f, buttonHeight * 0.55f));
}

void UltraLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                             const juce::Colour& bg, bool isOver, bool isDown)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    auto base = isDown ? kAccent.withAlpha (0.85f)
                       : (isOver ? kButtonBgHover : bg);

    juce::ColourGradient grad (base.brighter (0.12f),
                               r.getCentreX(), r.getY(),
                               base.darker (0.15f),
                               r.getCentreX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 4.0f);

    g.setColour (isDown ? kAccent : kPanelBorder);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    if (isOver && ! isDown)
    {
        g.setColour (kAccent.withAlpha (0.35f));
        g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);
    }
}

void UltraLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar&,
                                      int x, int y, int width, int height,
                                      bool isVertical,
                                      int thumbStart, int thumbSize,
                                      bool isOver, bool /*isDown*/)
{
    g.setColour (kPanelBgAlt);
    g.fillRect (x, y, width, height);

    auto r = isVertical ? juce::Rectangle<int> (x + 2, thumbStart, width - 4, thumbSize)
                        : juce::Rectangle<int> (thumbStart, y + 2, thumbSize, height - 4);

    g.setColour (isOver ? kAccent : kAccentDim);
    g.fillRoundedRectangle (r.toFloat(), 2.5f);
}

void UltraLookAndFeel::drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& h)
{
    auto r = h.getLocalBounds();
    juce::ColourGradient grad (kHeaderBg.brighter (0.1f),
                               0.0f, (float) r.getY(),
                               kHeaderBg.darker (0.2f),
                               0.0f, (float) r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRect (r);

    g.setColour (kAccent);
    g.fillRect (r.removeFromBottom (1));
}

void UltraLookAndFeel::drawTableHeaderColumn (juce::Graphics& g, juce::TableHeaderComponent& header,
                                              const juce::String& columnName,
                                              int /*columnId*/, int width, int height,
                                              bool isOver, bool /*isDown*/, int columnFlags)
{
    auto area = juce::Rectangle<int> (width, height).reduced (4, 0);

    g.setColour (isOver ? kAccent.brighter (0.2f) : kAccent);
    g.setFont (UF::getMonoBoldFont (12.5f));

    juce::String arrow;
    if (columnFlags & (juce::TableHeaderComponent::sortedForwards
                     | juce::TableHeaderComponent::sortedBackwards))
    {
        arrow = (columnFlags & juce::TableHeaderComponent::sortedForwards) ? "  v" : "  ^";
    }

    g.drawText (columnName.toUpperCase() + arrow, area,
                juce::Justification::centredLeft, true);

    // divider
    g.setColour (kPanelBorder);
    g.drawLine ((float) width - 0.5f, 4.0f, (float) width - 0.5f, (float) height - 4.0f);
    juce::ignoreUnused (header);
}

void UltraLookAndFeel::drawAlertBox (juce::Graphics& g, juce::AlertWindow& w,
                                     const juce::Rectangle<int>& textArea,
                                     juce::TextLayout& layout)
{
    auto bounds = w.getLocalBounds().toFloat();

    g.setColour (kPanelBg);
    g.fillRoundedRectangle (bounds, 8.0f);

    g.setColour (kAccent);
    g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 1.5f);

    // accent strip at top
    g.setColour (kAccent.withAlpha (0.18f));
    g.fillRoundedRectangle (bounds.withHeight (28.0f), 8.0f);

    g.setColour (kAccent);
    g.setFont (UF::getMonoBoldFont (13.0f));
    g.drawText (w.getName().toUpperCase(),
                bounds.withHeight (28.0f).reduced (14, 0),
                juce::Justification::centredLeft, true);

    g.setColour (kTextNormal);
    layout.draw (g, textArea.toFloat());
}

void UltraLookAndFeel::drawProgressBar (juce::Graphics& g, juce::ProgressBar& bar,
                                        int width, int height, double progress,
                                        const juce::String& textToShow)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);

    g.setColour (kPanelBgAlt);
    g.fillRoundedRectangle (bounds, 4.0f);

    if (progress >= 0.0 && progress <= 1.0)
    {
        auto filled = bounds.withWidth ((float) (width * progress));
        juce::ColourGradient grad (kAccent.brighter (0.2f), filled.getX(), 0.0f,
                                   kAccent.darker (0.1f), filled.getRight(), 0.0f, false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (filled, 4.0f);
    }
    else
    {
        // indeterminate
        auto t = (float) (juce::Time::getMillisecondCounter() % 1500) / 1500.0f;
        auto sweep = bounds.withWidth (width * 0.35f).translated ((width + width * 0.35f) * t - width * 0.35f, 0);
        g.setColour (kAccent.withAlpha (0.6f));
        g.fillRoundedRectangle (sweep, 4.0f);
    }

    g.setColour (kPanelBorder);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    g.setColour (juce::Colours::white);
    g.setFont (UF::getMonoBoldFont (height * 0.55f));
    g.drawText (textToShow, bounds.toNearestInt(), juce::Justification::centred, true);

    juce::ignoreUnused (bar);
}

#pragma once

#include <JuceHeader.h>

namespace UF
{
    inline juce::String formatSize (juce::int64 bytes)
    {
        if (bytes < 0) return "<DIR>";
        if (bytes < 1024) return juce::String (bytes) + " B";
        const char* units[] = { "K", "M", "G", "T", "P" };
        double v = (double) bytes / 1024.0;
        int u = 0;
        while (v >= 1024.0 && u < 4) { v /= 1024.0; ++u; }
        return juce::String (v, v < 10.0 ? 2 : (v < 100.0 ? 1 : 0)) + " " + units[u];
    }

    inline juce::String formatSizeFull (juce::int64 bytes)
    {
        // 1,234,567 bytes
        juce::String s (bytes);
        juce::String out;
        int c = 0;
        for (int i = s.length() - 1; i >= 0; --i)
        {
            out = juce::String::charToString (s[i]) + out;
            if (++c % 3 == 0 && i > 0) out = "," + out;
        }
        return out;
    }

    inline juce::String formatTime (const juce::Time& t)
    {
        if (t.toMilliseconds() == 0) return "          ";
        return t.formatted ("%Y-%m-%d %H:%M");
    }

    inline juce::String getMonospaceFontName()
    {
       #if JUCE_MAC
        return "Menlo";
       #elif JUCE_WINDOWS
        return "Consolas";
       #else
        return juce::Font::getDefaultMonospacedFontName();
       #endif
    }

    inline juce::Font getMonoFont (float size)
    {
        return juce::Font (juce::FontOptions (getMonospaceFontName(), size, juce::Font::plain));
    }

    inline juce::Font getMonoBoldFont (float size)
    {
        return juce::Font (juce::FontOptions (getMonospaceFontName(), size, juce::Font::bold));
    }

}

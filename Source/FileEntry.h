#pragma once

#include <JuceHeader.h>

struct FileEntry
{
    juce::File file;
    juce::String displayName;
    juce::String extension;
    juce::int64 size = 0;
    juce::Time modTime;
    bool isDirectory = false;
    bool isParentLink = false;
    bool isHidden = false;
    bool isSymlink = false;
    bool selected = false;
};

enum class SortColumn
{
    Name = 1,
    Extension = 2,
    Size = 3,
    Date = 4
};

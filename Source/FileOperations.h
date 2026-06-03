#pragma once

#include <JuceHeader.h>

namespace FileOps
{
    enum class Mode { Copy, Move };

    /** Recursively counts files and total bytes under the given paths. Directories themselves
        contribute 0 bytes but +1 file count for progress accounting. */
    void measure (const juce::Array<juce::File>& roots,
                  juce::int64& outBytes, int& outFileCount);

    /** Runs a copy or move with a JUCE ThreadWithProgressWindow.
        Returns true if all operations completed without error.
        Errors are collected in outErrors. */
    bool runTransfer (Mode mode,
                      const juce::Array<juce::File>& sources,
                      const juce::File& destinationDir,
                      juce::StringArray& outErrors);

    /** Runs a recursive delete with progress. If moveToTrash is true, items go to trash;
        otherwise they are permanently deleted. */
    bool runDelete (const juce::Array<juce::File>& items,
                    bool moveToTrash,
                    juce::StringArray& outErrors);
}

#include "FileOperations.h"

namespace FileOps
{

void measure (const juce::Array<juce::File>& roots,
              juce::int64& outBytes, int& outFileCount)
{
    outBytes = 0;
    outFileCount = 0;
    for (auto& root : roots)
    {
        if (! root.exists()) continue;

        if (root.isDirectory())
        {
            ++outFileCount; // directory itself
            juce::Array<juce::File> kids;
            root.findChildFiles (kids, juce::File::findFilesAndDirectories, true);
            for (auto& k : kids)
            {
                ++outFileCount;
                if (! k.isDirectory()) outBytes += k.getSize();
            }
        }
        else
        {
            ++outFileCount;
            outBytes += root.getSize();
        }
    }
}

namespace
{
    juce::File uniqueDestination (const juce::File& destDir, const juce::String& fileName)
    {
        auto candidate = destDir.getChildFile (fileName);
        if (! candidate.exists()) return candidate;

        auto stem = candidate.getFileNameWithoutExtension();
        auto ext  = candidate.getFileExtension();
        for (int i = 1; i < 10000; ++i)
        {
            auto c = destDir.getChildFile (stem + " (" + juce::String (i) + ")" + ext);
            if (! c.exists()) return c;
        }
        return candidate;
    }

    bool copyDirectoryRecursive (const juce::File& source, const juce::File& dest,
                                 juce::ThreadWithProgressWindow& thread,
                                 juce::int64 totalBytes, juce::int64& bytesDone,
                                 int totalFiles, int& filesDone,
                                 juce::StringArray& errors)
    {
        if (thread.threadShouldExit()) return false;

        if (! dest.createDirectory().wasOk())
        {
            errors.add ("mkdir failed: " + dest.getFullPathName());
            return false;
        }
        ++filesDone;
        thread.setProgress (totalBytes > 0
            ? juce::jlimit (0.0, 1.0, (double) bytesDone / (double) totalBytes)
            : juce::jlimit (0.0, 1.0, (double) filesDone / (double) totalFiles));
        thread.setStatusMessage ("Creating " + dest.getFileName());

        juce::Array<juce::File> kids;
        source.findChildFiles (kids, juce::File::findFilesAndDirectories, false);
        bool ok = true;
        for (auto& k : kids)
        {
            if (thread.threadShouldExit()) return false;
            auto target = dest.getChildFile (k.getFileName());
            if (k.isDirectory())
            {
                ok &= copyDirectoryRecursive (k, target, thread,
                                              totalBytes, bytesDone,
                                              totalFiles, filesDone, errors);
            }
            else
            {
                thread.setStatusMessage ("Copying " + k.getFileName());
                if (! k.copyFileTo (target))
                {
                    errors.add ("copy failed: " + k.getFullPathName());
                    ok = false;
                }
                bytesDone += k.getSize();
                ++filesDone;
                thread.setProgress (totalBytes > 0
                    ? juce::jlimit (0.0, 1.0, (double) bytesDone / (double) totalBytes)
                    : juce::jlimit (0.0, 1.0, (double) filesDone / (double) totalFiles));
            }
        }
        return ok;
    }

    class TransferThread : public juce::ThreadWithProgressWindow
    {
    public:
        TransferThread (Mode m,
                        juce::Array<juce::File> srcs,
                        juce::File dst,
                        juce::StringArray& errs)
            : ThreadWithProgressWindow (m == Mode::Copy ? "Copying files" : "Moving files", true, true),
              mode (m), sources (std::move (srcs)), dest (dst), errors (errs)
        {
            setStatusMessage ("Calculating size...");
        }

        void run() override
        {
            juce::int64 totalBytes = 0;
            int totalFiles = 0;
            FileOps::measure (sources, totalBytes, totalFiles);
            if (totalFiles == 0) { success = true; return; }

            juce::int64 bytesDone = 0;
            int filesDone = 0;

            for (auto& src : sources)
            {
                if (threadShouldExit()) { success = false; return; }

                auto target = uniqueDestination (dest, src.getFileName());

                if (mode == Mode::Move)
                {
                    setStatusMessage ("Moving " + src.getFileName());
                    if (src.moveFileTo (target))
                    {
                        if (src.isDirectory())
                        {
                            // count contents as done
                            juce::Array<juce::File> kids;
                            target.findChildFiles (kids, juce::File::findFilesAndDirectories, true);
                            filesDone += kids.size() + 1;
                            for (auto& k : kids)
                                if (! k.isDirectory()) bytesDone += k.getSize();
                        }
                        else
                        {
                            bytesDone += target.getSize();
                            ++filesDone;
                        }
                        setProgress (totalBytes > 0
                            ? juce::jlimit (0.0, 1.0, (double) bytesDone / (double) totalBytes)
                            : juce::jlimit (0.0, 1.0, (double) filesDone / (double) totalFiles));
                        continue;
                    }
                    // moveFileTo failed (cross-volume) — fall back to copy+delete
                }

                bool ok;
                if (src.isDirectory())
                {
                    ok = copyDirectoryRecursive (src, target, *this,
                                                 totalBytes, bytesDone,
                                                 totalFiles, filesDone, errors);
                }
                else
                {
                    setStatusMessage ("Copying " + src.getFileName());
                    ok = src.copyFileTo (target);
                    if (! ok) errors.add ("copy failed: " + src.getFullPathName());
                    bytesDone += src.getSize();
                    ++filesDone;
                    setProgress (totalBytes > 0
                        ? juce::jlimit (0.0, 1.0, (double) bytesDone / (double) totalBytes)
                        : juce::jlimit (0.0, 1.0, (double) filesDone / (double) totalFiles));
                }

                if (ok && mode == Mode::Move)
                {
                    setStatusMessage ("Removing source " + src.getFileName());
                    if (src.isDirectory())
                    {
                        if (! src.deleteRecursively())
                            errors.add ("delete-source failed: " + src.getFullPathName());
                    }
                    else
                    {
                        if (! src.deleteFile())
                            errors.add ("delete-source failed: " + src.getFullPathName());
                    }
                }

                if (! ok) success = false;
            }
        }

        bool wasSuccessful() const { return success && ! threadShouldExit(); }

    private:
        Mode mode;
        juce::Array<juce::File> sources;
        juce::File dest;
        juce::StringArray& errors;
        bool success = true;
    };

    class DeleteThread : public juce::ThreadWithProgressWindow
    {
    public:
        DeleteThread (juce::Array<juce::File> its, bool trash, juce::StringArray& errs)
            : ThreadWithProgressWindow ("Deleting", true, true),
              items (std::move (its)), useTrash (trash), errors (errs)
        {
            setStatusMessage ("Counting items...");
        }

        void run() override
        {
            juce::int64 totalBytes = 0;
            int totalFiles = 0;
            FileOps::measure (items, totalBytes, totalFiles);
            if (totalFiles == 0) return;

            int done = 0;
            for (auto& item : items)
            {
                if (threadShouldExit()) return;
                setStatusMessage ((useTrash ? "Trashing " : "Deleting ") + item.getFileName());

                bool ok = false;
                if (useTrash)
                {
                    ok = item.moveToTrash();
                }
                else
                {
                    ok = item.isDirectory() ? item.deleteRecursively() : item.deleteFile();
                }
                if (! ok) errors.add ("delete failed: " + item.getFullPathName());

                // approximate progress per top-level item
                done += 1;
                setProgress ((double) done / (double) items.size());
            }
        }

    private:
        juce::Array<juce::File> items;
        bool useTrash;
        juce::StringArray& errors;
    };
}

bool runTransfer (Mode mode,
                  const juce::Array<juce::File>& sources,
                  const juce::File& destinationDir,
                  juce::StringArray& outErrors)
{
    if (sources.isEmpty()) return true;
    if (! destinationDir.isDirectory())
    {
        outErrors.add ("destination is not a directory: " + destinationDir.getFullPathName());
        return false;
    }

    TransferThread t (mode, sources, destinationDir, outErrors);
    t.runThread();
    return t.wasSuccessful() && outErrors.isEmpty();
}

bool runDelete (const juce::Array<juce::File>& items, bool moveToTrash, juce::StringArray& outErrors)
{
    if (items.isEmpty()) return true;
    DeleteThread t (items, moveToTrash, outErrors);
    t.runThread();
    return outErrors.isEmpty();
}

} // namespace FileOps

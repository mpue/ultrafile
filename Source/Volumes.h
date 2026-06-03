#pragma once

#include <JuceHeader.h>

#if JUCE_WINDOWS
 #include <windows.h>
 #include <winioctl.h>
#endif

//==============================================================================
/** Cross-platform volume / drive enumeration and removable-media ejection.

    - Windows : every file-system root (A:, C:, D: ...) becomes a volume; the
                drive type decides whether it is ejectable.
    - macOS   : the boot volume plus every mount under /Volumes.
    - Linux   : every file-system root plus user mounts under /media and
                /run/media/<user>.

    All functions are header-only and only ever called from a single
    translation unit, so the inline definitions are ODR-safe.
*/
namespace UF
{
    struct VolumeInfo
    {
        juce::String name;            // display label, e.g. "C: System" or "USB STICK"
        juce::File   root;            // mount point / drive root
        bool         isRemovable = false; // USB stick, card reader, optical, ...
        bool         isNetwork   = false; // network share / remote mount
        bool         ejectable   = false; // show an eject affordance for this entry
    };

    //==============================================================================
   #if JUCE_MAC

    inline juce::Array<VolumeInfo> getVolumes()
    {
        juce::Array<VolumeInfo> vols;

        // Boot volume
        {
            VolumeInfo v;
            v.root = juce::File ("/");
            v.name = v.root.getVolumeLabel();
            if (v.name.isEmpty()) v.name = "Macintosh HD";
            vols.add (v);
        }

        // Everything mounted under /Volumes
        juce::Array<juce::File> mounts;
        juce::File ("/Volumes").findChildFiles (mounts, juce::File::findDirectories, false);
        for (auto& m : mounts)
        {
            // The boot volume is also exposed as a symlink in /Volumes — skip it.
            if (m.isSymbolicLink() && m.getLinkedTarget() == juce::File ("/"))
                continue;

            VolumeInfo v;
            v.root        = m;
            v.name        = m.getFileName();
            v.isRemovable = true;
            v.ejectable   = true;
            vols.add (v);
        }

        return vols;
    }

    inline bool ejectVolume (const VolumeInfo& vol, juce::String& errorOut)
    {
        juce::ChildProcess proc;
        if (! proc.start (juce::StringArray { "/usr/sbin/diskutil", "eject",
                                              vol.root.getFullPathName() }))
        {
            errorOut = "Could not launch diskutil.";
            return false;
        }

        auto output = proc.readAllProcessOutput();
        proc.waitForProcessToFinish (15000);

        if (proc.getExitCode() != 0)
        {
            errorOut = output.trim().isNotEmpty() ? output.trim()
                                                  : "diskutil reported an error.";
            return false;
        }
        return true;
    }

    //==============================================================================
   #elif JUCE_WINDOWS

    inline juce::Array<VolumeInfo> getVolumes()
    {
        juce::Array<VolumeInfo> vols;

        juce::Array<juce::File> roots;
        juce::File::findFileSystemRoots (roots);

        for (auto& r : roots)
        {
            VolumeInfo v;
            v.root = r;

            auto path   = r.getFullPathName();        // "C:\"
            auto letter = path.substring (0, 2);      // "C:"
            auto label  = r.getVolumeLabel();
            v.name = label.isNotEmpty() ? (letter + " " + label) : letter;

            switch (GetDriveTypeW (path.toWideCharPointer()))
            {
                case DRIVE_REMOVABLE:
                case DRIVE_CDROM:   v.isRemovable = true; v.ejectable = true; break;
                case DRIVE_REMOTE:  v.isNetwork   = true;                     break;
                default: break;
            }

            vols.add (v);
        }

        return vols;
    }

    inline bool ejectVolume (const VolumeInfo& vol, juce::String& errorOut)
    {
        auto letter     = vol.root.getFullPathName().substring (0, 2); // "E:"
        auto devicePath = "\\\\.\\" + letter;

        HANDLE h = CreateFileW (devicePath.toWideCharPointer(),
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr, OPEN_EXISTING, 0, nullptr);
        if (h == INVALID_HANDLE_VALUE)
        {
            errorOut = "Could not open the device for ejection.";
            return false;
        }

        DWORD bytes = 0;
        bool ok = true;

        // Lock so no other handle blocks the dismount.
        if (! DeviceIoControl (h, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &bytes, nullptr))
        {
            errorOut = "Volume is in use — close any open files first.";
            ok = false;
        }

        if (ok && ! DeviceIoControl (h, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &bytes, nullptr))
        {
            errorOut = "Could not dismount the volume.";
            ok = false;
        }

        if (ok)
        {
            PREVENT_MEDIA_REMOVAL pmr { FALSE };
            DeviceIoControl (h, IOCTL_STORAGE_MEDIA_REMOVAL, &pmr, sizeof (pmr),
                             nullptr, 0, &bytes, nullptr);

            if (! DeviceIoControl (h, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &bytes, nullptr))
            {
                errorOut = "The device refused the eject request.";
                ok = false;
            }
        }

        CloseHandle (h);
        return ok;
    }

    //==============================================================================
   #else // Linux / other POSIX

    inline juce::Array<VolumeInfo> getVolumes()
    {
        juce::Array<VolumeInfo> vols;

        juce::Array<juce::File> roots;
        juce::File::findFileSystemRoots (roots);
        for (auto& r : roots)
        {
            VolumeInfo v;
            v.root = r;
            v.name = r.getFullPathName();
            vols.add (v);
        }

        juce::StringArray mountDirs { "/media",
                                      "/media/" + juce::SystemStats::getLogonName(),
                                      "/run/media/" + juce::SystemStats::getLogonName() };
        for (auto& d : mountDirs)
        {
            juce::Array<juce::File> mounts;
            juce::File (d).findChildFiles (mounts, juce::File::findDirectories, false);
            for (auto& m : mounts)
            {
                VolumeInfo v;
                v.root        = m;
                v.name        = m.getFileName();
                v.isRemovable = true;
                v.ejectable   = true;
                vols.add (v);
            }
        }

        return vols;
    }

    inline bool ejectVolume (const VolumeInfo& vol, juce::String& errorOut)
    {
        juce::ChildProcess proc;
        if (! proc.start (juce::StringArray { "eject", vol.root.getFullPathName() }))
        {
            errorOut = "Could not launch 'eject'.";
            return false;
        }
        auto output = proc.readAllProcessOutput();
        proc.waitForProcessToFinish (15000);
        if (proc.getExitCode() != 0)
        {
            errorOut = output.trim().isNotEmpty() ? output.trim() : "eject reported an error.";
            return false;
        }
        return true;
    }

   #endif

    //==============================================================================
    /** A stable signature of the current volume set, used to detect hot-plug
        changes cheaply without rebuilding the UI every tick. */
    inline juce::String volumesSignature (const juce::Array<VolumeInfo>& vols)
    {
        juce::String sig;
        for (auto& v : vols)
            sig << v.root.getFullPathName() << "|" << v.name << ";";
        return sig;
    }
}

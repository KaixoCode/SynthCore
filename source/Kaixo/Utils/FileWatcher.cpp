
// ------------------------------------------------

#include "Kaixo/Utils/FileWatcher.hpp"

// ------------------------------------------------

#ifdef WIN32

// ------------------------------------------------

#define WINDOWS_LEAN_AND_MEAN
#include "Windows.h"

// ------------------------------------------------

FileWatcher::FileWatcher(std::filesystem::path path, std::function<void()> callback)
    : m_WatchThread(&FileWatcher::watch, this, std::move(path), std::move(callback))
{}

FileWatcher::~FileWatcher() {
    stop();
    if (m_WatchThread.joinable())
        m_WatchThread.join();
}

// ------------------------------------------------

void FileWatcher::stop() { m_Active = false; }

// ------------------------------------------------

void FileWatcher::watch(std::filesystem::path path, std::function<void()> callback) {
    HANDLE handle = FindFirstChangeNotificationW(path.c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (handle == INVALID_HANDLE_VALUE) return;

    while (m_Active) {
        auto waitStatus = WaitForSingleObject(handle, 100);
        switch (waitStatus) {
        case WAIT_OBJECT_0: 
            callback(); 
            if (FindNextChangeNotification(handle) == FALSE) return;
            // ^^ Tries to restart the file watch, if fail; return
            break;
        case WAIT_FAILED: return;
        default: break; // Just continue waiting
        }
    }
}

// ------------------------------------------------

#else

// ------------------------------------------------

FileWatcher::FileWatcher(std::filesystem::path, std::function<void()>) {}
FileWatcher::~FileWatcher() {}

// ------------------------------------------------

void FileWatcher::stop() {}

// ------------------------------------------------

void FileWatcher::watch(std::filesystem::path, std::function<void()>) {}

// ------------------------------------------------

#endif

// ------------------------------------------------

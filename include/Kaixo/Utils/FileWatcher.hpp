#pragma once

// ------------------------------------------------

#include <thread>
#include <filesystem>
#include <functional>

// ------------------------------------------------

class FileWatcher {
public:

    // ------------------------------------------------

    FileWatcher(std::filesystem::path path, std::function<void()> callback);
    ~FileWatcher();

    // ------------------------------------------------

    void stop();

    // ------------------------------------------------

private:
    std::thread m_WatchThread{};
    std::atomic_bool m_Active = true;
    
    // ------------------------------------------------

    void watch(std::filesystem::path path, std::function<void()> callback);

    // ------------------------------------------------

};

// ------------------------------------------------

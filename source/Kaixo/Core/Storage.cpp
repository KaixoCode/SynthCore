#include "Kaixo/Core/Storage.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    bool Storage::flag(std::string_view id) {
        return getOrDefault<bool>(id, false);
    }

    // ------------------------------------------------

    Storage& Storage::instance() {
        static Storage storage{};
        return storage;
    }

    // ------------------------------------------------

    Storage::Storage() {
        juce::PropertiesFile::Options options;
        options.applicationName = JucePlugin_Name;
        options.filenameSuffix = ".settings";
        options.storageFormat = juce::PropertiesFile::storeAsBinary;
        options.osxLibrarySubFolder = "Application Support";

        m_Properties.setStorageParameters(options);
        if (!m_Properties.getUserSettings()->save()) {
            std::cout << "Fuck aaaa";
        }
    }

    // ------------------------------------------------

}
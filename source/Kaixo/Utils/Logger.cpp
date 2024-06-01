
// ------------------------------------------------

#include "Kaixo/Utils/Logger.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    void Logger::log(const std::string& message) {
        auto const time = std::chrono::current_zone()
            ->to_local(std::chrono::system_clock::now());
        juce::Logger::writeToLog(std::format("{:%Y-%m-%d %X}", time) + message);
    }

    // ------------------------------------------------

}

// ------------------------------------------------

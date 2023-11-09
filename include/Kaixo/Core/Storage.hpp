#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    namespace Setting {

        // ------------------------------------------------

        constexpr std::string_view TouchMode = "TouchMode";

        // ------------------------------------------------

    }

    // ------------------------------------------------

    class Storage {
    public:

        // ------------------------------------------------

        template<class Ty>
        static std::optional<Ty> get(std::string_view id);
        
        template<class Ty, class Val>
        static Ty getOrDefault(std::string_view id, Val&& val);

        template<class Ty, class Val>
        static void set(std::string_view id, Val&& value);

        static bool flag(std::string_view id);

        // ------------------------------------------------

    private:
        ApplicationProperties m_Properties{};

        // ------------------------------------------------
        
        static Storage& instance();

        // ------------------------------------------------

        Storage();

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty>
    std::optional<Ty> Storage::get(std::string_view id) {
        juce::String key{ id.data(), id.size() };
        juce::PropertiesFile* settings = instance().m_Properties.getUserSettings();
        if (!settings->containsKey(key)) return {};

        if constexpr (std::same_as<Ty, double> || std::same_as<Ty, float>)
            return settings->getDoubleValue(key);
        if constexpr (std::same_as<Ty, int>)
            return settings->getIntValue(key);
        if constexpr (std::same_as<Ty, bool>)
            return settings->getBoolValue(key);
        if constexpr (std::same_as<Ty, std::string>)
            return settings->getValue(key).toStdString();
    }

    template<class Ty, class Val>
    Ty Storage::getOrDefault(std::string_view id, Val&& val) {
        juce::String key{ id.data(), id.size() };
        juce::PropertiesFile* settings = instance().m_Properties.getUserSettings();
        if (!settings->containsKey(key)) {
            if constexpr (std::same_as<Ty, std::string>) {
                settings->setValue(key, (juce::String)std::string{ val });
            } else {
                settings->setValue(key, (Ty)val);
            }
        }

        if constexpr (std::same_as<Ty, double> || std::same_as<Ty, float>)
            return settings->getDoubleValue(key);
        if constexpr (std::same_as<Ty, int>)
            return settings->getIntValue(key);
        if constexpr (std::same_as<Ty, bool>)
            return settings->getBoolValue(key);
        if constexpr (std::same_as<Ty, std::string>)
            return settings->getValue(key).toStdString();
    }

    template<class Ty, class Val>
    void Storage::set(std::string_view id, Val&& value) {
        juce::String key{ id.data(), id.size() };
        juce::PropertiesFile* settings = instance().m_Properties.getUserSettings();

        if constexpr (std::same_as<Ty, std::string>) {
            settings->setValue(key, (juce::String)std::string{ value });
        } else {
            settings->setValue(key, (Ty)value);
        }
    }

    // ------------------------------------------------

}
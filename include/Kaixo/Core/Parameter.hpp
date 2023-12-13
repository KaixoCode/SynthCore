#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Formatters.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    struct ParameterSettings {

        // ------------------------------------------------

        ParamID id;

        // ------------------------------------------------

        std::string_view name = "";              // Parameter name
        std::string_view shortName = name;       // Shortened parameter name
        std::string_view identifier = name;      // Full identifier (used as name in DAW)
        std::string_view shortIdentifier = name; // Shortened identifier
        std::string_view varName = name;         // Name of the variable in the global object
        std::string_view fullVarName = name;     // Full name of the variable
        std::string_view description = "";

        // ------------------------------------------------

        ParamValue defaultValue = 0.;
        std::size_t steps = 0;
        Transform transform = Transformers::Range<0.f, 1.f>;
        Formatter format = Formatters::Default;
        ParamValue normalizedDefaultValue() const { return transform.normalize(defaultValue); };
        bool doSmoothing() const { return smooth && steps == 0; }
        bool doModulation() const { return steps == 0 && modulatable; }

        // ------------------------------------------------

        bool smooth : 1 = false;
        bool multiply : 1 = false;
        bool constrain : 1 = true;
        bool modulatable : 1 = true;
        bool automatable : 1 = true;

        // ------------------------------------------------

        std::string toString(ParamValue val) const { return format.format(transform.transform(val)); }
        ParamValue fromString(std::string_view val) const { return transform.normalize(format.parse(val)); }

        // ------------------------------------------------
        
        constexpr operator ParamID() const { return id; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    constexpr basic_json& getFromIdentifier(basic_json& j, std::string_view identifier) {
        auto parts = split(identifier, ".");

        basic_json* p = &j;
        for (auto& part : parts) {
            if (part.ends_with("]")) {
                auto f = part.find_first_of('[') + 1;
                auto l = part.find_first_of(']');
                auto number = part.substr(f, l - f);
                auto name = part.substr(0, f - 1);
                std::size_t value = 0;
                std::from_chars(number.data(), number.data() + number.size(), value);
                p = &(*p)[name][value];
            }
            else {
                p = &(*p)[part];
            }
        }

        return *p;
    }

    // ------------------------------------------------
    
    class Parameter : public juce::AudioProcessorParameter {
    public:

        // ------------------------------------------------

        Parameter(const ParameterSettings& settings) 
            : m_Settings(&settings), 
            m_PrecalculatedNormalizedDefaultValue(m_Settings->normalizedDefaultValue()),
            m_NormalizedValue(m_PrecalculatedNormalizedDefaultValue)
        {}

        // ------------------------------------------------

        bool  isAutomatable()   const override { return m_Settings->automatable; }
        bool  isBoolean()       const override { return m_Settings->steps == 2; }
        bool  isDiscrete()      const override { return m_Settings->steps != 0; }
        int   getNumSteps()     const override { return static_cast<int>(m_Settings->steps); }
        float getDefaultValue() const override { return defaultValue(); }

        float getValue()         const override { return value(); }
        void  setValue(float newValue) override { m_NormalizedValue = newValue; }

        // ------------------------------------------------
        
        ParamValue value() const { return m_NormalizedValue; }
        ParamValue defaultValue() const { return m_PrecalculatedNormalizedDefaultValue; }

        // ------------------------------------------------

        juce::String getLabel() const override { return ""; };
        juce::String getName(int)                        const override { return juce::String{ identifier().data()}; }
        juce::String getText(float normalisedValue, int) const override { return m_Settings->toString(normalisedValue); }
        float getValueForText(const juce::String& text)  const override { return m_Settings->fromString(text.toStdString()); }

        // ------------------------------------------------
        
        std::string      toString()        const { return m_Settings->toString(m_NormalizedValue); }
        std::string_view displayName()     const { return m_Settings->name; }
        std::string_view shortName()       const { return m_Settings->shortName; }
        std::string_view identifier()      const { return m_Settings->identifier; }
        std::string_view shortIdentifier() const { return m_Settings->shortIdentifier; }
        std::string_view description()     const { return m_Settings->description; }

        // ------------------------------------------------

    protected:
        const ParameterSettings* m_Settings;
        const ParamValue m_PrecalculatedNormalizedDefaultValue;
        ParamValue m_NormalizedValue = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    struct ModulationSourceSettings {

        // ------------------------------------------------

        ModulationSourceID id = 0;

        // ------------------------------------------------

        std::string_view name = "";              // Source name
        std::string_view shortName = name;       // Shortened source name
        std::string_view identifier = name;      // Full identifier (used as name in DAW)
        std::string_view shortIdentifier = name; // Shortened identifier
        std::string_view varName = name;         // Name of the source in the global object
        std::string_view fullVarName = name;     // Full name of the source
        std::string_view description = "";

        // ------------------------------------------------

        bool bidirectional = false;

        // ------------------------------------------------

        constexpr operator ModulationSourceID() const { return id; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// Automatically generated parameter settings
#include <Parameters.hpp>

#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * [g]
     * [g, a]
     * [r, g, b]
     * [r, g, b, a]
     */

    // ------------------------------------------------
    
    class Color {
    public:

        // ------------------------------------------------

        struct Interface {
            virtual juce::Colour get() const = 0;
        };

        // ------------------------------------------------

        Color() = default;
        Color(std::unique_ptr<Interface> graphics);
        Color(Kaixo::Color color);

        // ------------------------------------------------

        operator juce::Colour() const;
        operator bool() const;

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};
        Kaixo::Color m_DefaultColor{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class ColorElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        Kaixo::Color color{ 0, 0, 0, 0, };

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator juce::Colour() const { return color; }
        operator Kaixo::Color() const { return color; }

        // ------------------------------------------------
        
        operator Color() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<>
    inline DynamicElement::operator Color() {
        if (auto color = dynamic_cast<ColorElement*>(m_Element)) {
            return *color;
        }

        return {};
    }

    // ------------------------------------------------

}
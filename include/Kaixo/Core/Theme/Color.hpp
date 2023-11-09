#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

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

        // ------------------------------------------------

        operator juce::Colour() const { return m_Graphics ? m_Graphics->get() : juce::Colour{}; }
        operator bool() const { return (bool)m_Graphics; }

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class ColorElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        Kaixo::Color color{};

        // ------------------------------------------------

        void interpret(const json& theme) override;

        // ------------------------------------------------

        operator juce::Colour() const { return color; }
        operator Kaixo::Color() const { return color; }

        // ------------------------------------------------
        
        operator Color() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
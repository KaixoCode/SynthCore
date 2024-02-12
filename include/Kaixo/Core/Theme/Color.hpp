#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/StateLinked.hpp"
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
    
    class Color : public Animation {
    public:

        // ------------------------------------------------

        struct Interface : Animation {
            virtual ~Interface() = default;
            virtual Kaixo::Color get(View::State state = View::State::Default) = 0;
        };

        // ------------------------------------------------

        Color() = default;
        Color(std::unique_ptr<Interface> graphics);
        Color(Kaixo::Color color);

        // ------------------------------------------------

        operator juce::Colour() const;
        operator Kaixo::Color() const;
        operator bool() const;

        // ------------------------------------------------

        Kaixo::Color get(View::State state) const;

        // ------------------------------------------------
        
        bool changing() const override { return m_Graphics ? m_Graphics->changing() : false; }

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
        
        StateLinked<Animated<Kaixo::Color>> color;

        // ------------------------------------------------

        void interpret(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme) override;

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
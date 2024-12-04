#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/StateLinked.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/ExpressionParser.hpp"

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
            virtual Kaixo::Color get(View::State state = View::State::Default, const ExpressionParser::ValueMap& values = {}) = 0;
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

        Kaixo::Color get(View::State state, const ExpressionParser::ValueMap& values = {}) const;

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
        
        std::size_t loadIndex = 0;
        StateLinked<Animated<ExpressionParser::Expression>> r;
        StateLinked<Animated<ExpressionParser::Expression>> g;
        StateLinked<Animated<ExpressionParser::Expression>> b;
        StateLinked<Animated<ExpressionParser::Expression>> a;

        // ------------------------------------------------

        void interpret(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme) override;

        // ------------------------------------------------
        
        bool hasR(View::State state = View::State::Default) const;
        bool hasG(View::State state = View::State::Default) const;
        bool hasB(View::State state = View::State::Default) const;
        bool hasA(View::State state = View::State::Default) const;
        bool hasValue(View::State state = View::State::Default) const;

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
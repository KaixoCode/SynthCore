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
     * [x, y, w, h]
     * [x, y] [w, h]
     * x, y, w, h
     */

    // ------------------------------------------------

    class Rectangle : Animation {
    public:

        // ------------------------------------------------

        struct Interface : Animation {
            virtual ~Interface() = default;
            virtual Rect<float> get(View::State state = View::State::Default, const ExpressionParser::ValueMap& values = {}) = 0;
        };

        // ------------------------------------------------

        Rectangle() = default;
        Rectangle(std::unique_ptr<Interface> graphics);
        Rectangle(Rect<float> value);

        // ------------------------------------------------

        operator juce::Rectangle<float>() const;
        operator juce::Rectangle<int>() const;
        operator Rect<float>() const;
        operator Rect<int>() const;
        operator bool() const;

        // ------------------------------------------------

        Rect<float> get(View::State state, const ExpressionParser::ValueMap& values = {}) const;

        // ------------------------------------------------

        bool changing() const override { return m_Graphics ? m_Graphics->changing() : false; }

        // ------------------------------------------------

    public:
        std::unique_ptr<Interface> m_Graphics{};
        Rect<float> m_Default{};

        // ------------------------------------------------

    };

    class RectangleElement : Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        std::size_t loadIndex = 0;
        StateLinked<Animated<ExpressionParser::Expression>> x;
        StateLinked<Animated<ExpressionParser::Expression>> y;
        StateLinked<Animated<ExpressionParser::Expression>> w;
        StateLinked<Animated<ExpressionParser::Expression>> h;

        // ------------------------------------------------
        
        bool hasX(View::State state = View::State::Default) const;
        bool hasY(View::State state = View::State::Default) const;
        bool hasWidth(View::State state = View::State::Default) const;
        bool hasHeight(View::State state = View::State::Default) const;
        bool hasPosition(View::State state = View::State::Default) const;
        bool hasSize(View::State state = View::State::Default) const;
        bool hasValue(View::State state = View::State::Default) const;

        // ------------------------------------------------
        
        void reset();

        void interpretX(const basic_json& theme, View::State state);
        void interpretY(const basic_json& theme, View::State state);
        void interpretWidth(const basic_json& theme, View::State state);
        void interpretHeight(const basic_json& theme, View::State state);
        void interpretPosition(const basic_json& theme, View::State state);
        void interpretSize(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator Rectangle();

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<>
    inline DynamicElement::operator Rectangle() {
        if (auto color = dynamic_cast<RectangleElement*>(m_Element)) {
            return *color;
        }

        return {};
    }

    // ------------------------------------------------

}
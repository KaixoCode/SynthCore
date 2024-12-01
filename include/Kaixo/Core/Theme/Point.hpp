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
     * [x, y]
     * x, y
     */

    // ------------------------------------------------

    class Point : Animation {
    public:

        // ------------------------------------------------

        struct Interface : Animation {
            virtual ~Interface() = default;
            virtual Kaixo::Point<float> get(View::State state = View::State::Default, const ExpressionParser::ValueMap& values = {}) = 0;
        };

        // ------------------------------------------------

        Point() = default;
        Point(std::unique_ptr<Interface> graphics);
        Point(Kaixo::Point<float> value);

        // ------------------------------------------------

        operator juce::Point<float>() const;
        operator juce::Point<int>() const;
        operator Kaixo::Point<float>() const;
        operator Kaixo::Point<int>() const;
        operator bool() const;

        // ------------------------------------------------

        Kaixo::Point<float> get(View::State state, const ExpressionParser::ValueMap& values = {}) const;

        // ------------------------------------------------

        bool changing() const override { return m_Graphics ? m_Graphics->changing() : false; }

        // ------------------------------------------------

    public:
        std::unique_ptr<Interface> m_Graphics{};
        Kaixo::Point<float> m_Default{};

        // ------------------------------------------------

    };

    class PointElement : Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        std::size_t loadIndex = 0;
        StateLinked<Animated<ExpressionParser::Function>> x;
        StateLinked<Animated<ExpressionParser::Function>> y;

        // ------------------------------------------------
        
        bool hasX(View::State state = View::State::Default) const;
        bool hasY(View::State state = View::State::Default) const;

        // ------------------------------------------------
        
        void reset();

        void interpretX(const basic_json& theme, View::State state);
        void interpretY(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator Point();

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<>
    inline DynamicElement::operator Point() {
        if (auto color = dynamic_cast<PointElement*>(m_Element)) {
            return *color;
        }

        return {};
    }

    // ------------------------------------------------

}
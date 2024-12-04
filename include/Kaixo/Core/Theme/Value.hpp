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

    class Value : Animation {
    public:

        // ------------------------------------------------

        struct Interface : Animation {
            virtual ~Interface() = default;
            virtual float get(View::State state = View::State::Default, const ExpressionParser::ValueMap& values = {}) = 0;
        };

        // ------------------------------------------------

        Value() = default;
        Value(std::unique_ptr<Interface> graphics);
        Value(float value);

        // ------------------------------------------------

        operator float() const;
        operator bool() const;

        // ------------------------------------------------

        float get(View::State state, const ExpressionParser::ValueMap& values = {}) const;

        // ------------------------------------------------

        bool changing() const override { return m_Graphics ? m_Graphics->changing() : false; }

        // ------------------------------------------------

    public:
        std::unique_ptr<Interface> m_Graphics{};
        float m_Default{};

        // ------------------------------------------------

    };

    class ValueElement : Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        std::size_t loadIndex = 0;
        StateLinked<Animated<ExpressionParser::Expression>> val;

        // ------------------------------------------------
        
        bool hasValue(View::State state = View::State::Default) const;

        // ------------------------------------------------
        
        void reset();

        void interpret(const basic_json& theme, View::State state);
        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator Value();

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    template<>
    inline DynamicElement::operator Value() {
        if (auto color = dynamic_cast<ValueElement*>(m_Element)) {
            return *color;
        }

        return {};
    }

    // ------------------------------------------------

}
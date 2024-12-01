
// ------------------------------------------------

#include "Kaixo/Core/Theme/Value.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {
    
    // ------------------------------------------------

    Value::Value(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    Value::Value(float def)
        : m_Default(def)
    {}

    // ------------------------------------------------

    Value::operator float() const { return m_Graphics ? m_Graphics->get() : m_Default; }
    Value::operator bool() const { return (bool)m_Graphics; }

    float Value::get(View::State state, const ExpressionParser::ValueMap& values) const {
        return m_Graphics ? m_Graphics->get(state, values) : m_Default;
    }

    // ------------------------------------------------
    
    bool ValueElement::hasValue(View::State state) const { return val[state].value.operator bool(); }

    // ------------------------------------------------

    void ValueElement::reset() {
        val.reset();
    }

    inline static auto parseExpressionOrNumber(ExpressionParser::Function& val, const basic_json& theme, View::State) {
        if (theme.is(basic_json::Number)) return val = [v = theme.as<float>()](auto&) { return v; }, true;
        if (theme.is(basic_json::String)) return val = ExpressionParser::parse(theme.as<std::string_view>()), true;
        return false;
    }
    
    void ValueElement::interpret(const basic_json& theme, View::State state) {
        val.interpret(theme, parseExpressionOrNumber, state);
    }

    void ValueElement::interpret(const basic_json& theme) {
        reset();
        interpret(theme, View::State::Default);
    }

    // ------------------------------------------------

    ValueElement::operator Value() {
        struct Implementation : public Value::Interface {
            Implementation(const ValueElement* self) : self(self) {}

            std::size_t loadIndex = npos;
            const ValueElement* self;
            Animated<float> val;
            ExpressionParser::ValueMap valuesCache;
            View::State state = View::State::NoState;
            bool changingCache = false;

            float get(View::State s = View::State::Default, const ExpressionParser::ValueMap& values = {}) override {
                const auto reassign = [&] {
                    auto ve = self->val[s];
                    if (ve.value) val = { ve.value(values), ve.transition };
                };

                if (loadIndex != self->loadIndex) {
                    loadIndex = self->loadIndex;
                    reassign();
                }

                if (valuesCache != values) {
                    valuesCache = values;
                    reassign();
                }

                if (state != s) {
                    state = s;
                    reassign();
                }

                changingCache = val.changing();

                return val.get();
            }

            bool changing() const override { return changingCache; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}

// ------------------------------------------------

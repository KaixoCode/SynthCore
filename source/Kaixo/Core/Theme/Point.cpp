
// ------------------------------------------------

#include "Kaixo/Core/Theme/Point.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {
    
    // ------------------------------------------------

    Point::Point(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    Point::Point(Kaixo::Point<float> def)
        : m_Default(def)
    {}

    // ------------------------------------------------

    Point::operator juce::Point<float>() const { return operator Kaixo::Point<float>(); }
    Point::operator juce::Point<int>() const { return operator Kaixo::Point<int>(); }
    Point::operator Kaixo::Point<float>() const { return m_Graphics ? m_Graphics->get() : m_Default; }
    Point::operator Kaixo::Point<int>() const { return m_Graphics ? m_Graphics->get() : m_Default; }
    Point::operator bool() const { return (bool)m_Graphics; }

    Kaixo::Point<float> Point::get(View::State state, const ExpressionParser::ValueMap& values) const {
        return m_Graphics ? m_Graphics->get(state, values) : m_Default;
    }

    // ------------------------------------------------
    
    bool PointElement::hasX(View::State state) const { return x[state].value.operator bool(); }
    bool PointElement::hasY(View::State state) const { return y[state].value.operator bool(); }

    // ------------------------------------------------

    void PointElement::reset() {
        x.reset();
        y.reset();
    }
    
    inline static auto parseExpressionOrNumber(const ExpressionParser::FunctionMap& funs) {
        return [&](ExpressionParser::Expression& val, const basic_json& theme, View::State) {
            if (theme.is(basic_json::Number)) return val = [v = theme.as<float>()](auto&) { return v; }, true;
            if (theme.is(basic_json::String)) return val = ExpressionParser::parse(theme.as<std::string_view>(), funs), true;
            return false;
        };
    }
    
    inline static auto parseIdx(const ExpressionParser::FunctionMap& funs, std::size_t index) {
        return [&, index](ExpressionParser::Expression& val, const basic_json& theme, View::State state) {
            if (theme.is(basic_json::Array) && index < theme.size()) {
                return parseExpressionOrNumber(funs)(val, theme[index], state);
            }

            return false;
        };
    }

    void PointElement::interpretX(const basic_json& theme, View::State state) {
        x.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void PointElement::interpretY(const basic_json& theme, View::State state) {
        y.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void PointElement::interpret(const basic_json& theme, View::State state) {
        x.interpret(theme, parseIdx(self->functions, 0), state, self->functions);
        y.interpret(theme, parseIdx(self->functions, 1), state, self->functions);

        if (theme.contains("x")) interpretX(theme["x"], state);
        if (theme.contains("y")) interpretY(theme["y"], state);
    }

    void PointElement::interpret(const basic_json& theme) {
        reset();
        interpret(theme, View::State::Default);
    }

    // ------------------------------------------------

    PointElement::operator Point() {
        struct Implementation : public Point::Interface {
            Implementation(const PointElement* self) : self(self) {}

            std::size_t loadIndex = npos;
            const PointElement* self;
            Animated<float> x;
            Animated<float> y;
            ExpressionParser::ValueMap valuesCache;
            View::State state = View::State::NoState;
            bool changingCache = false;

            Kaixo::Point<float> get(View::State s = View::State::Default, const ExpressionParser::ValueMap& values = {}) override {
                const auto reassign = [&] {
                    auto xe = self->x[s];
                    auto ye = self->y[s];

                    if (xe.value) x = { xe.value(values), xe.transition, std::move(xe.curve) };
                    if (ye.value) y = { ye.value(values), ye.transition, std::move(ye.curve) };
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

                changingCache = x.changing() || y.changing();

                return { x.get(), y.get() };
            }

            bool changing() const override { return changingCache; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}

// ------------------------------------------------

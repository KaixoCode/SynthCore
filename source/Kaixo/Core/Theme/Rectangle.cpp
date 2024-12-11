
// ------------------------------------------------

#include "Kaixo/Core/Theme/Rectangle.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {
    
    // ------------------------------------------------

    Rectangle::Rectangle(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    Rectangle::Rectangle(Rect<float> def)
        : m_Default(def)
    {}

    // ------------------------------------------------

    Rectangle::operator juce::Rectangle<float>() const { return operator Rect<float>(); }
    Rectangle::operator juce::Rectangle<int>() const { return operator Rect<int>(); }
    Rectangle::operator Rect<float>() const { return m_Graphics ? m_Graphics->get() : m_Default; }
    Rectangle::operator Rect<int>() const { return m_Graphics ? m_Graphics->get() : m_Default; }
    Rectangle::operator bool() const { return (bool)m_Graphics; }

    Rect<float> Rectangle::get(View::State state, const ExpressionParser::ValueMap& values) const {
        return m_Graphics ? m_Graphics->get(state, values) : m_Default;
    }

    // ------------------------------------------------
    
    bool RectangleElement::hasX(View::State state) const { return x[state].value.operator bool(); }
    bool RectangleElement::hasY(View::State state) const { return y[state].value.operator bool(); }
    bool RectangleElement::hasWidth(View::State state) const { return w[state].value.operator bool(); }
    bool RectangleElement::hasHeight(View::State state) const { return h[state].value.operator bool(); }
    bool RectangleElement::hasPosition(View::State state) const { return hasX(state) && hasY(state); }
    bool RectangleElement::hasSize(View::State state) const { return hasWidth(state) && hasHeight(state); }
    bool RectangleElement::hasValue(View::State state) const { return hasPosition(state) && hasSize(state); }

    // ------------------------------------------------

    void RectangleElement::reset() {
        x.reset();
        y.reset();
        w.reset();
        h.reset();
    }

    inline static auto parseExpressionOrNumber(const ExpressionParser::FunctionMap& funs) {
        return [&](ExpressionParser::Expression& val, const basic_json& theme, View::State) {
            if (theme.is<basic_json::number_t>()) return val = [v = theme.as<float>()](auto&) { return v; }, true;
            if (theme.is<basic_json::string_t>()) return val = ExpressionParser::parse(theme.as<std::string_view>(), funs), true;
            return false;
        };
    }
    
    inline static auto parseIdx(const ExpressionParser::FunctionMap& funs, std::size_t index) {
        return [&, index](ExpressionParser::Expression& val, const basic_json& theme, View::State state) {
            if (theme.is<basic_json::array_t>() && index < theme.size()) {
                return parseExpressionOrNumber(funs)(val, theme[index], state);
            }

            return false;
        };
    }

    void RectangleElement::interpretX(const basic_json& theme, View::State state) {
        x.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void RectangleElement::interpretY(const basic_json& theme, View::State state) {
        y.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void RectangleElement::interpretWidth(const basic_json& theme, View::State state) {
        w.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void RectangleElement::interpretHeight(const basic_json& theme, View::State state) {
        h.interpret(theme, parseExpressionOrNumber(self->functions), state, self->functions);
    }

    void RectangleElement::interpretPosition(const basic_json& theme, View::State state) {
        x.interpret(theme, parseIdx(self->functions, 0), state, self->functions);
        y.interpret(theme, parseIdx(self->functions, 1), state, self->functions);

        if (theme.contains("x")) interpretX(theme["x"], state);
        if (theme.contains("y")) interpretY(theme["y"], state);
    }

    void RectangleElement::interpretSize(const basic_json& theme, View::State state) {
        w.interpret(theme, parseIdx(self->functions, 0), state, self->functions);
        h.interpret(theme, parseIdx(self->functions, 1), state, self->functions);

        if (theme.contains("width")) interpretWidth(theme["width"], state);
        if (theme.contains("height")) interpretHeight(theme["height"], state);
    }

    void RectangleElement::interpret(const basic_json& theme, View::State state) {
        // [x, y, w, h]
        // position: [x, y], size: [w, h]
        // x: x, y: y, width: w, height: h

        x.interpret(theme, parseIdx(self->functions, 0), state, self->functions);
        y.interpret(theme, parseIdx(self->functions, 1), state, self->functions);
        w.interpret(theme, parseIdx(self->functions, 2), state, self->functions);
        h.interpret(theme, parseIdx(self->functions, 3), state, self->functions);

        if (theme.contains("position")) interpretPosition(theme["position"], state);
        if (theme.contains("size")) interpretSize(theme["size"], state);

        if (theme.contains("x")) interpretX(theme["x"], state);
        if (theme.contains("y")) interpretY(theme["y"], state);
        if (theme.contains("width")) interpretWidth(theme["width"], state);
        if (theme.contains("height")) interpretHeight(theme["height"], state);
    }

    void RectangleElement::interpret(const basic_json& theme) {
        reset();
        interpret(theme, View::State::Default);
    }

    // ------------------------------------------------

    RectangleElement::operator Rectangle() {
        struct Implementation : public Rectangle::Interface {
            Implementation(const RectangleElement* self) : self(self) {}

            std::size_t loadIndex = npos;
            const RectangleElement* self;
            Animated<float> x;
            Animated<float> y;
            Animated<float> w;
            Animated<float> h;
            ExpressionParser::ValueMap valuesCache;
            View::State state = View::State::NoState;
            bool changingCache = false;

            Rect<float> get(View::State s = View::State::Default, const ExpressionParser::ValueMap& values = {}) override {
                const auto reassign = [&] {
                    auto xe = self->x[s];
                    auto ye = self->y[s];
                    auto we = self->w[s];
                    auto he = self->h[s];

                    if (xe.value) x = { xe.value(values), xe.transition, std::move(xe.curve) };
                    if (ye.value) y = { ye.value(values), ye.transition, std::move(ye.curve) };
                    if (we.value) w = { we.value(values), we.transition, std::move(we.curve) };
                    if (he.value) h = { he.value(values), he.transition, std::move(he.curve) };
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

                changingCache = x.changing() || y.changing() || w.changing() || h.changing();

                return { x.get(), y.get(), w.get(), h.get() };
            }

            bool changing() const override { return changingCache; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}

// ------------------------------------------------

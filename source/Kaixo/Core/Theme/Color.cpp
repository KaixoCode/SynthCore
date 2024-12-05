#include "Kaixo/Core/Theme/Color.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Color::Color(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    Color::Color(Kaixo::Color color) 
        : m_DefaultColor(color) 
    {}

    // ------------------------------------------------

    Color::operator juce::Colour() const { return operator Kaixo::Color(); }
    Color::operator Kaixo::Color() const { return m_Graphics ? m_Graphics->get() : m_DefaultColor; }
    Color::operator bool() const { return (bool)m_Graphics; }

    Kaixo::Color Color::get(View::State state, const ExpressionParser::ValueMap& values) const { 
        return m_Graphics ? m_Graphics->get(state, values) : m_DefaultColor;
    }

    // ------------------------------------------------

    void ColorElement::interpret(const basic_json& theme, View::State state) {
        
        // ------------------------------------------------

        const auto parseExpressionOrNumber = [&](ExpressionParser::Expression& val, const basic_json& theme, View::State = {}) {
            if (theme.is(basic_json::Number)) return val = [v = theme.as<float>()](auto&) { return v; }, true;
            if (theme.is(basic_json::String)) return val = ExpressionParser::parse(theme.as<std::string_view>(), self->functions), true;
            return false;
        };

        const auto isValidColor = [](const basic_json& clr) -> bool {
            return clr.is(basic_json::Array) && !clr.empty() && clr.size() <= 4;
        };

        const auto parseRed = [&](ExpressionParser::Expression& red, const basic_json& theme, View::State) -> bool {
            return isValidColor(theme) && parseExpressionOrNumber(red, theme[0]);
        };
        
        const auto parseGreen = [&](ExpressionParser::Expression& green, const basic_json& theme, View::State) -> bool {
            return isValidColor(theme) && parseExpressionOrNumber(green, theme[theme.size() >= 3 ? 1 : 0]);
        };
        
        const auto parseBlue = [&](ExpressionParser::Expression& blue, const basic_json& theme, View::State) -> bool {
            return isValidColor(theme) && parseExpressionOrNumber(blue, theme[theme.size() >= 3 ? 2 : 0]);
        };
        
        const auto parseAlpha = [&](ExpressionParser::Expression& alpha, const basic_json& theme, View::State) -> bool {
            if (!isValidColor(theme)) return false;
            if (theme.size() == 1 || theme.size() == 3) return alpha = [](auto&) { return 255; }, true;
            return parseExpressionOrNumber(alpha, theme[theme.size() == 2 ? 1 : 3]);
        };

        // ------------------------------------------------
        
        r.interpret(theme, parseRed, state, self->functions);
        g.interpret(theme, parseGreen, state, self->functions);
        b.interpret(theme, parseBlue, state, self->functions);
        a.interpret(theme, parseAlpha, state, self->functions);

        // ------------------------------------------------
        
        if (theme.contains("rgb")) {
            r.interpret(theme["rgb"], parseRed, state, self->functions);
            g.interpret(theme["rgb"], parseGreen, state, self->functions);
            b.interpret(theme["rgb"], parseBlue, state, self->functions);
        }
        
        if (theme.contains("r")) r.interpret(theme["r"], parseExpressionOrNumber, state, self->functions);
        if (theme.contains("g")) g.interpret(theme["g"], parseExpressionOrNumber, state, self->functions);
        if (theme.contains("b")) b.interpret(theme["b"], parseExpressionOrNumber, state, self->functions);
        if (theme.contains("a")) a.interpret(theme["a"], parseExpressionOrNumber, state, self->functions);

        // ------------------------------------------------
        
        loadIndex++;

        // ------------------------------------------------

    }

    void ColorElement::interpret(const basic_json& theme) {
        r.reset();
        g.reset();
        b.reset();
        a.reset();
        interpret(theme, View::State::Default);
    }

    // ------------------------------------------------

    bool ColorElement::hasR(View::State state) const { return r[state].value.operator bool(); }
    bool ColorElement::hasG(View::State state) const { return g[state].value.operator bool(); }
    bool ColorElement::hasB(View::State state) const { return b[state].value.operator bool(); }
    bool ColorElement::hasA(View::State state) const { return a[state].value.operator bool(); }

    bool ColorElement::hasValue(View::State state) const { return hasR(state) && hasG(state) && hasB(state) && hasA(state); }

    // ------------------------------------------------
    
    ColorElement::operator Color() const {
        struct Implementation : public Color::Interface {
            Implementation(const ColorElement* self) : self(self) {}

            std::size_t loadIndex = npos;
            const ColorElement* self;
            Animated<float> r;
            Animated<float> g;
            Animated<float> b;
            Animated<float> a;
            ExpressionParser::ValueMap valuesCache;
            View::State state = View::State::NoState;
            bool changingCache = false;

            Kaixo::Color get(View::State s = View::State::Default, const ExpressionParser::ValueMap& values = {}) override {
                const auto reassign = [&] {
                    auto re = self->r[s];
                    auto ge = self->g[s];
                    auto be = self->b[s];
                    auto ae = self->a[s];

                    if (re.value) r = { re.value(values), re.transition, std::move(re.curve) };
                    else r = { 0, 0 };
                    if (ge.value) g = { ge.value(values), ge.transition, std::move(ge.curve) };
                    else g = { 0, 0 };
                    if (be.value) b = { be.value(values), be.transition, std::move(be.curve) };
                    else b = { 0, 0 };
                    if (ae.value) a = { ae.value(values), ae.transition, std::move(ae.curve) };
                    else a = { 0, 0 };
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

                changingCache = r.changing() || g.changing() || b.changing() || a.changing();\

                return { r.get(), g.get(), b.get(), a.get() };
            }

            bool changing() const override { return changingCache; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}
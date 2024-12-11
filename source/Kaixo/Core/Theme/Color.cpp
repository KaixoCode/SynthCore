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
            if (theme.is<basic_json::number_t>()) return val = [v = theme.as<float>()](auto&) { return v; }, true;
            if (theme.is<basic_json::string_t>()) return val = ExpressionParser::parse(theme.as<std::string_view>(), self->functions), true;
            return false;
        };

        const auto extractHex = [](ExpressionParser::Expression& val, const basic_json& clr, std::size_t index) -> bool {
            if (index >= 4) return false;
            auto str = clr.as<std::string_view>();
            str = str.substr(1);
            for (char c : str) {
                auto ch = std::tolower(c);
                if (!oneOf(ch, "0123456789abcdef")) return false; // Invalid
            }

            bool nullPad = false;
            switch (str.size()) {
            case 3: if (index == 3) return val = [](auto&) { return 255; }, true;
                [[fallthrough]];
            case 4: nullPad = true, str = str.substr(index, 1); break;
            case 6: if (index == 3) return val = [](auto&) { return 255; }, true;
                [[fallthrough]];
            case 8: str = str.substr(index * 2, 2); break;
            }

            std::size_t result = 0;
            for (char c : str) {
                auto ch = std::tolower(c);

                std::size_t value =
                      ch == '0' ?  0 : ch == '1' ?  1 : ch == '2' ?  2 : ch == '3' ?  3
                    : ch == '4' ?  4 : ch == '5' ?  5 : ch == '6' ?  6 : ch == '7' ?  7
                    : ch == '8' ?  8 : ch == '9' ?  9 : ch == 'a' ? 10 : ch == 'b' ? 11
                    : ch == 'c' ? 12 : ch == 'd' ? 13 : ch == 'e' ? 14 : ch == 'f' ? 15 : 0;

                result <<= 4;
                result += value;
            }

            if (nullPad) result <<= 4;
            val = [val = result](auto&) { return val; };
            return true;
        };
        
        const auto isHexColor = [](const basic_json& clr) -> bool {
            return clr.is<basic_json::string_t>()
                && clr.as<std::string_view>().starts_with("#")
                && (clr.as<std::string_view>().size() == 4   // #rgb
                 || clr.as<std::string_view>().size() == 5   // #rgba
                 || clr.as<std::string_view>().size() == 7   // #rrggbb
                 || clr.as<std::string_view>().size() == 9); // #rrggbbaa
        };

        const auto isValidColor = [](const basic_json& clr) -> bool {
            return clr.is<basic_json::array_t>() && !clr.empty() && clr.size() <= 4;
        };

        const auto parseRed = [&](ExpressionParser::Expression& red, const basic_json& theme, View::State) -> bool {
            return isHexColor(theme) && extractHex(red, theme, 0) 
                || isValidColor(theme) && parseExpressionOrNumber(red, theme[0]);
        };
        
        const auto parseGreen = [&](ExpressionParser::Expression& green, const basic_json& theme, View::State) -> bool {
            return isHexColor(theme) && extractHex(green, theme, 1)
                || isValidColor(theme) && parseExpressionOrNumber(green, theme[theme.size() >= 3 ? 1 : 0]);
        };
        
        const auto parseBlue = [&](ExpressionParser::Expression& blue, const basic_json& theme, View::State) -> bool {
            return isHexColor(theme) && extractHex(blue, theme, 2)
                || isValidColor(theme) && parseExpressionOrNumber(blue, theme[theme.size() >= 3 ? 2 : 0]);
        };
        
        const auto parseAlpha = [&](ExpressionParser::Expression& alpha, const basic_json& theme, View::State) -> bool {
            if (isHexColor(theme) && extractHex(alpha, theme, 3)) return true;
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
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
        
        auto parseRed = [&](ExpressionParser::Function& red, const basic_json& theme, View::State) -> bool {
            if (!theme.is(basic_json::Array)) return false;

            auto& arr = theme.as<basic_json::array>();
            if (arr.empty() || arr.size() > 4) return false;

            if (arr[0].is(basic_json::Number)) {
                red = [v = arr[0].as<float>()](auto&) { return v; };
            } else if (arr[0].is(basic_json::String)) {
                red = ExpressionParser::parse(arr[0].as<std::string>());
            } else {
                return false;
            }

            return true;
        };
        
        auto parseGreen = [&](ExpressionParser::Function& green, const basic_json& theme, View::State) -> bool {
            if (!theme.is(basic_json::Array)) return false;

            auto& arr = theme.as<basic_json::array>();
            if (arr.empty() || arr.size() > 4) return false;

            std::size_t index = 0;
            if (arr.size() == 3) index = 1;
            else if (arr.size() == 4) index = 1;

            if (arr[index].is(basic_json::Number)) {
                green = [v = arr[index].as<float>()](auto&) { return v; };
            } else if (arr[index].is(basic_json::String)) {
                green = ExpressionParser::parse(arr[index].as<std::string>());
            } else {
                return false;
            }

            return true;
        };
        
        auto parseBlue = [&](ExpressionParser::Function& blue, const basic_json& theme, View::State) -> bool {
            if (!theme.is(basic_json::Array)) return false;

            auto& arr = theme.as<basic_json::array>();
            if (arr.empty() || arr.size() > 4) return false;

            std::size_t index = 0;
            if (arr.size() == 3) index = 2;
            else if (arr.size() == 4) index = 2;

            if (arr[index].is(basic_json::Number)) {
                blue = [v = arr[index].as<float>()](auto&) { return v; };
            } else if (arr[index].is(basic_json::String)) {
                blue = ExpressionParser::parse(arr[index].as<std::string>());
            } else {
                return false;
            }

            return true;
        };
        
        auto parseAlpha = [&](ExpressionParser::Function& alpha, const basic_json& theme, View::State) -> bool {
            if (!theme.is(basic_json::Array)) return false;

            auto& arr = theme.as<basic_json::array>();
            if (arr.empty() || arr.size() > 4) return false;

            if (arr.size() == 1 || arr.size() == 3) {
                alpha = [](auto&) { return 255; };
                return true;
            }

            std::size_t index = 0;
            if (arr.size() == 2) index = 1;
            else if (arr.size() == 4) index = 3;

            if (arr[index].is(basic_json::Number)) {
                alpha = [v = arr[index].as<float>()](auto&) { return v; };
            } else if (arr[index].is(basic_json::String)) {
                alpha = ExpressionParser::parse(arr[index].as<std::string>());
            } else {
                return false;
            }

            return true;
        };

        // ------------------------------------------------
        
        r.interpret(theme, parseRed, state);
        g.interpret(theme, parseGreen, state);
        b.interpret(theme, parseBlue, state);
        a.interpret(theme, parseAlpha, state);

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

                    if (re.value) r = { re.value(values), re.transition };
                    else r = { 0, 0 };
                    if (ge.value) g = { ge.value(values), ge.transition };
                    else g = { 0, 0 };
                    if (be.value) b = { be.value(values), be.transition };
                    else b = { 0, 0 };
                    if (ae.value) a = { ae.value(values), ae.transition };
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
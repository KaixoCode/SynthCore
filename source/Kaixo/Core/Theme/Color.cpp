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

    Kaixo::Color Color::get(View::State state) const { return m_Graphics ? m_Graphics->get(state) : m_DefaultColor; }

    // ------------------------------------------------

    void ColorElement::interpret(const basic_json& theme, View::State state) {
        
        // ------------------------------------------------
        
        auto parseColor = [&](Kaixo::Color& color, const basic_json& theme) -> bool {

            // ------------------------------------------------

            if (!theme.is(basic_json::Array)) return false;

            // ------------------------------------------------

            float red = 0, green = 0, blue = 0, alpha = 0;

            // ------------------------------------------------

            auto& arr = theme.as<basic_json::array>();

            // ------------------------------------------------

            if (arr.size() == 1 &&
                arr[0].is(basic_json::Number))
            {
                red = arr[0].as<float>();
                green = arr[0].as<float>();
                blue = arr[0].as<float>();
                alpha = 255;
            }

            // ------------------------------------------------

            else if (arr.size() == 2 &&
                arr[0].is(basic_json::Number) &&
                arr[1].is(basic_json::Number))
            {
                red = arr[0].as<float>();
                green = arr[0].as<float>();
                blue = arr[0].as<float>();
                alpha = arr[1].as<float>();
            }

            // ------------------------------------------------

            else if (arr.size() == 3 &&
                arr[0].is(basic_json::Number) &&
                arr[1].is(basic_json::Number) &&
                arr[2].is(basic_json::Number))
            {
                red = arr[0].as<float>();
                green = arr[1].as<float>();
                blue = arr[2].as<float>();
                alpha = 255;
            }

            // ------------------------------------------------

            else if (arr.size() == 4 &&
                arr[0].is(basic_json::Number) &&
                arr[1].is(basic_json::Number) &&
                arr[2].is(basic_json::Number) &&
                arr[3].is(basic_json::Number))
            {
                red = arr[0].as<float>();
                green = arr[1].as<float>();
                blue = arr[2].as<float>();
                alpha = arr[3].as<float>();
            }

            // ------------------------------------------------

            else return false;

            // ------------------------------------------------
            
            color = { red, green, blue, alpha };
            
            // ------------------------------------------------
            
            return true;

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        color.interpret(theme, [&](Kaixo::Color& color, const basic_json& theme, View::State state) {
            if (parseColor(color, theme)) return true;
            if (theme.contains("color") && parseColor(color, theme["color"])) return true;
            return false;
        }, state);

        // ------------------------------------------------
        
        loadIndex++;

        // ------------------------------------------------

    }

    void ColorElement::interpret(const basic_json& theme) {
        color.reset();
        interpret(theme, View::State::Default);
    }

    // ------------------------------------------------
    
    ColorElement::operator Color() const {
        struct Implementation : public Color::Interface {
            Implementation(const ColorElement* self) : self(self) {}

            std::size_t loadIndex = npos;
            const ColorElement* self;
            Animated<Kaixo::Color> color;
            View::State state = static_cast<View::State>(-1);
            bool changingCache = false;

            Kaixo::Color get(View::State s = View::State::Default) override {
                if (loadIndex != self->loadIndex) {
                    loadIndex = self->loadIndex;
                    color = self->color[s];
                }

                if (state != s) {
                    color = self->color[s];
                    state = s;
                }
                changingCache = color.changing();
                return color.get();
            }

            bool changing() const override { return changingCache; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}
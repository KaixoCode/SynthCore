#include "Kaixo/Core/Theme/Color.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Color::Color(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    void ColorElement::interpret(const basic_json& theme) {

        // ------------------------------------------------

        if (!theme.is(basic_json::Array)) return;

        // ------------------------------------------------

        std::uint8_t red = 0, green = 0, blue = 0, alpha = 0;

        // ------------------------------------------------

        auto& arr = theme.as<basic_json::array>();

        // ------------------------------------------------

        if (arr.size() == 1 &&
            arr[0].is(basic_json::Number))
        {
            red = arr[0].as<std::uint8_t>();
            green = arr[0].as<std::uint8_t>();
            blue = arr[0].as<std::uint8_t>();
            alpha = 255;
        } 

        // ------------------------------------------------

        else if (arr.size() == 2 &&
            arr[0].is(basic_json::Number) &&
            arr[1].is(basic_json::Number))
        {
            red = arr[0].as<std::uint8_t>();
            green = arr[0].as<std::uint8_t>();
            blue = arr[0].as<std::uint8_t>();
            alpha = arr[1].as<std::uint8_t>();
        } 

        // ------------------------------------------------

        else if (arr.size() == 3 &&
            arr[0].is(basic_json::Number) &&
            arr[1].is(basic_json::Number) &&
            arr[2].is(basic_json::Number)) 
        {
            red = arr[0].as<std::uint8_t>();
            green = arr[1].as<std::uint8_t>();
            blue = arr[2].as<std::uint8_t>();
            alpha = 255;
        }

        // ------------------------------------------------

        else if (arr.size() == 4 &&
            arr[0].is(basic_json::Number) &&
            arr[1].is(basic_json::Number) &&
            arr[2].is(basic_json::Number) &&
            arr[3].is(basic_json::Number)) 
        {
            red = arr[0].as<std::uint8_t>();
            green = arr[1].as<std::uint8_t>();
            blue = arr[2].as<std::uint8_t>();
            alpha = arr[3].as<std::uint8_t>();
        }

        // ------------------------------------------------

        color = { red, green, blue, alpha };

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    ColorElement::operator Color() const {
        struct Implementation : public Color::Interface {
            Implementation(const ColorElement* self) : self(self) {}

            const ColorElement* self;

            juce::Colour get() const override { return *self; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}
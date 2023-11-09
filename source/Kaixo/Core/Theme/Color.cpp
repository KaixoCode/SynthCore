#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Color::Color(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    void ColorElement::interpret(const json& theme) {

        // ------------------------------------------------

        if (!theme.is(json::Array)) return;

        // ------------------------------------------------

        std::uint8_t red = 0, green = 0, blue = 0, alpha = 0;

        // ------------------------------------------------

        auto& arr = theme.as<json::array>();

        // ------------------------------------------------

        if (arr.size() == 1 &&
            arr[0].is(json::Unsigned)) 
        {
            red = arr[0].as<json::unsigned_integral>();
            green = arr[0].as<json::unsigned_integral>();
            blue = arr[0].as<json::unsigned_integral>();
            alpha = 255;
        } 

        // ------------------------------------------------

        else if (arr.size() == 2 &&
            arr[0].is(json::Unsigned) &&
            arr[1].is(json::Unsigned)) 
        {
            red = arr[0].as<json::unsigned_integral>();
            green = arr[0].as<json::unsigned_integral>();
            blue = arr[0].as<json::unsigned_integral>();
            alpha = arr[1].as<json::unsigned_integral>();
        } 

        // ------------------------------------------------

        else if (arr.size() == 3 &&
            arr[0].is(json::Unsigned) &&
            arr[1].is(json::Unsigned) &&
            arr[2].is(json::Unsigned)) 
        {
            red = arr[0].as<json::unsigned_integral>();
            green = arr[1].as<json::unsigned_integral>();
            blue = arr[2].as<json::unsigned_integral>();
            alpha = 255;
        }

        // ------------------------------------------------

        else if (arr.size() == 4 &&
            arr[0].is(json::Unsigned) &&
            arr[1].is(json::Unsigned) &&
            arr[2].is(json::Unsigned) &&
            arr[3].is(json::Unsigned)) 
        {
            red = arr[0].as<json::unsigned_integral>();
            green = arr[1].as<json::unsigned_integral>();
            blue = arr[2].as<json::unsigned_integral>();
            alpha = arr[3].as<json::unsigned_integral>();
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
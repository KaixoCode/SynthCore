#pragma once

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    struct Color {

        // ------------------------------------------------

        float r = 0;
        float g = 0;
        float b = 0;
        float a = 0;

        // ------------------------------------------------

        operator juce::Colour() const { 
            return juce::Colour::fromRGBA(
                static_cast<std::uint8_t>(std::clamp(r, 0.f, 255.f)),
                static_cast<std::uint8_t>(std::clamp(g, 0.f, 255.f)), 
                static_cast<std::uint8_t>(std::clamp(b, 0.f, 255.f)), 
                static_cast<std::uint8_t>(std::clamp(a, 0.f, 255.f))
            );
        }

        // ------------------------------------------------
        
        friend Color operator+(Color a, Color b) { return { a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a }; };
        friend Color operator-(Color a, Color b) { return { a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a }; };
        friend Color operator*(Color a, float n) { return { a.r * n, a.g * n, a.b * n, a.a * n }; };
        friend Color operator*(float n, Color a) { return { a.r * n, a.g * n, a.b * n, a.a * n }; };

        // ------------------------------------------------

        template<class ...Args>
            requires (std::same_as<std::decay_t<Args>, Color> && ...)
        constexpr static Color lerp(float l, Args&& ...args) {
            constexpr std::size_t count = sizeof...(Args);
            const float progress = Math::Fast::clamp((count - 1) * l, 0, count - 1);
            Color result{ 0, 0, 0, 0 };
            std::size_t index = 0;
            float ratio = 0;
            ((ratio = 1 - Math::Fast::min(1, Math::Fast::abs(progress - index)),
                result.r += ratio * args.r,
                result.g += ratio * args.g,
                result.b += ratio * args.b,
                result.a += ratio * args.a,
                ++index
                ), ...);

            return result;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
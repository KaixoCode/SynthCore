#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * {
     *     "map" : "relative/path/to/fontmap.png",
     *
     *     "description" : "relative/path/to/description.json", // Load description from json file
     *     "description" : {                                    // Description is embedded in this json
     *         "a" : { 
     *             "location" : [0u, 0u, 0u, 0u], // Location of glyph in font map
     *             "pre-spacing"  : 0i,           // Number of pixels of spacing before glyph
     *             "post-spacing" : 0i,           // Number of pixels of spacing after glyph
     *             "exceptions" : [ // List of spacing exceptions
     *                 {
     *                     "after" : [ "a" ], // This exception occurs when the letter is after any character in this list
     *                     "before": [ "a" ], // This exception occurs when the letter is before any character in this list
     *                     "pre-spacing" : 0i, // Pre-spacing for this exception
     *                     "post-spacing" : 0i, // Post-spacing for this exception
     *                 }
     *             ]
     *         }
     *     }
     * }
     */

    // ------------------------------------------------
    
    class Font {
    public:

        // ------------------------------------------------

        struct Interface {
            virtual float fontSize() const = 0;
            virtual float stringWidth(std::string_view) const = 0;
            virtual void draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align = Align::TopLeft, bool fillAlphaWithColor = false) const = 0;
        };

        // ------------------------------------------------

        Font() = default;
        Font(std::unique_ptr<Interface> graphics);

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------
        
        float fontSize() const;
        
        // ------------------------------------------------
        
        float stringWidth(std::string_view str) const;

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align = Align::TopLeft, bool fillAlphaWithColor = false) const;

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class FontElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        struct Letter {
            constexpr static std::int64_t NoSpacing = std::numeric_limits<std::int64_t>::max();

            Rect<int> clip{ 0, 0, 0, 0 };
            std::int64_t preSpacing = 0;
            std::int64_t postSpacing = 2;

            struct Exception {
                std::set<char> after;
                std::set<char> before;
                std::int64_t preSpacing = NoSpacing;
                std::int64_t postSpacing = NoSpacing;
            };

            std::vector<Exception> exceptions{};

            std::int64_t calcPreSpacing(char before, char after) const;
            std::int64_t calcPostSpacing(char before, char after) const;
        };

        // ------------------------------------------------

        FontID font = NoFont; // Actual font file
        float size = 0;
        ImageID id = NoImage;
        std::map<char, Letter> charMap{};
        float maxHeight = 0;
        std::int64_t defaultSpacing = 2;

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align = Align::TopLeft, bool fillAlphaWithColor = false) const;

        // ------------------------------------------------

        float stringWidth(std::string_view str) const;
        std::string fitWithinWidth(std::string_view str, float maxWidth, std::string_view delim = "...") const;

        // ------------------------------------------------

        operator Font() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator Font() {
        if (auto font = dynamic_cast<FontElement*>(m_Element)) {
            return *font;
        }

        return {};
    }

    // ------------------------------------------------

}
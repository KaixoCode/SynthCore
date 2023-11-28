#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * [{
     *     "layers": [{
     *         "image": "relative/path/to/image.png",
     *
     *         "offset": [0u, 0u],         // Offset in image
     *         "size"  : [0u, 0u],         // Size of sub-image (and also frame)
     *         "clip"  : [0u, 0u, 0u, 0u], // Clip sub-image
     *
     *         "edges" : [0u, 0u],         // Nine-tiled x and y
     *         "edges" : [0u, 0u, 0u, 0u], // Nine-tiled all edges separate
     *
     *         "background-color": some color, // background fill color
     *
     *         "text": "some string", 
     *         "font": some font,
     *         "text-color": some color,  // Color of the text
     *         "text-offset": [0u, 0u],
     *         "text-align": "top-left | top-right | top-center | center-left | center | center-right | bottom-left | bottom-center | bottom-left",
     *     }],
     *
     *     "states" : [    // This image is selected when all these states match
     *         "hovering",
     *         "pressed",
     *         "selected",
     *         "disabled",
     *         "enabled",
     *         "focused",
     *     ]
     * }]
     * }]
     */

    // ------------------------------------------------
    
    class Stateful {
    public:
    
        // ------------------------------------------------
        
        struct Interface {
            virtual void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const = 0;
        };

        // ------------------------------------------------

        Stateful() = default;
        Stateful(std::unique_ptr<Interface> graphics);

        // ------------------------------------------------
        
        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const;

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class StatefulElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        struct State {

            // ------------------------------------------------

            Theme* self;

            // ------------------------------------------------

            struct Layer {

                // ------------------------------------------------

                Layer(Theme* self) : self(self) {}

                // ------------------------------------------------

                Theme* self;

                // ------------------------------------------------

                ImageID id = NoImage;
                Rect<int> clip{ 0, 0, 0, 0 };
                Point<int> imagePosition{ 0, 0 };
                bool isTiled = false;
                TiledDescription tiles{};

                // ------------------------------------------------

                std::string text{};
                Point<int> textOffset{ 0, 0 };
                Align textAlign = Align::Center;
                FontElement font{ self };
                bool fillAlphaWithColor = false;
                ColorElement textColor{ self };

                // ------------------------------------------------

                bool hasBackgroundColor = false;
                ColorElement backgroundColor{ self };

                // ------------------------------------------------

                bool draw(juce::Graphics& g, const Rect<float>& pos, Align align = Align::Center) const;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            std::vector<Layer> layers;
            View::State state = View::State::Default;

            // ------------------------------------------------

            void interpret(const basic_json& theme);
            
            // ------------------------------------------------

            bool draw(juce::Graphics& g, const Rect<float>& pos, Align align = Align::TopLeft) const;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::vector<State> states{};

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const;

        // ------------------------------------------------

        operator Stateful() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    std::vector<std::pair<std::string, View::State>> interpretState(const basic_json& theme);

    // ------------------------------------------------

}
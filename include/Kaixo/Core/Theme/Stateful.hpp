#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Theme/Image.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * [{
     *     "image" : "relative/path/to/image.png",
     *
     *     "states" : [    // This image is selected when all these states match
     *         "hovering",
     *         "pressed",
     *         "selected",
     *         "disabled",
     *         "enabled",
     *         "focused",
     *     ],
     *
     *     "offset": [0u, 0u],         // Offset in image
     *     "size"  : [0u, 0u],         // Size of sub-image (and also frame)
     *     "clip"  : [0u, 0u, 0u, 0u], // Clip sub-image
     *
     *     "edges" : [0u, 0u],         // Nine-tiled x and y
     *     "edges" : [0u, 0u, 0u, 0u], // Nine-tiled all edges separate
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

            ImageID id = NoImage;
            View::State state = View::State::Default;
            Rect<int> clip{ 0, 0, 0, 0 };
            bool isTiled = false;
            TiledDescription tiles{};

            // ------------------------------------------------

            void interpret(const json& theme, ZoomMultiplier zoom);
            
            // ------------------------------------------------

            bool draw(juce::Graphics& g, const Rect<float>& pos, Align align = Align::TopLeft) const;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct ZoomLevel {
            std::vector<State> states{};
        };

        // ------------------------------------------------

        std::map<ZoomMultiplier, ZoomLevel> zoomLevel{};

        // ------------------------------------------------

        void interpret(const json& theme) override;

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const;

        // ------------------------------------------------

        operator Stateful() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    View::State interpretState(const json& theme);

    // ------------------------------------------------

}
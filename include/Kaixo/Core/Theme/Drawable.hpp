#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/Stateful.hpp"
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
     *         "text": [ "some string", "$value", "$name", "$short-name" ], // variables are related to linked param
     *         "font": some font,
     *         "text-color": some color,  // Color of the text
     *         "text-offset": [0u, 0u],
     *         "text-align": "top-left | top-right | top-center | center-left | center | center-right | bottom-left | bottom-center | bottom-left",
     *
     *         "start-angle": 0,       // Rotate the layer starting with this angle
     *         "end-angle": 360,       // and ending with this angle (degrees)
     *
     *         "frames"         : 0u, // Number of frames in multi-frame
     *         "frames-per-row" : 0u, // Number of frames per row
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
     *
     */

     // ------------------------------------------------

    class Drawable {
    public:

        // ------------------------------------------------
        
        struct Instruction {
            juce::Graphics& graphics;
            Rect<float> position;
            ParamValue value = -1; // Range must be [0, inf), -1 means not set
            std::size_t index = npos;
            View::State state = View::State::Default;
            Align align = Align::TopLeft;
        };

        // ------------------------------------------------

        struct Interface {
            virtual void draw(Instruction) const = 0;
            virtual void link(ParamID id) = 0;
        };

        // ------------------------------------------------

        Drawable() = default;
        Drawable(std::unique_ptr<Interface> graphics) : m_Graphics(graphics) {}

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(Instruction instr) const { if (m_Graphics) m_Graphics->draw(std::move(instr)); }
        void link(ParamID id) { if (m_Graphics) m_Graphics->link(id); }

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class DrawableElement : Element {
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

                Layer(Theme* self, DrawableElement* parent) : self(self), parent(parent) {}

                // ------------------------------------------------

                DrawableElement* parent;
                Theme* self;

                // ------------------------------------------------

                ImageID id = NoImage;
                MultiFrameDescription description{};
                Rect<int> clip{ 0, 0, 0, 0 };
                Rect<int> imagePosition{ 0, 0, 0, 0 };
                bool isTiled = false;
                TiledDescription tiles{};

                // ------------------------------------------------

                std::vector<std::string> text{};
                Point<int> textOffset{ 0, 0 };
                Align textAlign = Align::Center;
                FontElement font{ self };
                bool fillAlphaWithColor = false;
                ColorElement textColor{ self };

                // ------------------------------------------------

                bool hasBackgroundColor = false;
                ColorElement backgroundColor{ self };

                // ------------------------------------------------

                bool draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align align = Align::Center) const;

                // ------------------------------------------------

            };

            // ------------------------------------------------

            std::vector<Layer> layers;
            View::State state = View::State::Default;
            std::size_t frames = 1;

            // ------------------------------------------------

            void interpret(const basic_json& theme, DrawableElement* parent);

            // ------------------------------------------------

            bool draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align align = Align::Center) const;
            bool draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, Align align = Align::Center) const;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::vector<State> states{};
        ParamID parameter = NoParam;

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;
        void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;

        // ------------------------------------------------

        operator Drawable();

        // ------------------------------------------------

        class Index {
        public:

            // ------------------------------------------------

            void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;

            // ------------------------------------------------

            operator Stateful() const;

            // ------------------------------------------------

        private:
            const DrawableElement* m_Self;
            std::size_t m_Index;

            // ------------------------------------------------

            Index(const DrawableElement* self, std::size_t index)
                : m_Self(self), m_Index(index) {}

            // ------------------------------------------------

            friend class DrawableElement;
        };

        // ------------------------------------------------

        Index operator[](std::size_t i) const { return { this, i }; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    MultiFrameDescription interpretMultiFrame(const basic_json& theme);

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator Drawable() {
        if (auto multiframe = dynamic_cast<DrawableElement*>(m_Element)) {
            return *multiframe;
        }

        return {};
    }

    //inline Stateful DynamicElement::operator[](std::size_t index) {
    //    if (auto multiframe = dynamic_cast<DrawableElement*>(m_Element)) {
    //        return (*multiframe)[index];
    //    }

    //    return {};
    //}

    // ------------------------------------------------

}
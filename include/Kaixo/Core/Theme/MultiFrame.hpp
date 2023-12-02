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
     *         "start-angle": 0,       // Rotate the layer starting with this angle
     *         "end-angle": 360,       // and ending with this angle (degrees)
     *         
     *         "frames"         : 0u, // Number of frames in multi-frame
     *         "frames-per-row" : 0u, // Number of frames per row 
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

    class MultiFrame {
    public:

        // ------------------------------------------------

        struct Interface {
            virtual void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const = 0;
            virtual void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const = 0;
            virtual void link(ParamID id) = 0;
        };

        // ------------------------------------------------

        MultiFrame() = default;
        MultiFrame(std::unique_ptr<Interface> graphics);

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const;
        void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const;

        // ------------------------------------------------
        
        void link(ParamID id);

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class MultiFrameElement : Element {
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
                
                Layer(Theme* self, MultiFrameElement* parent) : self(self), parent(parent) {}

                // ------------------------------------------------

                MultiFrameElement* parent;
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

            void interpret(const basic_json& theme, MultiFrameElement* parent);

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

        operator MultiFrame();

        // ------------------------------------------------

        class Index {
        public:

            // ------------------------------------------------

            void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;

            // ------------------------------------------------

            operator Stateful() const;

            // ------------------------------------------------

        private:
            const MultiFrameElement* m_Self;
            std::size_t m_Index;

            // ------------------------------------------------

            Index(const MultiFrameElement* self, std::size_t index)
                : m_Self(self), m_Index(index) {}

            // ------------------------------------------------

            friend class MultiFrameElement;
        };

        // ------------------------------------------------

        Index operator[](std::size_t i) const { return { this, i }; }

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    MultiFrameDescription interpretMultiFrame(const basic_json& theme);

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator MultiFrame() {
        if (auto multiframe = dynamic_cast<MultiFrameElement*>(m_Element)) {
            return *multiframe;
        }

        return {};
    }

    inline Stateful DynamicElement::operator[](std::size_t index) {
        if (auto multiframe = dynamic_cast<MultiFrameElement*>(m_Element)) {
            return (*multiframe)[index];
        }

        return {};
    }


    // ------------------------------------------------

}
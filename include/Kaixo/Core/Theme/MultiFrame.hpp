#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/Stateful.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * [{
     *     "image" : "relative/path/to/image.png",
     *
     *     "frames"         : 0u, // Number of frames in multi-frame
     *     "frames-per-row" : 0u, // Number of frames per row 
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

    class MultiFrame {
    public:

        // ------------------------------------------------

        struct Interface {
            virtual void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const = 0;
            virtual void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::TopLeft) const = 0;
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

            ImageID id = NoImage;
            View::State state = View::State::Default;
            MultiFrameDescription description{};
            Rect<int> clip{ 0, 0, 0, 0 };
            bool isTiled = false;
            TiledDescription tiles{};
            
            // ------------------------------------------------

            void interpret(const json& theme, ZoomMultiplier zoom);

            // ------------------------------------------------

            bool draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align align = Align::Center) const;
            bool draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, Align align = Align::Center) const;

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

        void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;
        void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const;

        // ------------------------------------------------

        operator MultiFrame() const;

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
    
    MultiFrameDescription interpretMultiFrame(const json& theme);

    // ------------------------------------------------

}
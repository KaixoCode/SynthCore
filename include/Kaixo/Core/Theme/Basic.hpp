#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    /*
     * {
     *     "image" : "relative/path/to/image.png",
     * 
     *     "offset": [0u, 0u],         // Offset in image
     *     "size"  : [0u, 0u],         // Size of sub-image
     *     "clip"  : [0u, 0u, 0u, 0u], // Clip sub-image
     * 
     *     "edges" : [0u, 0u],         // Nine-tiled x and y
     *     "edges" : [0u, 0u, 0u, 0u], // Nine-tiled all edges separate
     * }
     */

    // ------------------------------------------------

    class Basic {
    public:

        // ------------------------------------------------

        struct Interface {
            virtual void draw(juce::Graphics& g, const Rect<float>& pos) const = 0;
        };

        // ------------------------------------------------

        Basic() = default;
        Basic(std::unique_ptr<Interface> graphics);

        // ------------------------------------------------

        operator bool() const { return m_Graphics.operator bool(); }

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Rect<float>& pos) const;

        // ------------------------------------------------

    private:
        std::unique_ptr<Interface> m_Graphics{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class BasicElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        struct ZoomLevel {
            ImageID id = NoImage;
            bool isTiled = false;
            Rect<int> clip{ 0, 0, 0, 0 };
            TiledDescription tiles{};
        };

        // ------------------------------------------------

        std::map<ZoomMultiplier, ZoomLevel> zoomLevel{ { 1, {} } };

        // ------------------------------------------------

        void interpret(const json& theme) override;

        // ------------------------------------------------

        void draw(juce::Graphics& g, const Rect<float>& pos) const;

        // ------------------------------------------------
        
        operator Basic() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    Rect<int> interpretClip(const json& theme);
    std::optional<TiledDescription> interpretTiles(const json& theme);

    // ------------------------------------------------

}
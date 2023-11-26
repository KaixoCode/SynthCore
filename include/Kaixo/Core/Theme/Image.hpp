#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {
    
    // ------------------------------------------------
    
    enum Align : std::int8_t {
        None         = 0,
        Left         = 1 << 0,
        Right        = 2 << 0,
        CenterX      = 3 << 0,
        Top          = 1 << 2,
        Bottom       = 2 << 2,
        CenterY      = 3 << 2,
        TopLeft      = Top     | Left,
        TopRight     = Top     | Right,
        TopCenter    = Top     | CenterX,
        BottomLeft   = Bottom  | Left,
        BottomRight  = Bottom  | Right,
        BottomCenter = Bottom  | CenterX,
        CenterLeft   = CenterY | Left,
        CenterRight  = CenterY | Right,
        Center       = CenterY | CenterX,
        X            = Left    | Right   | CenterX,
        Y            = Top     | Bottom  | CenterY,
    };

    // ------------------------------------------------
    
    struct TiledDescription {
        std::uint64_t left = 0;
        std::uint64_t top = 0;
        std::uint64_t right = 0;
        std::uint64_t bottom = 0;
    };
    
    struct TiledInstruction {
        juce::Graphics&         graphics;
        const TiledDescription& description;

        Rect<int>               clip     = { 0, 0, 0, 0 };
        Rect<float>             position = { 0, 0, 0, 0 };

        bool                    fillAlphaWithColor = false;
    };

    // ------------------------------------------------
    
    struct MultiFrameDescription {
        std::size_t numFrames = 1;
        std::size_t framesPerRow = 1;
    };

    struct FrameInstruction {
        juce::Graphics&              graphics;
        const MultiFrameDescription& description;

        const TiledDescription*      tiled    = nullptr;
        Rect<int>                    clip     = { 0, 0, 0, 0 };
        Align                        align    = Align::TopLeft;
        std::size_t                  frame    = 0;
        Rect<float>                  position = { 0, 0, 0, 0 };

        bool                         fillAlphaWithColor = false;
    };

    // ------------------------------------------------
    
    struct ClippedInstruction {
        juce::Graphics&         graphics;

        const TiledDescription* tiled    = nullptr;
        Rect<int>               clip     = { 0, 0, 0, 0 };
        Align                   align    = Align::TopLeft;
        Rect<float>             position = { 0, 0, 0, 0 };

        bool                    fillAlphaWithColor = false;
    };

    // ------------------------------------------------

    class Image {
    public:

        // ------------------------------------------------

        Image() : m_Image(), m_ZoomLevel(1) {}

        Image(juce::Image img, ZoomMultiplier zoom = 1)
            : m_Image(img), m_ZoomLevel(zoom) {}

        // ------------------------------------------------
        
        operator bool() const { return m_Image.isValid(); }

        // ------------------------------------------------

        void draw(TiledInstruction instruction) const;
        void draw(ClippedInstruction instruction) const;
        void draw(FrameInstruction instruction) const;

        // ------------------------------------------------
        
        juce::Image* operator->() { return &m_Image; }

        // ------------------------------------------------

    private:
        juce::Image m_Image;
        ZoomMultiplier m_ZoomLevel;
    };

    // ------------------------------------------------

    inline Align alignFromString(std::string_view align) {
        if (align == "top-left") return Align::TopLeft;
        else if (align == "top-center") return Align::TopCenter;
        else if (align == "top-right") return Align::TopRight;
        else if (align == "center-left") return Align::CenterLeft;
        else if (align == "center") return Align::Center;
        else if (align == "center-right") return Align::CenterRight;
        else if (align == "bottom-left") return Align::BottomLeft;
        else if (align == "bottom-center") return Align::BottomCenter;
        else if (align == "bottom-right") return Align::BottomRight;
        return Align::Center;
    }

    inline Point<float> pointFromAlign(Align align, Rect<float> rect) {
        if (align == Align::TopLeft) return rect.topLeft();
        else if (align == Align::TopCenter) return rect.topCenter();
        else if (align == Align::TopRight) return rect.topRight();
        else if (align == Align::CenterLeft) return rect.centerLeft();
        else if (align == Align::Center) return rect.center();
        else if (align == Align::CenterRight) return rect.centerRight();
        else if (align == Align::BottomLeft) return rect.bottomLeft();
        else if (align == Align::BottomCenter) return rect.bottomCenter();
        else if (align == Align::BottomRight) return rect.bottomRight();
        return rect.center();
    }

    // ------------------------------------------------

}
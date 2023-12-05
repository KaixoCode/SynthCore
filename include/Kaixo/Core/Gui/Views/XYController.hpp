#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class XYController : public View, public ParameterListener {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------
            
            Theme::Drawable background;
            Theme::Drawable graphics;

            // ------------------------------------------------
            
            Point<> padding{ 10, 10 };
            Coord handleSize = 20;

            // ------------------------------------------------
            
            std::function<std::string(ParamValue, ParamValue)> tooltip = nullptr;

            // ------------------------------------------------

            ParamID x = NoParam;
            ParamID y = NoParam;

            ParamValue valueX = 0.5;
            ParamValue valueY = 0.5;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        XYController(Context c, Settings s = {});

        // ------------------------------------------------

        void parameterChanged(ParamID id, ParamValue val) override;

        // ------------------------------------------------

        void mouseExit(const juce::MouseEvent& event) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

    private:
        Point<> m_PreviousMousePosition{ 0, 0 };

        // ------------------------------------------------

        Point<> handlePos();

        // ------------------------------------------------

        void beginEdit();
        void performEdit(ParamValue x, ParamValue y);
        void endEdit();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
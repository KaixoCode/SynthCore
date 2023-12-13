#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/PointsDisplay.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class LfoDisplay : public PointsDisplay {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PointsDisplay::Settings pointsDisplay{};

            // ------------------------------------------------

            Processing::InterfaceStorage<PointsDisplayInterface> interface{};

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        LfoDisplay(Context c, Settings s = {});

        // ------------------------------------------------
        
        float at(float x) override;
        std::size_t nofPoints() const override;
        Point getPoint(std::size_t i) override;
        void setPoint(std::size_t i, Point point) override;
        void addPoint(Point point) override;
        void removePoint(std::size_t i) override;
        float phase() override;

        // ------------------------------------------------

    };


    // ------------------------------------------------

}
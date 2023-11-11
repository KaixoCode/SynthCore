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
    
    class LfoInterface : public Processing::Interface {
    public:

        // ------------------------------------------------

        using Point = PointsDisplay::Point;

        // ------------------------------------------------

        virtual float at(float) = 0;
        virtual float phase() = 0;

        virtual std::size_t size() = 0;

        virtual Point get(std::size_t) = 0;
        virtual void set(std::size_t, Point) = 0;
        
        virtual void remove(std::size_t) = 0;
        virtual void add(Point) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class LfoDisplay : public PointsDisplay {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PointsDisplay::Settings pointsDisplay{};

            // ------------------------------------------------

            Processing::InterfaceStorage<LfoInterface> interface{};

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
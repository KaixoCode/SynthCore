#include "Kaixo/Core/Gui/Views/LfoDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    LfoDisplay::LfoDisplay(Context c, Settings s)
        : PointsDisplay(c, std::move(s.pointsDisplay)), settings(std::move(s))
    {
        PointsDisplay::settings.phase.display = true;
        PointsDisplay::settings.line.start = Height;
        PointsDisplay::settings.line.end = Height;
    }

    // ------------------------------------------------

    float LfoDisplay::at(float x) { return settings.interface->at(x); }
    std::size_t LfoDisplay::nofPoints() const { return settings.interface->size(); }
    LfoDisplay::Point LfoDisplay::getPoint(std::size_t i) { return settings.interface->get(i); }
    void LfoDisplay::setPoint(std::size_t i, Point point) { settings.interface->set(i, point); }
    void LfoDisplay::addPoint(Point point) { settings.interface->add(point); }
    void LfoDisplay::removePoint(std::size_t i) { settings.interface->remove(i); }
    float LfoDisplay::phase() { return settings.interface->phase(); }

    // ------------------------------------------------

}
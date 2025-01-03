#include "Kaixo/Core/Gui/Views/FilterDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    FilterDisplay::FilterDisplay(Context c, Settings s)
        : PointsDisplay(c, std::move(s.pointsDisplay)), settings(std::move(s))
    {
        PointsDisplay::settings.line.start = Height;
        PointsDisplay::settings.line.end = Height;
        PointsDisplay::settings.line.loop = false;
        PointsDisplay::settings.enableAddPoints = false;
        PointsDisplay::settings.keepPointsInOrder = false;
        PointsDisplay::settings.hasCurves = false;
        PointsDisplay::settings.grid.x = 0;
        PointsDisplay::settings.grid.y = 0;
    }

    // ------------------------------------------------

    float FilterDisplay::at(float x) {
        float decibels = 0;
        for (auto& filter : m_Filters)
            decibels += filter->decibelsAt(x);
        if (std::isnan(decibels)) return -1;
        if (std::isinf(decibels)) return -1;
        return decibels / 36 + 0.5;
    }

    std::size_t FilterDisplay::nofPoints() const { return m_Filters.size(); }

    FilterDisplay::Point FilterDisplay::getPoint(std::size_t i) {
        return {
            .x = m_Filters[i]->x(),
            .y = m_Filters[i]->y(),
        };
    }

    FilterDisplay::PointMetadata FilterDisplay::getPointMetadata(std::size_t i) {
        return {
            .disabled = !m_Filters[i]->enabled(),
            .xvalueText = m_Filters[i]->xText(),
            .yvalueText = m_Filters[i]->yText(),
        };
    }

    void FilterDisplay::setPoint(std::size_t i, Point point) {
        m_Filters[i]->x(point.x);
        m_Filters[i]->y(point.y);
    }

    void FilterDisplay::resetPoint(std::size_t i) {
        m_Filters[i]->reset();
    }

    // ------------------------------------------------

    void FilterDisplay::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        if (hoveringPoint() != npos) {
            float mult = 1;
            if (event.mods.isCtrlDown()) mult *= 0.25;
            if (event.mods.isShiftDown()) mult *= 0.25;
            m_Filters[hoveringPoint()]->addZ(d.deltaY * mult * 0.1);
        }
    }

    // ------------------------------------------------

}
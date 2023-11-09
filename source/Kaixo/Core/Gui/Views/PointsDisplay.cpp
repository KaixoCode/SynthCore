#include "Kaixo/Core/Gui/Views/PointsDisplay.hpp"
#include "Kaixo/Core/Storage.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    PointsDisplay::PointsDisplay(Context c, Settings s)
        : View(c), settings(std::move(s)) 
    {
        wantsIdle(true);
        m_UIPoints.resize(settings.maxPoints);
    }

    // ------------------------------------------------

    void PointsDisplay::mouseDoubleClick(const juce::MouseEvent& event) {
        if (m_Closest == npos) {
            addNewPointAtPosition({
                static_cast<float>(event.x),
                static_cast<float>(event.y)
            });

            return;
        }

        auto part = m_Closest;
        ensureUIPointsSize();        

        if (m_IsCurve) {
            resetCurve(part);
        } else {
            if (settings.enableAddPoints) {
                removePoint(part);
            }
            else {
                resetPoint(part);
            }
        }
    }

    void PointsDisplay::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        synchronizeValues();

        findClosest({
            static_cast<float>(event.x),
            static_cast<float>(event.y)
        });

        select(m_Closest);

        if (m_Closest == npos) {
            return;
        }

        m_DidDrag = false;

        auto part = m_Closest;
        ensureUIPointsSize();
        if (event.getNumberOfClicks() != 2) {

            if (!Storage::flag(Setting::TouchMode)) {
                setMouseCursor(juce::MouseCursor::NoCursor);
            }

            m_UIPoints[part].dragging = true;
        }

        m_PreviousMousePosition = event.mouseDownPosition;
    }

    void PointsDisplay::mouseExit(const juce::MouseEvent& event) {
        View::mouseExit(event);

        for (auto& p : m_UIPoints) p.hovering = false;
    }

    void PointsDisplay::mouseMove(const juce::MouseEvent& event) {
        View::mouseMove(event);

        findClosest({
            static_cast<float>(event.x),
            static_cast<float>(event.y)
        });

        repaint();
    }


    void PointsDisplay::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);

        auto parts = nofPoints();

        ensureUIPointsSize();
        for (std::size_t part = 0; part < parts; ++part) {
            if (m_UIPoints[part].dragging) {
                m_DidDrag = true;
                Kaixo::Point mult = { 1.f / width(), -1.f / height() };
                if (event.mods.isShiftDown()) mult *= 0.25;
                if (event.mods.isCtrlDown())  mult *= 0.25;

                Kaixo::Point value{
                    mult.x() * (event.x - m_PreviousMousePosition.x()),
                    mult.y() * (event.y - m_PreviousMousePosition.y())
                };

                if (!m_IsCurve && event.mods.isMiddleButtonDown()) value.x(0);
                if (!m_IsCurve && event.mods.isRightButtonDown()) value.y(0);

                addAmountToPoint(part, value, !event.mods.isAltDown());

                if (Storage::flag(Setting::TouchMode)) {
                    m_PreviousMousePosition = { event.x, event.y };
                } else {
                    context.cursorPos(localPointToGlobal(Kaixo::Point<float>{
                        m_PreviousMousePosition.x(),
                        m_PreviousMousePosition.y(),
                    }));

                    setMouseCursor(juce::MouseCursor::NoCursor);
                }

                repaint();
                return;
            }
        }
    }

    void PointsDisplay::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);

        auto parts = nofPoints();

        synchronizeValues();

        std::size_t selected = npos;
        ensureUIPointsSize();
        for (std::size_t part = 0; part < parts; ++part) {
            if (m_UIPoints[part].dragging) selected = part;
            m_UIPoints[part].dragging = false;
        }

        if (selected != npos && !Storage::flag(Setting::TouchMode)) {
            if (m_DidDrag) {
                Kaixo::Point p = m_IsCurve
                    ? positionOfCurvePoint(selected)
                    : positionOfPoint(selected);

                context.cursorPos(localPointToGlobal(p));
            }
        }

        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    // ------------------------------------------------

    void PointsDisplay::paint(juce::Graphics& g) {
        settings.background.draw(g, localDimensions());

        auto v = paddedDimensions();

        g.setColour(settings.grid.color);
        for (std::size_t i = 1; i < settings.grid.x; i++) {
            float x = v.x() + Math::trunc(0.5 * i * v.width() / settings.grid.x) * 2;
            g.fillRect(Rect<float>{ x, v.y(), 2, v.height() });
        }

        for (std::size_t i = 1; i < settings.grid.y; i++) {
            float y = v.y() + Math::trunc(0.5 * i * v.height() / settings.grid.y) * 2;
            g.fillRect(Rect<float>{ v.x(), y, v.width(), 2 });
        }

        if (settings.phase.display) {
            g.setColour(settings.phase.color);
            float x = v.x() + m_CurrentPhase * v.width();
            g.fillRect(Rect<float>{ x, v.y(), 2, v.height() });
        }

        // ------------------------------------------------

        if (settings.line.fill) {
            juce::Path path;
            if (settings.line.start) {
                path.startNewSubPath(Kaixo::Point<float>{ 
                    v.x(), 
                    v.y() + settings.line.start(v) 
                });
            } else {
                path.startNewSubPath(Kaixo::Point<float>{ 
                    v.x(),
                    v.y() + (1.f - at(0)) * v.height() 
                });
            }

            for (float x = 0; x <= v.width(); ++x) {
                float y = (1.f - at(x / v.width())) * v.height();
                path.lineTo(Kaixo::Point<float>{ x + v.x(), y + v.y() });
            }

            if (settings.line.end) {
                path.lineTo(Kaixo::Point<float>{ v.x() + v.width(), v.y() + settings.line.end(v) });
            }

            g.setColour(settings.line.fill);
            g.fillPath(path);
        }

        if (settings.line.stroke) {
            juce::Path path;
            path.startNewSubPath(Kaixo::Point<float>{ v.x(), v.y() + (1 - at(0)) * v.height() });

            for (float x = 0; x <= v.width(); ++x) {
                float y = (1.f - at(x / v.width())) * v.height();
                path.lineTo({ x + v.x(), y + v.y() });
            }

            g.setColour(settings.line.stroke);
            g.strokePath(path, juce::PathStrokeType{ settings.line.strokeWeight });
        }

        // ------------------------------------------------

        auto parts = nofPoints();
        ensureUIPointsSize();
        for (std::size_t i = 0; i < parts; ++i) {
            {
                State state = Default;
                if (!m_IsCurve) {
                    if (m_UIPoints[i].dragging) state |= Pressed;
                    if (m_UIPoints[i].hovering) state |= Hovering;
                }
                settings.mainPoint.draw(g, handleRectAt(positionOfPoint(i)), state);
            }

            if (shouldIgnoreCurve(i)) continue;

            {
                State state = Default;
                if (m_IsCurve) {
                    if (m_UIPoints[i].dragging) state |= Pressed;
                    if (m_UIPoints[i].hovering) state |= Hovering;
                }

                settings.curvePoint.draw(g, handleRectAt(positionOfCurvePoint(i)), state);
            }
        }
    }

    // ------------------------------------------------
    
    void PointsDisplay::onIdle() {
        View::onIdle();
        if (settings.phase.display) {
            auto newPhase = phase();
            if (m_CurrentPhase != newPhase) {
                m_CurrentPhase = newPhase;
                repaint(); 
            }
        }
    }

    // ------------------------------------------------

    void PointsDisplay::select(std::size_t i) {
        ensureUIPointsSize();

        for (auto& point : m_UIPoints)
            point.selected = false;

        if (i == npos) return;

        m_UIPoints[i].selected = true;

        for (auto& listener : m_Listeners)
            listener->select(i);

        for (auto& callback : m_Callbacks)
            callback(i);
    }

    // ------------------------------------------------
    
    void PointsDisplay::resetCurve(std::size_t i) {
        auto point = getPoint(i);
        point.c = 0;
        setPoint(i, point);
    }

    // ------------------------------------------------

    void PointsDisplay::hover(std::size_t i) {
        // m_UIPoints[i].hovering already happens in mouse callbacks
        for (auto& listener : m_Listeners)
            listener->hover(i);
    }

    // ------------------------------------------------
    
    void PointsDisplay::ensureUIPointsSize() {
        auto parts = nofPoints();
        if (m_UIPoints.size() < parts)
            m_UIPoints.resize(parts);
    }

    // ------------------------------------------------

    Rect<> PointsDisplay::paddedDimensions() {
        return {
            settings.padding.x(),
            settings.padding.y(),
            settings.line.loop // When not loop, don't add padding to right
                ? width() - settings.padding.x() * 2 
                : width() - settings.padding.x(),
            height() - settings.padding.y() * 2,
        };
    }

    // ------------------------------------------------

    float PointsDisplay::snapToGrid(float x, int n, float distance) {
        float minDistToSnap = distance;
        float snapped = Math::round(x * n) / n;
        float distFromSnap = Math::abs(snapped - x);
        if (distFromSnap < minDistToSnap) return snapped;
        return x;
    }

    float PointsDisplay::snapToGridX(float x) {
        if (settings.grid.x == 1) return x;
        return snapToGrid(x, settings.grid.x, settings.grid.snapDistance / width());
    }

    float PointsDisplay::snapToGridY(float y) {
        if (settings.grid.y == 1) return y;
        return snapToGrid(y, settings.grid.y, settings.grid.snapDistance / height());
    }

    // ------------------------------------------------

    void PointsDisplay::addAmountToPoint(std::size_t i, Kaixo::Point<float> amount, bool snap) {
        std::size_t parts = nofPoints();
        Point point = getPoint(i);
        UIPoint& uipoint = m_UIPoints[i];

        if (m_IsCurve) {
            float a = 5 * amount.y();
            float after = i + 1 == parts ? getPoint(0).y : getPoint(i + 1).y;

            if (point.y > after) a = -a;

            point.c = Math::clamp(point.c + a, -settings.maxCurve, settings.maxCurve);
        } else {
            float before = i == 0 ? 0 : getPoint(i - 1).x;
            float after = i + 1 == parts ? 1 : getPoint(i + 1).x;

            float newx = settings.keepPointsInOrder 
                ? Math::clamp(uipoint.x + amount.x(), before, after) 
                :             uipoint.x + amount.x();
            float newy = Math::clamp1(uipoint.y + amount.y());

            uipoint.x = newx;
            uipoint.y = newy;

            newx = settings.line.loop && i == 0 ? 0 : newx;
            
            if (snap) {
                if (amount.x() != 0) point.x = snapToGridX(newx);
                if (amount.y() != 0) point.y = snapToGridY(newy);
            } else {
                point.x = newx;
                point.y = newy;
            }
        }

        setPoint(i, point);
    }

    void PointsDisplay::addNewPointAtPosition(Kaixo::Point<float> position) {
        if (!settings.enableAddPoints) return;
        if (nofPoints() >= settings.maxPoints) return;

        auto v = paddedDimensions();
        float xpos = Math::clamp1((position.x() - v.x()) / v.width());
        float ypos = Math::clamp1(1 - (position.y() - v.y()) / v.height());

        addPoint({ .x = xpos, .y = ypos });
    }

    // ------------------------------------------------

    Kaixo::Point<float> PointsDisplay::positionOfCurvePoint(std::size_t i) {
        auto v = paddedDimensions();
        auto parts = nofPoints();
        auto point = getPoint(i);
        float next = i == parts - 1 ? 1 : getPoint(i + 1).x;
        float x = (point.x + next) / 2;
        float y = at(x);
        auto xpos = v.x() + x * v.width();
        auto ypos = v.y() + (1 - y) * v.height();
        return { xpos, ypos };
    }

    Kaixo::Point<float> PointsDisplay::positionOfPoint(std::size_t i) {
        auto v = paddedDimensions();
        auto point = getPoint(i);
        auto xpos = v.x() + point.x * v.width();
        auto ypos = v.y() + (1 - point.y) * v.height();
        return { xpos, ypos };
    }

    Rect<float> PointsDisplay::handleRectAt(Kaixo::Point<float> point) {
        return {
            point.x() - settings.size, 
            point.y() - settings.size,
            settings.size * 2, 
            settings.size * 2,
        };
    }

    // ------------------------------------------------

    void PointsDisplay::synchronizeValues() {
        auto parts = nofPoints();

        for (std::size_t i = 0; i < parts; ++i) {
            auto point = getPoint(i);
            m_UIPoints[i].x = point.x;
            m_UIPoints[i].y = point.y;
        }
    }

    // ------------------------------------------------

    bool PointsDisplay::shouldIgnoreCurve(std::size_t i) {
        if (!settings.hasCurves) return true;
        auto parts = nofPoints();

        if (i == parts - 1) {
            if (!settings.line.loop) return true;
            auto point = getPoint(i);
            return point.x == 1 || point.y == getPoint(0).y;
        }

        auto point = getPoint(i);
        auto next = i == parts - 1 ? getPoint(0) : getPoint(i + 1);
        return next.x == point.x || next.y == point.y;
    }

    void PointsDisplay::findClosest(Kaixo::Point<float> point) {
        auto parts = nofPoints();

        std::size_t closestNext = npos;
        float closestDistance = 1000;
        bool isClosestCurve = false;
        for (std::size_t part = 0; part < parts; ++part) {
            Kaixo::Point position1 = positionOfPoint(part);
            float distance1 = std::sqrt((position1.x() - point.x()) * (position1.x() - point.x())
                + (position1.y() - point.y()) * (position1.y() - point.y()));

            if (distance1 < closestDistance && distance1 < 20) {
                closestDistance = distance1;
                closestNext = part;
                isClosestCurve = false;
            }

            if (shouldIgnoreCurve(part)) continue;

            Kaixo::Point position2 = positionOfCurvePoint(part);
            float distance2 = std::sqrt((position2.x() - point.x()) * (position2.x() - point.x())
                + (position2.y() - point.y()) * (position2.y() - point.y()));

            if (distance2 < closestDistance && distance2 < 20) {
                closestDistance = distance2;
                closestNext = part;
                isClosestCurve = true;
            }
        }

        if (m_Closest != npos && m_Closest != closestNext && m_UIPoints[m_Closest].hovering) {
            m_UIPoints[m_Closest].hovering = false;
        }

        std::size_t previous = m_Closest;
        if (closestNext != npos) {
            m_UIPoints[closestNext].hovering = true;
            m_Closest = closestNext;
            m_IsCurve = isClosestCurve;
        } else {
            m_Closest = npos;
        }

        if (m_Closest != previous)
            hover(m_Closest);
    }

    // ------------------------------------------------

}

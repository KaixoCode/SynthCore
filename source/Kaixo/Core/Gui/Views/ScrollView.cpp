#include "Kaixo/Core/Gui/Views/ScrollView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    ScrollView::ScrollView(Context c, Settings s) 
        : View(c), settings(std::move(s)) 
    {}

    // ------------------------------------------------

    void ScrollView::mouseMove(const juce::MouseEvent& event) {
        bool overBar = scrollbar().contains(Point{ event.x, event.y });
        if (!hovering() && overBar) {
            hovering(true);
            repaint();
        }
        else if (hovering() && !overBar) {
            hovering(false);
            repaint();
        }
    }

    void ScrollView::mouseEnter(const juce::MouseEvent&) {} // Prevent View from setting hovering

    void ScrollView::mouseExit(const juce::MouseEvent&) {
        hovering(false);
    }

    void ScrollView::mouseDrag(const juce::MouseEvent& event) {
        if (pressed()) {
            switch (settings.type) {
            case Type::Vertical: {
                Coord delta = scrollSpace() * (event.y - event.mouseDownPosition.y) / (height() - barSize());
                m_Scrolled = Math::clamp(m_PressScrolled + delta, 0, scrollSpace());
                break;
            }
            case Type::Horizontal: {
                Coord delta = scrollSpace() * (event.x - event.mouseDownPosition.x) / (width() - barSize());
                m_Scrolled = Math::clamp(m_PressScrolled + delta, 0, scrollSpace());
                break;
            }
            }

            positionChildren();
        }
    }

    void ScrollView::mouseDown(const juce::MouseEvent& event) {
        if (hovering()) {
            pressed(true);
            m_PressScrolled = m_Scrolled;
        }
    }

    void ScrollView::mouseUp(const juce::MouseEvent& event) {
        pressed(false);
    }

    void ScrollView::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        if (scrollbarNecessary()) {
            m_Scrolled = Math::Fast::clamp(m_Scrolled - 100 * d.deltaY, 0, scrollSpace());
            positionChildren();
        }
        else {
            View::mouseWheelMove(event, d);
        }
    }

    // ------------------------------------------------

    void ScrollView::paint(juce::Graphics& g) {
        settings.background.draw({ 
            .graphics = g, 
            .bounds = localDimensions() 
        });
    }

    void ScrollView::paintOverChildren(juce::Graphics& g) {
        if (scrollbarNecessary()) {
            settings.scrollbar.draw({ 
                .graphics = g,
                .bounds = scrollbar(),
                .state = state() 
            });
        }
    }

    // ------------------------------------------------

    void ScrollView::updateDimensions() {
        View::updateDimensions();
        for (auto& view : views()) {
            view->evaluateDimensions(childrenRect());
        }
        positionChildren();
    }

    // ------------------------------------------------

    Coord ScrollView::scrollSpace() {
        switch (settings.type) {
        case Type::Vertical:   return m_HighestCoord - height() + settings.margin.height();
        case Type::Horizontal: return m_HighestCoord - width() + settings.margin.width();
        }
    }

    Coord ScrollView::barSize() {
        if (!scrollbarNecessary()) return 0;
        switch (settings.type) {
        case Type::Vertical: return Math::max((height() * height()) / m_HighestCoord, settings.minSize);
        case Type::Horizontal: return Math::max((width() * width()) / m_HighestCoord, settings.minSize);
        }
    }

    Coord ScrollView::barPosition() {
        switch (settings.type) {
        case Type::Vertical: return (height() - barSize()) * m_Scrolled / scrollSpace();
        case Type::Horizontal: return (width() - barSize()) * m_Scrolled / scrollSpace();
        }
    }

    Rect<> ScrollView::scrollbar() {
        auto size = barSize();
        auto position = barPosition();
        switch (settings.type) {
        case Type::Vertical: return {
            width() - settings.barThickness - settings.barPadding.width(),
            position + settings.barPadding.y(),
            settings.barThickness,
            size - settings.barPadding.y() - settings.barPadding.height(),
        };
        case Type::Horizontal: return {
            position + settings.barPadding.x(),
            height() - settings.barThickness - settings.barPadding.height(),
            size - settings.barPadding.x() - settings.barPadding.width(),
            settings.barThickness,
        };
        }
    }

    Rect<> ScrollView::childrenRect() {
        Point<> barSize = scrollbarNecessary() || settings.keepBarSpace
            ? Point {
                settings.barThickness + settings.barPadding.x() + settings.barPadding.width(),
                settings.barThickness + settings.barPadding.y() + settings.barPadding.height(),
            } 
            : Point{ settings.margin.width(), settings.margin.height() };

        switch (settings.type) {
        case Type::Vertical: return {
            settings.margin.x(),
            settings.margin.y(),
            width() - settings.margin.x() - barSize.x(),
            height() - settings.margin.y() - settings.margin.height(),
        };
        case Type::Horizontal: return {
            settings.margin.x(),
            settings.margin.y(),
            width() - settings.margin.x() - settings.margin.width(),
            height() - settings.margin.y() - barSize.y(),
        };
        }
    }

    void ScrollView::positionChildren() {
        auto rect = childrenRect();
        switch (settings.type) {
        case Type::Vertical: {
            Coord w = rect.width();
            Coord x = rect.x();
            Coord y = rect.y();
            bool first = true;
            for (auto& view : views()) {
                if (!view->isVisible()) continue;
                if (!first) y += settings.gap;
                y += view->height();
                first = false;
            }

            m_HighestCoord = Math::max(y, 1.);

            if (scrollbarNecessary()) m_Scrolled = Math::Fast::clamp(m_Scrolled, 0, scrollSpace());
            else m_Scrolled = 0;

            Coord scroll = Math::floor(m_Scrolled);

            y = rect.y() - scroll;
            for (auto& view : views()) {
                if (!view->isVisible()) continue;
                Coord h = view->height();

                switch (settings.alignChildren & Theme::Align::X) {
                case Theme::Align::CenterX: {
                    view->x(rect.x() + rect.width() / 2 - view->width() / 2);
                    break;
                }
                case Theme::Align::Left: {
                    view->x(rect.x());
                    break;
                }
                case Theme::Align::Right: {
                    view->x(rect.x() + rect.width() - view->width());
                    break;
                }
                }

                view->y(y);
                view->updateDimensions();

                y += h + settings.gap;
            }

            repaint();
            break;
        }
        case Type::Horizontal: {
            Coord h = rect.height();
            Coord x = rect.x();
            Coord y = rect.y();
            bool first = true;
            for (auto& view : views()) {
                if (!view->isVisible()) continue;
                if (!first) x += settings.gap;
                x += view->width();
                first = false;
            }

            m_HighestCoord = Math::max(x, 1.);

            if (scrollbarNecessary()) m_Scrolled = Math::Fast::clamp(m_Scrolled, 0, scrollSpace());
            else m_Scrolled = 0;

            Coord scroll = Math::floor(m_Scrolled);

            x = rect.x() - scroll;
            for (auto& view : views()) {
                if (!view->isVisible()) continue;
                Coord w = view->width();

                switch (settings.alignChildren & Theme::Align::Y) {
                case Theme::Align::CenterY: {
                    view->y(rect.y() + rect.height() / 2 - view->height() / 2);
                    break;
                }
                case Theme::Align::Left: {
                    view->y(rect.y());
                    break;
                }
                case Theme::Align::Right: {
                    view->y(rect.y() + rect.height() - view->height());
                    break;
                }
                }

                view->x(x);
                view->updateDimensions();

                x += w + settings.gap;
            }

            repaint();
            break;
        }
        }
    }

    // ------------------------------------------------

}
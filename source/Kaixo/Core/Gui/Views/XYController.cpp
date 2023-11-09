#include "Kaixo/Core/Gui/Views/XYController.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    XYController::XYController(Context c, Settings s)
        : View(c), settings(std::move(s))
    {}

    // ------------------------------------------------

    void XYController::parameterChanged(ParamID id, ParamValue val) {
        if (id == settings.x) { settings.valueX = val; repaint(); }
        if (id == settings.y) { settings.valueY = val; repaint(); }
    }

    // ------------------------------------------------

    void XYController::mouseExit(const juce::MouseEvent& event) {
        View::mouseExit(event);
        context.tooltip().close();
    }

    void XYController::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        beginEdit();
        setMouseCursor(juce::MouseCursor::NoCursor);
    }

    void XYController::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);
        endEdit();

        context.cursorPos(localPointToGlobal(handlePos()));

        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void XYController::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);

        auto w = width() - settings.padding.x() * 2;
        auto mult = 1.0 / w;
        if (event.mods.isShiftDown()) mult *= 0.25;
        if (event.mods.isCtrlDown()) mult *= 0.25;

        auto deltaX = (event.mouseDownPosition.x - event.x) * mult;
        auto deltaY = (event.mouseDownPosition.y - event.y) * mult;
        performEdit(Math::Fast::clamp1(settings.valueX - deltaX),
            Math::Fast::clamp1(settings.valueY - deltaY));

        context.cursorPos(localPointToGlobal(event.mouseDownPosition));
        setMouseCursor(juce::MouseCursor::NoCursor);

        repaint();

        if (settings.tooltip) {
            auto handle = handlePos();
            handle.y(handle.y() + settings.handleSize / 2 + 2);
            ;
            context.tooltip().open({
                .string = settings.tooltip(settings.valueX, settings.valueY),
                .position = localPointToGlobal(handle),
                });
        }
    }

    void XYController::paint(juce::Graphics& g) {
        Point<> p = handlePos();
        Rect<> pos = {
            p.x() - settings.handleSize / 2,
            p.y() - settings.handleSize / 2,
            settings.handleSize,
            settings.handleSize
        };

        settings.graphics.draw(g, pos, state(), settings.align);
    }

    // ------------------------------------------------

    Point<> XYController::handlePos() {
        Rect<> v{
            settings.padding.x(),
            settings.padding.y(),
            width() - 2 * settings.padding.x(),
            height() - 2 * settings.padding.y(),
        };

        auto xval = settings.valueX * v.width() + v.x();
        auto yval = settings.valueY * v.height() + v.y();
        return { xval, yval };
    }

    // ------------------------------------------------

    void XYController::beginEdit() {
        if (settings.x != NoParam) context.beginEdit(settings.x);
        if (settings.y != NoParam) context.beginEdit(settings.y);
    }

    void XYController::performEdit(ParamValue x, ParamValue y) {
        settings.valueX = x;
        settings.valueY = y;
        if (settings.x != NoParam) context.performEdit(settings.x, x);
        if (settings.y != NoParam) context.performEdit(settings.y, y);
    }

    void XYController::endEdit() {
        if (settings.x != NoParam) context.endEdit(settings.x);
        if (settings.y != NoParam) context.endEdit(settings.y);
    }

    // ------------------------------------------------

}
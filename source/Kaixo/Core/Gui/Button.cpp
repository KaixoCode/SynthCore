#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    using enum Button::Behaviour;
    using enum Button::Trigger;

    // ------------------------------------------------

    Button::Button(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        if (settings.param != NoParam) {
            auto& param = parameter(settings.param);
            context.description(param.description);
        }

        animation(settings.graphics);
    }

    // ------------------------------------------------

    void Button::parameterChanged(ParamID id, ParamValue value) {
        if (settings.param == id) {
            selected(value > 0.5);
            repaint();
        }
    }

    // ------------------------------------------------

    void Button::mouseEnter(const juce::MouseEvent& event) {
        View::mouseEnter(event);
        if (trigger() == OnHover) {
            if (behaviour() == Click) return callback(true);
            if (behaviour() == Toggle) return callback(!selected());
        }
    }

    void Button::mouseExit(const juce::MouseEvent& event) {
        View::mouseExit(event);
        if (behaviour() == Click && trigger() == OnHover) return callback(false);
    }

    void Button::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (trigger() == OnMouseDown) {
            if (behaviour() == Click) return callback(true);
            if (behaviour() == Toggle) return callback(!selected());
        }
    }

    void Button::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);
        if (behaviour() == Click && trigger() == OnMouseDown) return callback(false);

        if (contains(event.position) && trigger() == OnMouseUp) {
            if (behaviour() == Click) return callback(true);
            if (behaviour() == Toggle) return callback(!selected());
        }
    }

    // ------------------------------------------------

    void Button::paint(juce::Graphics& g)  {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .parameter = settings.param,
            .state = state(),
        });
    }

    // ------------------------------------------------

    void Button::callback(bool value) {
        bool isUntrigger = value == false;
        bool shouldCallback = isUntrigger ? settings.untrigger : true;
        if (settings.callback && shouldCallback) {
            settings.callback(value);
        }

        if (behaviour() == Toggle) selected(value);

        if (settings.param != NoParam) {
            context.beginEdit(settings.param);
            context.performEdit(settings.param, value);
            context.endEdit(settings.param);
        }
    }

    // ------------------------------------------------

}
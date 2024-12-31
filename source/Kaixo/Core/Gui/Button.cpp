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
        if (linkedToParam()) {
            auto& param = parameter(settings.param);
            description(param.description);
        }

        animation(settings.graphics);
    }

    // ------------------------------------------------

    void Button::parameterChanged(ParamID id, ParamValue value) {
        if (settings.param == id) {
            callback(value > 0.5, false);
            repaint();
        }
    }

    // ------------------------------------------------

    void Button::mouseEnter(const juce::MouseEvent& event) {
        View::mouseEnter(event);
        if (trigger() == OnHover) {
            if (behaviour() == Click) return callback(true);
            if (behaviour() == Toggle) return callback(!m_Value);
        }
    }

    void Button::mouseExit(const juce::MouseEvent& event) {
        View::mouseExit(event);
        if (behaviour() == Click && trigger() == OnHover) return callback(false);
    }

    void Button::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (event.mods.isLeftButtonDown()) {
            if (trigger() == OnMouseDown) {
                if (behaviour() == Click) return callback(true);
                if (behaviour() == Toggle) return callback(!m_Value);
            }
        } else if (event.mods.isRightButtonDown()) {
            if (isLinkedToParam()) {
                context.openParameterContextMenu(settings.param);
            }
        }
    }

    void Button::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);
        if (event.mods.isLeftButtonDown()) {
            if (behaviour() == Click && trigger() == OnMouseDown) return callback(false);

            if (contains(event.position) && trigger() == OnMouseUp) {
                if (behaviour() == Click) return callback(true);
                if (behaviour() == Toggle) return callback(!m_Value);
            }
        }
    }

    // ------------------------------------------------
    
    void Button::value(bool v) {
        callback(v);
    }

    // ------------------------------------------------

    void Button::paint(juce::Graphics& g) {
        settings.graphics.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .parameter = settings.param,
            .value = static_cast<float>(m_Value),
            .text{ { "$text", settings.text } },
            .state = state(),
        });
    }

    // ------------------------------------------------

    void Button::callback(bool value, bool withEdit) {
        if (m_Value == value) return; // Prevents recursion with parameterChanged
        m_Value = value;

        bool isUntrigger = value == false;
        bool shouldCallback = isUntrigger ? settings.untrigger : true;
        if (settings.callback && shouldCallback) {
            settings.callback(value);
        }

        if (behaviour() == Toggle) selected(value);

        if (linkedToParam() && withEdit) {
            context.beginEdit(settings.param);
            context.performEdit(settings.param, value);
            context.endEdit(settings.param);
        }
    }

    // ------------------------------------------------

}
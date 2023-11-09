#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    View::View(Context context)
        : context(context)
    {
        setWantsKeyboardFocus(true);
        setVisible(true);
        setRepaintsOnMouseActivity(true);
    }

    // ------------------------------------------------

    View::State View::state() const {
        State _state = State::Default;
        if (hovering()) _state |= State::Hovering;
        if (selected()) _state |= State::Selected;
        if (pressed())  _state |= State::Pressed;
        if (enabled())  _state |= State::Enabled;
        if (disabled()) _state |= State::Disabled;
        if (focused())  _state |= State::Focused;
        return _state;
    }

    // ------------------------------------------------

    void View::mouseMove(const juce::MouseEvent&) {
    }

    void View::mouseEnter(const juce::MouseEvent&) {
        hovering(true);
        if (!m_Description.empty()) {
            context.description(m_Description);
        }
    }

    void View::mouseExit(const juce::MouseEvent&) { 
        hovering(false);
        if (!m_Description.empty()) {
            context.clearDescription();
        }
    }

    void View::mouseDrag(const juce::MouseEvent&) {
    }

    void View::mouseDown(const juce::MouseEvent& event) {
        pressed(true);
    }

    void View::mouseUp(const juce::MouseEvent& event) {
        pressed(false);
    }

    void View::mouseDoubleClick(const juce::MouseEvent&) {
    }

    void View::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        juce::Component::mouseWheelMove(event, d);
    }

    // ------------------------------------------------

    void View::updateDimensions() {
        if (m_UseDimensions) {
            if (auto _parent = getParentComponent()) {
                evaluateDimensions(_parent->getLocalBounds());
            }
        }
    }

    // ------------------------------------------------
    
    void View::onIdle() {
        for (auto& view : m_Views) {
            if (view->wantsIdle()) 
                view->onIdle();
        }
    }

    // ------------------------------------------------

}
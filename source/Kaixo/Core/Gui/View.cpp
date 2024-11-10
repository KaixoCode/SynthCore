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

    void View::clear() {
        removeAllChildren();
        m_Views.clear();
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

    void View::wantsIdle(bool v) {
        m_Flags.wantsIdle = v; 
    }

    bool View::wantsIdle() const {
        if (m_Flags.wantsIdle) return true;
        // Also return true if a child wants idle
        for (auto& view : m_Views) {
            if (view->wantsIdle()) 
                return true;
        }

        return false; 
    }

    void View::onIdle() {
        for (auto& view : m_Views) {
            if (view->wantsIdle()) 
                view->onIdle();
        }

        for (auto& anim : m_LinkedAnimations) {
            if (anim->changing()) {
                repaint();
                break;
            }
        }

        for (auto& anim : m_ValueWatchers) {
            anim->update();
        }
    }

    // ------------------------------------------------
    
    void View::animation(Animation& anim) {
        wantsIdle(true); // Need idle when animation
        m_LinkedAnimations.push_back(&anim);
    }

    // ------------------------------------------------

}
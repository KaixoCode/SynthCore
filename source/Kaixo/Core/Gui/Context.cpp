#include "Kaixo/Core/Gui/Context.hpp"
#include "Kaixo/Core/Gui/Window.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    void Context::repaint() const { getWindow().repaint(); }

    // ------------------------------------------------

    void Context::listener(Listener* listener) { getWindow().listener(listener); }
    void Context::removeListener(Listener* listener) { getWindow().removeListener(listener); }

    // ------------------------------------------------

    void Context::beginEdit(ParamID id) const { getController().beginEdit(id); }

    void Context::performEdit(ParamID id, ParamValue val, bool force) const {
        val = Math::clamp1(val);
        if (force || getController().parameter(id).value() != val) {
            getController().performEdit(id, val);
            getWindow().notifyParameterChange(id, val, true);
        }
    }

    void Context::endEdit(ParamID id) const { getController().endEdit(id); }

    ParamValue       Context::param       (ParamID id) const { return getController().parameter(id).value(); }
    ParamValue       Context::defaultValue(ParamID id) const { return getController().parameter(id).getDefaultValue(); }
    int              Context::steps       (ParamID id) const { return getController().parameter(id).getNumSteps(); }
    std::string      Context::toString    (ParamID id) const { return getController().parameter(id).toString(); }
    std::string_view Context::name        (ParamID id) const { return getController().parameter(id).displayName(); }

    // ------------------------------------------------

    void Context::cursorPos(Point<int> pos) const {
        juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(pos.toFloat());
    }

    // ------------------------------------------------

    void Context::scale(float zoom) const { getWindow().scale(zoom); }
    float Context::scale() const { return getWindow().scale(); }

    // ------------------------------------------------

    Tooltip& Context::tooltip() const { return getWindow().tooltip(); }

    // ------------------------------------------------

    Window& Context::window() const { return *m_Window; }

    // ------------------------------------------------

    TabControl& Context::tabControl(std::int64_t id) const { return getWindow().m_TabControls[id]; }

    // ------------------------------------------------

    void Context::defaultDescription(std::string_view str) const { getWindow().defaultDescription(str); }
    void Context::description(std::string_view str) const { getWindow().description(str); }
    void Context::clearDescription() const { getWindow().clearDescription(); }

    // ------------------------------------------------

    void Context::initPreset() const {
        getController().initPreset();
    }

    SaveResult Context::savePreset(std::filesystem::path path, bool force) const {
        return getController().savePreset(path, force);
    }
    
    void Context::loadPreset(std::filesystem::path path) const {
        getController().loadPreset(path);
    }

    // ------------------------------------------------

    Context::Context(Window& window) : m_Window(&window) {}

    // ------------------------------------------------

    Controller& Context::getController() const { return getWindow().m_Controller; }
    Window& Context::getWindow() const { return *m_Window; };

    // ------------------------------------------------

}
#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Listeners.hpp"
#include "Kaixo/Core/Gui/Window.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class TabControl;
    class Tooltip;
    class View;

    // ------------------------------------------------

    class Context {
    public:

        // ------------------------------------------------

        void repaint() const;

        // ------------------------------------------------

        void listener(Listener* listener);
        void removeListener(Listener* listener);

        // ------------------------------------------------

        void beginEdit  (ParamID id)                                     const;
        void performEdit(ParamID id, ParamValue val, bool force = false) const;
        void endEdit    (ParamID id)                                     const;

        ParamValue       param       (ParamID id) const;
        ParamValue       defaultValue(ParamID id) const;
        int              steps       (ParamID id) const;
        std::string      toString    (ParamID id) const;
        std::string_view name        (ParamID id) const;

        // ------------------------------------------------
        
        void openParameterContextMenu(ParamID id) const;

        // ------------------------------------------------

        void cursorPos(Point<int> pos) const; // requires screen coordinates

        // ------------------------------------------------
        
        void scale(float zoom) const;
        float scale() const;

        // ------------------------------------------------
        
        Tooltip& tooltip() const;

        // ------------------------------------------------
        
        Window& window() const;

        // ------------------------------------------------

        template<std::derived_from<Processing::Interface> Ty>
        Processing::InterfaceStorage<Ty> interface(Ty::Settings settings) const;
        
        template<std::derived_from<Processing::Interface> Ty>
        Processing::InterfaceStorage<Ty> interface() const;

        // ------------------------------------------------

        template<std::derived_from<Controller> Ty>
        Ty& controller() const;

        // ------------------------------------------------
        
        template<std::derived_from<Serializable> Ty>
        Ty& data() const { return getController().data<Ty>(); }

        // ------------------------------------------------
        
        TabControl& tabControl(std::int64_t id) const;

        // ------------------------------------------------

        void defaultDescription(std::string_view str) const;
        void description(std::string_view str) const;
        void clearDescription() const;

        // ------------------------------------------------

        void initPreset() const;
        SaveResult savePreset(std::filesystem::path path, bool force = false) const;
        void loadPreset(std::filesystem::path path) const;

        // ------------------------------------------------

    private:
        Window* m_Window;

        // ------------------------------------------------

        Context(Window& window);

        // ------------------------------------------------

        Controller& getController() const;
        Window& getWindow() const;

        // ------------------------------------------------

        friend class Window;
    };

    // ------------------------------------------------
    
    template<std::derived_from<Controller> Ty>
    Ty& Context::controller() const { return dynamic_cast<Ty&>(getController()); };

    // ------------------------------------------------

    template<std::derived_from<Processing::Interface> Ty>
    Processing::InterfaceStorage<Ty> Context::interface(Ty::Settings settings) const {
        Ty* interface = getController().interface<Ty>();
        if (interface) {
            return Processing::InterfaceStorage<Ty>{ interface, settings };
        }
        throw std::exception("Interface does not exist");
    }

    template<std::derived_from<Processing::Interface> Ty>
    Processing::InterfaceStorage<Ty> Context::interface() const {
        auto interface = getController().interface<Ty>();
        if (interface) {
            return Processing::InterfaceStorage<Ty>{ interface };
        }
        throw std::exception("Interface does not exist");
    }

    // ------------------------------------------------

}
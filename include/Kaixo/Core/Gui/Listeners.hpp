#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class Window;
    class Listener {
    public:

        // ------------------------------------------------

        virtual ~Listener();

        // ------------------------------------------------
    private:
        Window* m_Window = nullptr;

        // ------------------------------------------------

        friend class Window;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class ParameterListener : public virtual Listener {
    public:

        // ------------------------------------------------

        virtual void parameterChanged(ParamID id, ParamValue val) {};
        virtual void parameterUIChanged(ParamID id, ParamValue val) {};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class DescriptionListener : public virtual Listener {
    public:

        // ------------------------------------------------

        virtual void updateDescription(std::string_view description) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class PresetListener : public virtual Listener {
    public:

        // ------------------------------------------------

        virtual void presetSaved() {};
        virtual void presetLoaded() {};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
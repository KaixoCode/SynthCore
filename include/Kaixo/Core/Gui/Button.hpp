#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class Button : public View, public ParameterListener {
    public:

        // ------------------------------------------------
        
        enum class Behaviour { Click, Toggle };

        enum class Trigger { OnMouseUp, OnMouseDown, OnHover };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::function<void(bool)> callback{};
            Theme::Drawable graphics{};

            // ------------------------------------------------

            Behaviour behaviour = Behaviour::Click;
            Trigger trigger = Trigger::OnMouseDown;
            bool untrigger = behaviour == Behaviour::Toggle;

            // ------------------------------------------------

            ParamID param = NoParam;

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        Button(Context c, Settings settings = {});

        // ------------------------------------------------

        void parameterChanged(ParamID id, ParamValue value) override;

        // ------------------------------------------------

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        Behaviour behaviour() const { return settings.behaviour; }
        Trigger trigger() const { return settings.trigger; }

        // ------------------------------------------------
    protected:
        bool m_Value = false;

        // ------------------------------------------------

        void callback(bool value);

        // ------------------------------------------------
        
        bool linkedToParam() const { return settings.param != NoParam; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
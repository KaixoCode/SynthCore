#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Stateful.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    class Button : public View {
    public:

        // ------------------------------------------------
        
        enum class Behaviour { Click, Toggle };

        enum class Trigger { OnMouseUp, OnMouseDown, OnHover };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::function<void(bool)> callback{};
            Theme::Stateful graphics{};
            Theme::Align align = Theme::Align::Center;

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

        // ------------------------------------------------

        void callback(bool value);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
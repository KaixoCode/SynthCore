#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Controller.hpp"
#include "Kaixo/Core/Gui/Listeners.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class TabControl;
    class Tooltip;
    class Context;
    class View;

    // ------------------------------------------------

    class Window : public juce::AudioProcessorEditor, public juce::Timer {
    public:

        // ------------------------------------------------

        Window(Controller&);
        ~Window() override;

        // ------------------------------------------------

        void paint(juce::Graphics&) override;
        void resized() override;

        // ------------------------------------------------
        
        Tooltip& tooltip() { return *m_Tooltip; }

        // ------------------------------------------------
        
        void notifyParameterChange(ParamID id, ParamValue value, bool ui = false);

        // ------------------------------------------------
        
        void defaultDescription(std::string_view str);
        void description(std::string_view str);
        void clearDescription();

        // ------------------------------------------------
        
        void notifyPresetLoad();
        void notifyPresetSave();

        // ------------------------------------------------
        
        void listener(Listener* listener);
        void removeListener(Listener* listener);

        // ------------------------------------------------

    private:
        Controller& m_Controller;
        std::vector<ParamValue> m_ParameterValues{};
        std::map<std::int64_t, TabControl> m_TabControls{};
        std::unique_ptr<Tooltip> m_Tooltip;

        // ------------------------------------------------
        
        std::string m_DefaultDescription = "";
        std::string m_Description = "";
        bool m_ClearedDescription = true;

        void descriptionUpdated();

        // ------------------------------------------------
        
        std::list<Listener*> m_Listeners;
        bool m_InsideNotifyLoop = false;

        // ------------------------------------------------

        std::unique_ptr<View> m_BaseComponent;

        // ------------------------------------------------

        void timerCallback() override;

        // ------------------------------------------------
        
        template<class ...Tys, std::derived_from<Listener> Ty, class ...Args>
        void notifyListeners(void(Ty::*fun)(Tys ...), Args&&... args) {
            m_InsideNotifyLoop = true;
            for (auto& listener : m_Listeners) {
                if (auto typed = dynamic_cast<Ty*>(listener)) {
                    (typed->*fun)(std::forward<Args>(args)...);
                }
            }
            std::erase(m_Listeners, nullptr);
            m_InsideNotifyLoop = false;
        }

        // ------------------------------------------------

        friend class Context;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Window)
    };

    // ------------------------------------------------

    View* createBaseView(Context);

    // ------------------------------------------------

}

#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Gui/Window.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Context.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"
#include "Kaixo/Core/Gui/TabControl.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Window::Window(Controller& p)
        : AudioProcessorEditor(&p), 
          m_Controller(p), 
          m_Tooltip(std::make_unique<Tooltip>(Context{ *this })),
          m_BaseComponent(createBaseView(Context{ *this }))
    {

        if (auto l = dynamic_cast<Listener*>(m_BaseComponent.get())) {
            listener(l);
        }

        setSize(SYNTH_InitialSize);

        addChildComponent(*m_BaseComponent);
        addChildComponent(*m_Tooltip);
        m_BaseComponent->updateDimensions();
        m_Tooltip->updateDimensions();

        startTimerHz(60); // idle timer

        m_ParameterValues.resize(m_Controller.numParameters());

        auto count = m_Controller.numParameters();
        for (ParamID id = 0; id < count; ++id) {
            notifyParameterChange(id, m_Controller.parameter(id).value());
        }

        m_Controller.m_Window = this;
        notifyPresetLoad();
    }

    Window::~Window() {
        m_Controller.m_Window = nullptr;
    }

    // ------------------------------------------------

    void Window::paint(juce::Graphics& g) {}

    void Window::resized() {}
    
    // ------------------------------------------------
    
    void Window::notifyParameterChange(ParamID id, ParamValue value, bool ui) {
        m_ParameterValues[id] = value;

        notifyListeners(&ParameterListener::parameterChanged, id, value);
        if (ui) notifyListeners(&ParameterListener::parameterUIChanged, id, value);
    }

    // ------------------------------------------------

    void Window::defaultDescription(std::string_view str) { 
        m_DefaultDescription = std::string{ str };
        if (m_ClearedDescription) {
            m_Description = m_DefaultDescription;
            descriptionUpdated();
        }
    }

    void Window::description(std::string_view str) {
        m_Description = std::string{ str }; 
        m_ClearedDescription = false;
        descriptionUpdated();
    }

    void Window::clearDescription() { 
        m_Description = m_DefaultDescription;
        m_ClearedDescription = true;
        descriptionUpdated();
    }

    void Window::descriptionUpdated() {
        notifyListeners(&DescriptionListener::updateDescription, m_Description);
    }

    // ------------------------------------------------

    void Window::notifyPresetLoad() {
        notifyListeners(&PresetListener::presetLoaded);
        repaint();
    }

    void Window::notifyPresetSave() {
        notifyListeners(&PresetListener::presetSaved);
        repaint();
    }

    // ------------------------------------------------

    void Window::listener(Listener* listener) {
        m_Listeners.push_back(listener);
        listener->m_Window = this;
    }

    void Window::removeListener(Listener* listener) {
        if (m_InsideNotifyLoop) { // When inside loop, mark as deleted (nullptr)
            for (auto& l : m_Listeners) {
                if (l == listener) l = nullptr;
            }
        } else { 
            std::erase(m_Listeners, listener);
        }
    }

    // ------------------------------------------------

    void Window::timerCallback() {
        auto count = m_Controller.numParameters();
        for (ParamID id = 0; id < count; ++id) {
            auto value = m_Controller.parameter(id).value();

            if (value != m_ParameterValues[id]) {
                notifyParameterChange(id, value);
            }
        }

        m_BaseComponent->onIdle();
    }

    // ------------------------------------------------

}
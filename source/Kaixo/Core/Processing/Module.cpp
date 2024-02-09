#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    Buffer& Module::outputBuffer() const { return m_Controller->m_Output; }
    const Buffer& Module::inputBuffer() const { return m_Controller->m_Input; }

    void Module::oversample(std::size_t n) const { m_Controller->m_Oversample = n; }

    bool Module::offline() const { return m_Controller->m_Offline; }
    double Module::generatingSampleRate() const { return m_Controller->m_SampleRate; }
    double Module::sampleRate() const { return m_Controller->m_SampleRate * m_Controller->m_Oversample; }
    double Module::bpm() const { return m_Controller->m_Bpm; }
    std::int64_t Module::timeInSamples() const { return m_Controller->m_TimeInSamples; }

    juce::AudioPlayHead::TimeSignature Module::timeSignature() const { return m_Controller->m_TimeSignature; }

    // ------------------------------------------------

    void ModuleContainer::prepare(double sampleRate, std::size_t maxBufferSize) {
        Module::prepare(sampleRate, maxBufferSize);
        for (auto& module : m_Modules)
            module->prepare(sampleRate, maxBufferSize);
    }

    void ModuleContainer::reset() {
        Module::reset();
        for (auto& module : m_Modules)
            module->reset();
    }

    // ------------------------------------------------

    bool ModuleContainer::active() const {
        for (auto& module : m_Modules) {
            if (module->active()) return true;
        }
        return false;
    }

    // ------------------------------------------------

    void ModuleContainer::param(ParamID id, ParamValue value) {
        for (auto& module : m_Listeners)
            module->param(id, value);
    }

    void ModuleContainer::setController(Controller* controller) {
        Module::setController(controller);
        for (auto& module : m_Modules) {
            module->setController(controller);
        }
    }

    // ------------------------------------------------

}
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    auto Module::outputBuffer() const ->       Buffer& { return m_Controller->m_Output; }
    auto Module::inputBuffer()  const -> const Buffer& { return m_Controller->m_Input; }

    auto Module::sampleRate()    const -> double { return m_Controller->m_SampleRate; }
    auto Module::bpm()           const -> double { return m_Controller->m_Bpm; }
    auto Module::timeInSamples() const -> std::int64_t { return m_Controller->m_TimeInSamples; }

    auto Module::timeSignature() const -> juce::AudioPlayHead::TimeSignature { return m_Controller->m_TimeSignature; }

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
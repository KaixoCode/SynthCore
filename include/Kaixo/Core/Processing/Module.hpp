#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"

// ------------------------------------------------

namespace Kaixo { class Controller; }

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Module {
    public:

        // ------------------------------------------------

        virtual void process() {};
        virtual void prepare(double sampleRate, std::size_t maxBufferSize) {}
        virtual void reset() {};
        virtual bool active() const { return false; }

        // ------------------------------------------------
        
        Buffer& outputBuffer() const;
        const Buffer& inputBuffer() const;

        double sampleRate() const;
        double bpm() const;
        std::int64_t timeInSamples() const;

        juce::AudioPlayHead::TimeSignature timeSignature() const;

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        Controller* m_Controller;

        // ------------------------------------------------

        virtual void setController(Controller* controller) { m_Controller = controller; }

        // ------------------------------------------------
        
        friend class ModuleContainer;
        friend class Controller;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ParameterListener {
    public:

        // ------------------------------------------------

        virtual void param(ParamID id, ParamValue val) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class ModuleContainer : public Module, public ParameterListener {
    public:

        // ------------------------------------------------

        virtual void process() override {};
        virtual void prepare(double sampleRate, std::size_t maxBufferSize) override;
        virtual void reset() override;

        // ------------------------------------------------
        
        virtual bool active() const override;

        // ------------------------------------------------

        virtual void param(ParamID id, ParamValue value) override;

        // ------------------------------------------------

        template<std::derived_from<Module> Ty>
        void registerModule(Ty& module);

        // ------------------------------------------------

    private:
        std::vector<Module*> m_Modules{};
        std::vector<ParameterListener*> m_Listeners{};

        // ------------------------------------------------
        
        void setController(Controller* controller) override;

        // ------------------------------------------------
        
        friend class Kaixo::Controller;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<std::derived_from<Module> Ty>
    void ModuleContainer::registerModule(Ty& module) {
        m_Modules.push_back(&module);
        if constexpr (std::derived_from<Ty, ParameterListener>) {
            m_Listeners.push_back(&module);
        }
    }

    // ------------------------------------------------

}
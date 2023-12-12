#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Parameter.hpp"
#include "Kaixo/Core/Serializable.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Interface.hpp"

// ------------------------------------------------

namespace Kaixo { class Controller; }

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Processor : public ModuleContainer, public Serializable {
    public:

        // ------------------------------------------------

        Processor() = default;
        virtual ~Processor() = default;

        // ------------------------------------------------

        virtual void noteOn(Note note, double velocity, int channel) {}
        virtual void noteOff(Note note, double velocity, int channel) {}

        // ------------------------------------------------
        
        virtual bool isActive() const { return true; }

        // ------------------------------------------------

        virtual void init() override {}
        virtual json serialize() override { return {}; }
        virtual void deserialize(json& data) override {}

        // ------------------------------------------------

        template<std::derived_from<Interface> Ty>
        void registerInterface();

        // ------------------------------------------------
        
        ParamValue paramValue(ParamID id) const { return m_ParameterValues[id]; }

        // ------------------------------------------------

    private:
        std::vector<ParamValue> m_ParameterValues{};
        std::map<std::type_index, std::unique_ptr<Interface>> m_Interfaces{};

        // ------------------------------------------------

        template<std::derived_from<Interface> Ty>
        Ty* interface();

        // ------------------------------------------------

        void receiveParameterValue(ParamID id, ParamValue value);

        // ------------------------------------------------

        friend class ::Kaixo::Controller;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
    };

    // ------------------------------------------------

    template<std::derived_from<Interface> Ty>
    void Processor::registerInterface() {
        Ty* interface = new Ty{};
        interface->m_Self = this;
        m_Interfaces[typeid(Ty)] = std::unique_ptr<Ty>(interface);
    }
    
    template<std::derived_from<Interface> Ty>
    Ty* Processor::interface() {
        if (m_Interfaces.contains(typeid(Ty))) return dynamic_cast<Ty*>(m_Interfaces[typeid(Ty)].get());
        return nullptr;
    }

    // ------------------------------------------------

    Processor* createProcessor();

    // ------------------------------------------------
}
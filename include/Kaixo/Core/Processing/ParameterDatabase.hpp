#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/ModulationDatabase.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    template<class Self>
    class ParameterDatabase : public Module, public ParameterListener {
    public:

        // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
        constexpr static std::size_t Sources = nofSources();
        struct Source {
            float value = 0;
            float normalized = 0;
        };
#endif

        // ------------------------------------------------

        constexpr static std::size_t Parameters = nofParameters();
        struct Param {
            ParamValue add{};
            ParamValue goal{};
            ParamValue value{};
            ParamValue access{};
        };

        // ------------------------------------------------
        
        ParameterDatabase(Self* self) 
            : m_Self(self) 
        {}

        // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
        void link(ModulationDatabase& database) { m_LinkedModulationDatabase = &database; }
#endif
        // ------------------------------------------------

        ParamValue read(ParamID id) { return m_Parameters[id].access; }

        void setChanging(ParamID id) { m_Changing.set(id); }

        // ------------------------------------------------

        void process() override {
            m_SamplesSinceInterpolation++;
            if (m_SamplesSinceInterpolation >= m_InterpolateEvery) {
                m_SamplesSinceInterpolation = 0;
                
                float r = 1.f / m_InterpolateEvery;

                m_Changing.foreach([&](ParamID i) {
                    m_Parameters[i].add = r * (m_Parameters[i].goal - m_Parameters[i].value);

                    if (!changing(i)) {
                        Kaixo::Processing::assignParameter(*this, i, m_Parameters[i].goal);
                        m_Changing.unset(i);
                    }
                });
            }

#ifdef KAIXO_INTERNAL_MODULATION
            Kaixo::Processing::assignSources(*this);
#endif
            Kaixo::Processing::assignParameters(*this);
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            Module::prepare(sampleRate, maxBufferSize);
            m_InterpolateEvery = (m_MillisToInterpolate / 1000.f) * sampleRate;
            reset();
        }

        void reset() override { 
            rebase(); 
#ifdef KAIXO_INTERNAL_MODULATION
            for (Source& source : m_Sources)
                source.value = source.normalized = 0;
#endif
        }

        void rebase() {
            m_SamplesSinceInterpolation = 0;
            for (ParamID id = 0; id < Parameters; ++id) {
                m_Parameters[id].value = m_Parameters[id].access = m_Parameters[id].goal;
                m_Parameters[id].add = 0;
            }
        }

        // ------------------------------------------------

        void param(ParamID id, ParamValue val) override { 
            Kaixo::Processing::assignParameter(*this, id, val);
        }

        // ------------------------------------------------
        
        constexpr bool necessary(ParamID id) const { return m_Changing.test(id); }

        constexpr Self& self() { return *m_Self; }
        constexpr Param& parameter(ParamID id) { return m_Parameters[id]; }

#ifdef KAIXO_INTERNAL_MODULATION
        constexpr Source& source(ModulationSourceID id) { return m_Sources[id]; }

        constexpr void loopOverSources(ParamID id, auto fun) const {
            m_LinkedModulationDatabase->foreach(id, fun);
        }
#endif

        // ------------------------------------------------

    protected:
        constexpr static std::size_t m_MillisToInterpolate = 2;

        // ------------------------------------------------

        Self* m_Self;

        // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
        ModulationDatabase* m_LinkedModulationDatabase = nullptr;
        std::array<Source, Sources> m_Sources{};
#endif
        std::array<Param, Parameters> m_Parameters;
        StateVector<ParamID, Parameters> m_Changing;

        // ------------------------------------------------

        std::size_t m_SamplesSinceInterpolation = 0;
        std::size_t m_InterpolateEvery = 100;

        // ------------------------------------------------

        constexpr bool changing(ParamID i) const {
#ifdef KAIXO_INTERNAL_MODULATION
            return m_LinkedModulationDatabase->modulated(i)
                || Math::Fast::abs(m_Parameters[i].add) > 0.00000001f;
#else
            return Math::Fast::abs(m_Parameters[i].add) > 0.00000001f;
#endif
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
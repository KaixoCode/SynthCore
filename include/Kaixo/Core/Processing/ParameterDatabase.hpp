#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/ModulationDatabase.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    template<class Type>
    class ParameterDatabase : public Module, public ParameterListener {
    public:

        // ------------------------------------------------
        
        constexpr static std::size_t Parameters = nofParameters();

        // ------------------------------------------------

        using ParameterAssigner = void(Type&, ParamValue);

        // ------------------------------------------------
        
        ParameterDatabase(Type* self) : m_Self(self) {
            for (std::size_t i = 0; i < Parameters; ++i) {
                m_Parameters[i].multiply = parameter(i).multiply;
                m_Parameters[i].constrain = parameter(i).constrain;
                m_Parameters[i].smoothed = parameter(i).doSmoothing();
                m_Parameters[i].modulated = parameter(i).doModulation();
            }
        }

        // ------------------------------------------------

        void link(ParamID id, ParameterAssigner* value) { m_Parameters[id].assigner = value; }

        ParamValue read(ParamID id) { return access(id); }

        // ------------------------------------------------

        void process() override {
            m_SamplesSinceInterpolation++;
            if (m_SamplesSinceInterpolation >= m_InterpolateEvery) {
                m_SamplesSinceInterpolation = 0;
                
                float r = 1.f / m_InterpolateEvery;

                m_Changing.foreach([&](ParamID i) {
                    add(i) = r * (goal(i) - value(i));

                    if (!changing(i)) {
                        m_Changing.unset(i);
                        // Need to do a final synchronization when no longer changing
                        // this is necessary for when a modulation is reset to 0
                        access(i) = goal(i);
                        assigner(i, access(i));
                    }
                });
            }

            m_Changing.foreach([&](ParamID id) {
                access(id) = value(id) += add(id);
                assigner(id, access(id));
            });
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            Module::prepare(sampleRate, maxBufferSize);
            m_InterpolateEvery = 0.001 * m_MillisToInterpolate * sampleRate;
            reset();
        }

        void reset() override { rebase(); }

        void rebase() {
            m_SamplesSinceInterpolation = 0;
            for (ParamID id = 0; id < Parameters; ++id) {
                value(id) = access(id) = goal(id);
                add(id) = 0;
            }
        }

        // ------------------------------------------------

        void param(ParamID id, ParamValue val) override { 
            if (!smoothed(id) || !m_Self->isActive()) {
                access(id) = value(id) = goal(id) = val;
                assigner(id, val);
                add(id) = 0;
            } else {
                goal(id) = val; 
                m_Changing.set(id);
            }
        }

        // ------------------------------------------------

    protected:
        constexpr static std::size_t m_MillisToInterpolate = 2;

        // ------------------------------------------------

        struct Param {
            ParameterAssigner* assigner = [](Type&, ParamValue) {};
            ParamValue add;
            ParamValue goal;
            ParamValue value;
            ParamValue access;
            bool modulated = false;
            bool smoothed = false;
            bool multiply = false;
            bool constrain = false;
        };

        // ------------------------------------------------

        Type* m_Self;

        std::size_t m_SamplesSinceInterpolation = 0;
        std::size_t m_InterpolateEvery = 100;

        std::array<Param, Parameters> m_Parameters;
        StateVector<ParamID, Parameters> m_Changing;

        // ------------------------------------------------

        constexpr ParamValue& add   (ParamID i) { return m_Parameters[i].add; }
        constexpr ParamValue& value (ParamID i) { return m_Parameters[i].value; }
        constexpr ParamValue& goal  (ParamID i) { return m_Parameters[i].goal; }
        constexpr ParamValue& access(ParamID i) { return m_Parameters[i].access; }

        constexpr bool modulated(ParamID i) { return m_Parameters[i].modulated; }
        constexpr bool smoothed(ParamID i) { return m_Parameters[i].smoothed; }
        constexpr bool multiply(ParamID i) { return m_Parameters[i].multiply; }
        constexpr bool constrain(ParamID i) { return m_Parameters[i].constrain; }

        // ------------------------------------------------

        constexpr void assigner(ParamID i, ParamValue v) { return m_Parameters[i].assigner(*m_Self, v); }
        constexpr bool changing(ParamID i) { return Math::Fast::abs(add(i)) > 0.00000001f; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Type, class Mod = Modulation>
    class ModulatedParameterDatabase : public ParameterDatabase<Type> {
    public:

        // ------------------------------------------------
        
        constexpr static std::size_t Parameters = nofParameters();
        constexpr static std::size_t Sources = nofSources();

        // ------------------------------------------------

        using ModulationAssigner = float(Type&);
        using ParameterDB = ParameterDatabase<Type>;
        using ModulationDB = ModulationDatabase<Mod>;

        // ------------------------------------------------

        using ParameterDB::ParameterDatabase;

        // ------------------------------------------------
        
        using ParameterDB::link;
        
        void link(ModulationDB& database) { m_LinkedDatabase = &database; }
        void link(ModulationSourceID id, ModulationAssigner* assigner, bool bi = false) { 
            m_Sources[id].assigner = assigner; 
            m_Sources[id].bidirectional = bi;
        }

        // ------------------------------------------------
        
        void process() override {
            if (m_LinkedDatabase == nullptr) {
                ParameterDB::process();
                return;
            }

            this->m_SamplesSinceInterpolation++;
            if (this->m_SamplesSinceInterpolation >= this->m_InterpolateEvery) {
                this->m_SamplesSinceInterpolation = 0;

                float r = 1.f / this->m_InterpolateEvery;

                this->m_Changing.foreach([&](ParamID i) {
                    if (this->modulated(i)) {
                        this->add(i) = r * (this->goal(i) - this->value(i));

                        if (!this->changing(i) && !m_LinkedDatabase->modulated(i)) {
                            this->m_Changing.unset(i);
                            // Need to do a final synchronization when no longer changing
                            // this is necessary for when a modulation is reset to 0
                            this->access(i) = this->goal(i);
                            this->assigner(i, this->access(i));
                        }
                    } else if (this->smoothed(i)) {
                        this->add(i) = r * (this->goal(i) - this->value(i));
                        if (!this->changing(i)) {
                            this->m_Changing.unset(i);
                            // Need to do a final synchronization when no longer changing
                            // this is necessary for when a modulation is reset to 0
                            this->access(i) = this->goal(i);
                            this->assigner(i, this->access(i));
                        }
                    } else {
                        this->access(i) = this->value(i) = this->goal(i);
                        this->assigner(i, this->access(i));
                    }
                });
            }

            for (ModulationSourceID i = 0; i < Sources; ++i) {
                auto value = m_Sources[i].assigner(*this->m_Self);
                m_Sources[i].normalized = value;
                m_Sources[i].value = m_Sources[i].bidirectional ? value * 2 - 1 : value;
            }

            this->m_Changing.foreach([&](ParamID id) {
                if (this->modulated(id)) {
                    float value = this->value(id) += this->add(id);
                    if (this->multiply(id)) {
                        m_LinkedDatabase->foreach(id, [&](ModulationSourceID source, Mod& mod) {
                            value *= mod.amount() * m_Sources[source].normalized + Math::Fast::min(1 - mod.amount(), 1);    
                        });
                    } else {
                        m_LinkedDatabase->foreach(id, [&](ModulationSourceID source, Mod& mod) {
                            value += mod.amount() * m_Sources[source].value;
                        });
                    }

                    this->access(id) = this->constrain(id) ? Math::Fast::clamp1(value) : value;
                    this->assigner(id, this->access(id));
                } else if (this->smoothed(id)) {
                    this->access(id) = this->value(id) += this->add(id);
                    this->assigner(id, this->access(id));
                } else {
                    this->assigner(id, this->access(id));
                }
            });
        }

        void reset() override {
            ParameterDB::reset();
            for (auto& source : m_Sources)
                source.value = source.normalized = 0;
        }

        // ------------------------------------------------
        
        float source(ModulationSourceID id) const { return m_Sources[id].normalized; }

        // ------------------------------------------------

    private:
        struct Source {
            ModulationAssigner* assigner = [](Type&) -> float { return 0; };
            bool bidirectional = false;
            float value = 0;
            float normalized = 0;
        };

        // ------------------------------------------------

        ModulationDB* m_LinkedDatabase = nullptr;
        std::array<Source, Sources> m_Sources{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
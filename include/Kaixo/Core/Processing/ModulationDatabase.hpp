#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Parameter.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    class Modulation {
    public:

        // ------------------------------------------------
        
        Modulation(Sample amount = 0)
            : m_Amount(amount)
        {}

        // ------------------------------------------------

        Sample amount() const { return m_Amount; }
        bool active() const { return amount() != 0; };

        // ------------------------------------------------

        void init() { m_Amount = 0; }
        json serialize() { return m_Amount; }
        void deserialize(json& data) { data.assign_or_default(m_Amount, 0); }

        // ------------------------------------------------

    private:
        Sample m_Amount = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<std::derived_from<Modulation> Mod = Modulation>
    class ModulationDatabase : public Serializable {
    public:

        // ------------------------------------------------
        
        const inline static Mod Zero{};

        // ------------------------------------------------
        
        constexpr static std::size_t Parameters = nofParameters();
        constexpr static std::size_t Sources = nofSources();

        // ------------------------------------------------
        
        ModulationDatabase() {}

        // ------------------------------------------------

        void set(ParamID id, ModulationSourceID source, Mod mod) {
            bool shouldRemove = !mod.active();
            for (std::size_t i = 0; i < m_Modulations[id].size(); ++i) {
                if (m_Modulations[id][i].source == source) {
                    if (shouldRemove) {
                        m_Modulations[id].erase_index(i);
                    } else {
                        m_Modulations[id][i].modulation = std::move(mod);
                    }
                    return;
                }
            }

            if (!shouldRemove) {
                m_Modulations[id].emplace_back(source, std::move(mod));
            }
        }

        const Mod& get(ParamID id, ModulationSourceID source) {
            for (std::size_t i = 0; i < m_Modulations[id].size(); ++i) {
                if (m_Modulations[id][i].source == source) {
                    return m_Modulations[id][i].modulation;
                }
            }

            return Zero;
        }

        // ------------------------------------------------
        
        void foreach(ParamID id, auto fun) {
            for (auto& [source, mod] : m_Modulations[id]) fun(source, mod);
        }

        // ------------------------------------------------
        
        bool modulated(ParamID id) { return m_Modulations[id].size() != 0; }

        // ------------------------------------------------

        void init() override {
            for (auto& p : m_Modulations) p.clear();
        }

        json serialize() override {
            json data = json::object();
            for (ParamID param = 0; param < Parameters; ++param) {
                if (!modulated(param)) continue;
                auto& modulations = m_Modulations[param];
                auto& val = getFromIdentifier(data, parameter(param).fullVarName);
                val = json::array();
                for (Entry& mod : modulations) {
                    json el;
                    ModulationSourceID source = mod.source;
                    el["source"] = std::string{ modulationSource(source).fullVarName };
                    el["modulation"] = mod.modulation.serialize();
                    val.emplace(el);
                }
            }

            return data;
        }

        void deserialize(json& data) override {
            for (ParamID param = 0; param < Parameters; ++param) {
                auto& modulations = m_Modulations[param];
                auto& val = getFromIdentifier(data, parameter(param).fullVarName);
                if (val.is(json::Array)) {
                    auto& arr = val.as<json::array>();
                    for (auto& el : arr) {
                        if (el.contains("source", json::String) &&
                            el.contains("modulation"))
                        {
                            ModulationSourceID source = NoSource;
                            auto& sourceName = el["source"].as<json::string>();
                            for (ModulationSourceID id = 0; id < Sources; ++id) {
                                if (modulationSource(id).fullVarName == sourceName) {
                                    source = id;
                                    break;
                                }
                            }

                            if (source != NoSource) {
                                Entry& mod = modulations.emplace_back();
                                mod.source = source;
                                mod.modulation.deserialize(el["modulation"]);
                            }
                        }
                    }
                }
            }
        }

        // ------------------------------------------------

    private:
        struct Entry {
            ModulationSourceID source;
            Mod modulation;
        };

        std::array<Vector<Entry, Sources>, Parameters> m_Modulations{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
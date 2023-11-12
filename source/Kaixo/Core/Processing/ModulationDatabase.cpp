#include "Kaixo/Core/Processing/ModulationDatabase.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
    void ModulationDatabase::init() {
        for (auto& p : m_Modulations) p.clear();
    }

    json ModulationDatabase::serialize() {
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
                el["amount"] = mod.amount;
                val.emplace(el);
            }
        }

        return data;
    }

    void ModulationDatabase::deserialize(json& data) {
        init();
        for (ParamID param = 0; param < Parameters; ++param) {
            auto& modulations = m_Modulations[param];
            auto& val = getFromIdentifier(data, parameter(param).fullVarName);
            if (val.is(json::Array)) {
                auto& arr = val.as<json::array>();
                for (auto& el : arr) {
                    if (el.contains("source", json::String) &&
                        el.contains("amount", json::Floating))
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
                            mod.amount = el["amount"].as<json::floating>();
                        }
                    }
                }
            }
        }
    }
#endif

    // ------------------------------------------------

}
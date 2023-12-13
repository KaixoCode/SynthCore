#include "Kaixo/Core/Processing/ModulationDatabase.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
    void ModulationDatabase::init() {
        for (auto& p : m_Modulations) p.clear();
    }

    basic_json ModulationDatabase::serialize() {
        basic_json data = basic_json::object();
        for (ParamID param = 0; param < Parameters; ++param) {
            if (!modulated(param)) continue;
            auto& modulations = m_Modulations[param];
            auto& val = getFromIdentifier(data, parameter(param).fullVarName);
            val = basic_json::array();
            for (Entry& mod : modulations) {
                basic_json el;
                ModulationSourceID source = mod.source;
                el["source"] = std::string{ modulationSource(source).fullVarName };
                el["amount"] = mod.amount;
                val.push_back(el);
            }
        }

        return data;
    }

    void ModulationDatabase::deserialize(basic_json& data) {
        init();
        for (ParamID param = 0; param < Parameters; ++param) {
            auto& modulations = m_Modulations[param];
            auto& val = getFromIdentifier(data, parameter(param).fullVarName);
            if (val.is(basic_json::Array)) {
                auto& arr = val.as<basic_json::array>();
                for (auto& el : arr) {
                    if (el.contains("source", basic_json::String) &&
                        el.contains("amount", basic_json::Number))
                    {
                        ModulationSourceID source = NoSource;
                        auto& sourceName = el["source"].as<basic_json::string>();
                        for (ModulationSourceID id = 0; id < Sources; ++id) {
                            if (modulationSource(id).fullVarName == sourceName) {
                                source = id;
                                break;
                            }
                        }

                        if (source != NoSource) {
                            Entry& mod = modulations.emplace_back();
                            mod.source = source;
                            mod.amount = el["amount"].as<float>();
                        }
                    }
                }
            }
        }
    }
#endif

    // ------------------------------------------------

}
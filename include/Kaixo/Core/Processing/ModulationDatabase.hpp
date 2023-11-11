#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Parameter.hpp"
#include "Kaixo/Core/Serializable.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

#ifdef KAIXO_INTERNAL_MODULATION
    class ModulationDatabase : public Serializable {
    public:

        // ------------------------------------------------
        
        constexpr static std::size_t Parameters = nofParameters();
        constexpr static std::size_t Sources = nofSources();

        // ------------------------------------------------

        void set(ParamID id, ModulationSourceID source, float amount) {
            bool shouldRemove = amount == 0;
            for (std::size_t i = 0; i < m_Modulations[id].size(); ++i) {
                if (m_Modulations[id][i].source == source) {
                    if (shouldRemove) {
                        m_Modulations[id].erase_index(i);
                    } else {
                        m_Modulations[id][i].amount = amount;
                    }
                    return;
                }
            }

            if (!shouldRemove) {
                m_Modulations[id].emplace_back(source, amount);
            }
        }

        double get(ParamID id, ModulationSourceID source) {
            for (std::size_t i = 0; i < m_Modulations[id].size(); ++i) {
                if (m_Modulations[id][i].source == source) {
                    return m_Modulations[id][i].amount;
                }
            }

            return 0;
        }

        // ------------------------------------------------
        
        constexpr void foreach(ParamID id, auto fun) const {
            for (auto& [source, mod] : m_Modulations[id]) fun(source, mod);
        }

        // ------------------------------------------------
        
        constexpr bool modulated(ParamID id) const { return m_Modulations[id].size() != 0; }

        // ------------------------------------------------

        void init() override;
        json serialize() override;
        void deserialize(json& data) override;

        // ------------------------------------------------

    private:
        struct Entry {
            ModulationSourceID source;
            float amount;
        };

        std::array<Vector<Entry, Sources>, Parameters> m_Modulations{};

        // ------------------------------------------------

    };
#endif

    // ------------------------------------------------

}
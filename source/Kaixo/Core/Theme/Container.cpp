#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    void Container::interpret(const basic_json& json) {
        for (auto& [name, interpret] : m_Elements) {
            if (json.contains(name)) {
                interpret(json[name]);
            }
        }
    }

    // ------------------------------------------------

}
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    void Container::interpret(const json& json) {
        for (auto& [name, interpret] : m_Elements) {
            if (json.contains(name, json::String)) {
                const std::string& _value = json[name].as<json::string>();
                if (self->hasVariable(_value)) {
                    interpret(self->variable(_value));
                    continue;
                }
            } 
            
            if (json.contains(name)) {
                interpret(json[name]);
            }
        }
    }

    // ------------------------------------------------

}
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    DynamicElement DynamicElement::operator[](std::string_view name) {
        if (auto container = dynamic_cast<Container*>(m_Element)) {
            return (*container)[name];
        }
        return nullptr;
    }

    // ------------------------------------------------

    void Container::interpret(const basic_json& json) {
        for (auto& [name, el] : m_Elements) {
            if (json.contains(name)) {
                el->interpret(json[name]);
            }
        }
    }

    // ------------------------------------------------
    
    DynamicElement Container::operator[](std::string_view search) {
        for (auto& [name, el] : m_Elements) {
            if (name == search) return el;
        }
        return nullptr;
    }

    // ------------------------------------------------

    void Container::addElement(std::string_view name, Element* el) {
        m_Elements.emplace_back(name, el);
    }

    // ------------------------------------------------

}
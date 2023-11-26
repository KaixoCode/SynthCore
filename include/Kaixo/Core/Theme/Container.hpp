#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    class Container {
    public:

        // ------------------------------------------------
        
        Theme* self;

        // ------------------------------------------------

        Container(Theme* self) : self(self) {}
        Container(ElementAdder adder) : self(adder.create(this)) {}

        // ------------------------------------------------

        void interpret(const basic_json& json);

        // ------------------------------------------------

        ElementAdder add(std::string_view name) { return { self, this, name }; }

        template<class Ty>
        void addElement(std::string_view name, Ty& obj);

        // ------------------------------------------------

    private:
        struct Entry {
            std::string_view name;
            std::function<void(const basic_json&)> interpret;
        };

        std::vector<Entry> m_Elements{};
    };

    // ------------------------------------------------

    template<class Ty>
    void Container::addElement(std::string_view name, Ty& obj) {
        m_Elements.emplace_back(name, [obj = &obj](const basic_json& val) { obj->interpret(val); });
    }

    // ------------------------------------------------

    template<class Ty>
    Theme* ElementAdder::create(Ty* obj) {
        m_AddTo->addElement(m_Name, *obj);
        return m_Self;
    }

    // ------------------------------------------------

}
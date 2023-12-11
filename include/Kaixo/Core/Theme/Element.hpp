#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {}
namespace Kaixo::Theme {
    using namespace Kaixo::Gui;

    // ------------------------------------------------

    class Theme;
    class Container;
    class Element;

    // ------------------------------------------------

    class ElementAdder {
    public:

        // ------------------------------------------------

        Theme* create(Element* obj);

        // ------------------------------------------------

    private:
        Theme* m_Self;
        Container* m_AddTo;
        std::string_view m_Name;

        // ------------------------------------------------

        ElementAdder(Theme* self, Container* me, std::string_view name);

        // ------------------------------------------------

        friend class Container;
    };

    // ------------------------------------------------

    class Element {
    public:

        // ------------------------------------------------

        Theme* self;

        // ------------------------------------------------

        Element(Theme* self) : self(self) {}
        Element(ElementAdder adder) : self(adder.create(this)) {}

        Element(const Element&) = delete;
        Element(Element&&) = default;
        Element& operator=(const Element&) = delete;
        Element& operator=(Element&&) = default;

        // ------------------------------------------------

        virtual void interpret(const basic_json&) {};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {}
namespace Kaixo::Theme {
    using namespace Kaixo::Gui;

    // ------------------------------------------------

    class Theme;
    class Container;

    // ------------------------------------------------

    class ElementAdder {
    public:

        // ------------------------------------------------

        template<class Ty>
        Theme* create(Ty* obj);

        // ------------------------------------------------

    private:
        Theme* m_Self;
        Container* m_AddTo;
        std::string_view m_Name;

        // ------------------------------------------------

        ElementAdder(Theme* self, Container* me, std::string_view name)
            :m_Self(self), m_AddTo(me), m_Name(name) {}

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
        Element& operator=(const Element&) = delete;

        // ------------------------------------------------

        virtual void interpret(const json&) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
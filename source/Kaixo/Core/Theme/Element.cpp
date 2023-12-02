
// ------------------------------------------------

#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Theme* ElementAdder::create(Element* obj) {
        m_AddTo->addElement(m_Name, obj);
        return m_Self;
    }

    // ------------------------------------------------

    ElementAdder::ElementAdder(Theme* self, Container* me, std::string_view name)
        :m_Self(self), m_AddTo(me), m_Name(name) {}

    // ------------------------------------------------

}
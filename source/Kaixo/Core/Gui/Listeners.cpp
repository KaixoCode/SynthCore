#include "Kaixo/Core/Gui/Listeners.hpp"
#include "Kaixo/Core/Gui/Window.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Listener::~Listener() { if (m_Window) m_Window->removeListener(this); }

    // ------------------------------------------------

}

// ------------------------------------------------

#include "Kaixo/Core/Editor/Editor.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Gui/Window.hpp"

// ------------------------------------------------

namespace Kaixo::Editor {

    // ------------------------------------------------

#if HAS_INLIDE_UI_EDITOR

    Editor::Editor(Window& window)
        : m_Window(window)
    {

        // ------------------------------------------------

        setSize(1920, 1080);

        addToDesktop(ComponentPeer::StyleFlags::windowHasDropShadow
                   | ComponentPeer::StyleFlags::windowIsResizable
                   | ComponentPeer::StyleFlags::windowHasMinimiseButton
                   | ComponentPeer::StyleFlags::windowHasMaximiseButton
                   | ComponentPeer::StyleFlags::windowHasCloseButton
                   | ComponentPeer::StyleFlags::windowHasTitleBar
                   | ComponentPeer::StyleFlags::windowAppearsOnTaskbar, window.getWindowHandle());

        setVisible(true);

        // ------------------------------------------------

    }

#endif

    // ------------------------------------------------

}
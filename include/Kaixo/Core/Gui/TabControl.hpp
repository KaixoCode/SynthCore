#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    constexpr ViewAttribute<std::size_t, "HiddenInThisManyTabControls"> HiddenInThisManyTabControls{};

    // ------------------------------------------------

    class TabControl {
    public:

        // ------------------------------------------------

        using Id = std::size_t;

        // ------------------------------------------------

        constexpr static Id None = static_cast<Id>(-1);

        // ------------------------------------------------

        Id addTab();

        // ------------------------------------------------

        void    remove   (Id id, View& view);
        View&   add      (Id id, View& view);
        Button& addButton(Id id, Button& button);

        // ------------------------------------------------

        void select(Id i);
        Id   selected();

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        struct Tab {

            // ------------------------------------------------

            void select(bool v);
            bool selected();

            // ------------------------------------------------
            
            void addCallback(std::function<void(bool)> fun);

            // ------------------------------------------------
            
            void addButton(Button& button);

            // ------------------------------------------------

        private:
            std::vector<juce::Component::SafePointer<Button>> m_Buttons;
            std::vector<std::function<void(bool)>> m_Callbacks;
            bool m_Selected = false;

            // ------------------------------------------------
            
            void removeDeletedComponents();

            // ------------------------------------------------

        };

        // ------------------------------------------------

        std::map<juce::Component::SafePointer<View>, std::set<Id>> m_Views;
        std::map<Id, Tab> m_Tabs{};

        // ------------------------------------------------
        
    public:

        // ------------------------------------------------

        Tab& tab(Id i) { return m_Tabs[i]; }

        // ------------------------------------------------

        void removeDeletedComponents();

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
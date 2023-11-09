#include "Kaixo/Core/Gui/TabControl.hpp"
#include "Kaixo/Core/Gui/Button.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    TabControl::Id TabControl::addTab() {
        Id newId{}; // Generate new unique id
        for (auto& [id, tab] : m_Tabs) {
            if (newId <= id) newId = id + 1;
        }

        m_Tabs.emplace(newId, Tab{});
        return newId;
    }

    // ------------------------------------------------

    void TabControl::remove(Id id, View& view) {
        m_Views[&view].erase(id);
        if (m_Views[&view].size() == 0) {
            m_Views.erase(&view);
        }

        if (!m_Tabs[id].selected()) {
            auto& count = view.attribute(HiddenInThisManyTabControls);
            count -= 1;
            if (count == 0) view.setVisible(true);
        }
    }

    View& TabControl::add(Id id, View& view) {
        bool alreadyInThisTabView = m_Views.contains(&view);

        m_Views[&view].insert(id);

        view.setVisible(m_Tabs[id].selected());
        if (!m_Tabs[id].selected() && !alreadyInThisTabView) {
            view.attribute(HiddenInThisManyTabControls) += 1;
        }

        return view;
    }

    Button& TabControl::addButton(Id id, Button& button) {
        m_Tabs[id].addButton(button);
        button.settings.callback = [this, index = id](bool val) {
            if (val) select(index);
            else if (selected() == index) select(None);
        };
        button.selected(m_Tabs[id].selected());
        return button;
    }

    // ------------------------------------------------

    void TabControl::select(Id i) {
        if (i == selected()) return; // Ignore already selected

        Id previous = selected();

        removeDeletedComponents();

        for (auto& [view, ids] : m_Views) {
            bool partOfSelectedTab = false;
            bool partOfPreviouslySelectedTab = false;
            for (auto& id : ids) {
                if (id == i) partOfSelectedTab = true;
                if (id == previous) partOfPreviouslySelectedTab = true;
            }

            if (partOfPreviouslySelectedTab && !partOfSelectedTab) {
                view->attribute(HiddenInThisManyTabControls) += 1;
                view->setVisible(false);
            } else if (partOfSelectedTab && !partOfPreviouslySelectedTab) {
                auto& count = view->attribute(HiddenInThisManyTabControls);
                count -= 1;
                if (count == 0) view->setVisible(true);
            }
        }

        for (auto& [id, tab] : m_Tabs) {
            tab.select(id == i);
        }
    }

    TabControl::Id TabControl::selected() {
        for (auto& [id, tab] : m_Tabs) {
            if (tab.selected()) return id;
        }
        return None;
    }

    // ------------------------------------------------

    void TabControl::removeDeletedComponents() {
        auto it = m_Views.begin();
        while (it != m_Views.end()) {
            if (!(*it).first.getComponent()) {
                it = m_Views.erase(it);
            } else ++it;
        }
    }

    // ------------------------------------------------

    void TabControl::Tab::select(bool v) {
        if (m_Selected == v) return;
        m_Selected = v;

        removeDeletedComponents();

        for (auto& callback : m_Callbacks) callback(v);
        
        if (v) {
            for (auto& button : m_Buttons) {
                if (!button->selected()) button->repaint();
                button->selected(true);
            }
        } else {
            for (auto& button : m_Buttons) {
                if (button->selected()) button->repaint();
                button->selected(false);
            }
        }
    }

    // ------------------------------------------------

    bool TabControl::Tab::selected() { return m_Selected; }

    // ------------------------------------------------

    void TabControl::Tab::addCallback(std::function<void(bool)> fun) {
        m_Callbacks.push_back(std::move(fun));
    }

    // ------------------------------------------------

    void TabControl::Tab::addButton(Button& button) {
        m_Buttons.push_back(&button);
    }

    // ------------------------------------------------
    
    void TabControl::Tab::removeDeletedComponents() {
        for (std::size_t i = 0; i < m_Buttons.size(); ++i) {
            if (!m_Buttons[i].getComponent()) {
                m_Buttons.erase(m_Buttons.begin() + i);
            }
            else ++i;
        }
    }

    // ------------------------------------------------

}
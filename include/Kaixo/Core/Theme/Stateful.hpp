#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    template<class Ty>
    struct Stateful : Animation {

        // ------------------------------------------------

        struct State {
            View::State state;
            Ty value;
        };

        // ------------------------------------------------

        mutable Ty current;
        std::vector<State> states;

        // ------------------------------------------------
        
        bool changing() const override {
            if constexpr (std::derived_from<Ty, Animation>) return current.changing();
            return false;
        }

        // ------------------------------------------------

        void interpret(Theme& self, const basic_json& theme) {

            // ------------------------------------------------

            current = {}; // clear current
            states.clear();
            State& state = states.emplace_back(View::State::Default);
            state.value.interpret(self, theme);

            // ------------------------------------------------

            theme.foreach([&](std::string_view key, const basic_json& theme) {
                View::State state = View::State::Default;
                if (key.contains("hovering")) state |= View::State::Hovering;
                if (key.contains("disabled")) state |= View::State::Disabled;
                if (key.contains("selected")) state |= View::State::Selected;
                if (key.contains("pressed")) state |= View::State::Pressed;
                if (key.contains("focused")) state |= View::State::Focused;
                if (key.contains("enabled")) state |= View::State::Enabled;
                if (state != View::State::Default) {
                    State& s = states.emplace_back(state);
                    s.value.interpret(self, theme);
                }
            });

            // ------------------------------------------------

        }

        // ------------------------------------------------

        template<class ...Args>
        Ty& state(View::State state) const {

            // ------------------------------------------------

            Ty result{};

            for (const State& s : states) {
                bool matches = (s.state & state) == s.state;
                if (!matches) continue;

                result.merge(s.value);
            }

            current.merge(result);

            return current;

            // ------------------------------------------------

        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
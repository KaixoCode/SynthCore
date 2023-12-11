#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Animated.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    template<class Ty>
    struct StateLinked {

        // ------------------------------------------------

        struct State {
            View::State state;
            Ty value;
        };

        // ------------------------------------------------

        Ty base;
        std::vector<State> states{};

        // ------------------------------------------------

        const Ty& operator[](View::State state) const {
            const Ty* match = &base;
            for (const State& s : states) {
                if ((s.state & state) == s.state) {
                    match = &s.value;
                }
            }

            return *match;
        }

        // ------------------------------------------------

        void reset() { states.clear(); }

        // ------------------------------------------------

        void interpret(const basic_json& theme, auto interpret, View::State state = View::State::Default) {
            if (state == View::State::Default) {
                interpret(base, theme);

                theme.foreach([&](std::string_view key, const basic_json& theme) {
                    View::State state = View::State::Default;
                    if (key.contains("hovering")) state |= View::State::Hovering;
                    if (key.contains("disabled")) state |= View::State::Disabled;
                    if (key.contains("selected")) state |= View::State::Selected;
                    if (key.contains("pressed")) state |= View::State::Pressed;
                    if (key.contains("focused")) state |= View::State::Focused;
                    if (key.contains("enabled")) state |= View::State::Enabled;
                    if (state != View::State::Default) {
                        Ty result;
                        if (interpret(result, theme)) {
                            states.emplace_back(state, std::move(result));
                        }
                    }
                });
            } else {
                Ty result;
                if (interpret(result, theme)) {
                    states.emplace_back(state, std::move(result));
                }
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty>
    struct StateLinked<Animated<Ty>> {

        // ------------------------------------------------

        struct State {
            View::State state;
            Ty value;
            std::optional<double> transition;
        };

        // ------------------------------------------------

        Ty base;
        std::vector<State> states{};
        double transition = 0;

        // ------------------------------------------------

        Animated<Ty>::Assign operator[](View::State state) const {
            double trns = transition;
            const Ty* match = &base;
            for (const State& s : states) {
                if ((s.state & state) == s.state) {
                    if (s.transition.has_value()) trns = s.transition.value();
                    match = &s.value;
                }
            }

            return { *match, trns };
        }

        // ------------------------------------------------
        
        void reset() { states.clear(); }

        // ------------------------------------------------

        void interpret(const basic_json& theme, auto interpret, View::State state = View::State::Default) {
            if (state == View::State::Default) {
                if (interpret(base, theme)) {
                    if (theme.contains("transition", basic_json::Number)) {
                        transition = theme["transition"].as<double>();
                    }
                }

                theme.foreach([&](std::string_view key, const basic_json& theme) {
                    View::State state = View::State::Default;
                    if (key.contains("hovering")) state |= View::State::Hovering;
                    if (key.contains("disabled")) state |= View::State::Disabled;
                    if (key.contains("selected")) state |= View::State::Selected;
                    if (key.contains("pressed")) state |= View::State::Pressed;
                    if (key.contains("focused")) state |= View::State::Focused;
                    if (key.contains("enabled")) state |= View::State::Enabled;
                    if (state != View::State::Default) {
                        Ty result;
                        if (interpret(result, theme)) {
                            State& s = states.emplace_back(state, std::move(result));
                            if (theme.contains("transition", basic_json::Number)) {
                                s.transition = theme["transition"].as<double>();
                            }
                        }
                    }
                });
            } else {
                Ty result;
                if (interpret(result, theme)) {
                    State& s = states.emplace_back(state, std::move(result));
                    if (theme.contains("transition", basic_json::Number)) {
                        s.transition = theme["transition"].as<double>();
                    }
                }
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Animated.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/ExpressionParser.hpp"

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
                if (theme.contains("value")) {
                    interpret(base, theme["value"], state);
                } else {
                    interpret(base, theme, state);
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
                        if (interpret(result, theme, state)) {
                            states.emplace_back(state, std::move(result));
                        }
                    }
                });
            } else {
                Ty result;
                if (interpret(result, theme, state)) {
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
            std::optional<std::function<double(double)>> curve;
        };

        // ------------------------------------------------

        Ty base;
        std::vector<State> states{};
        double transition = 0;
        std::function<double(double)> curve{};

        // ------------------------------------------------
        
        Animated<Ty>::Assign operator[](View::State state) const {
            double trns = transition;
            auto crv = curve;
            const Ty* match = &base;
            for (const State& s : states) {
                if ((s.state & state) == s.state) {
                    if (s.transition.has_value()) trns = s.transition.value();
                    if (s.curve.has_value()) crv = s.curve.value();
                    match = &s.value;
                }
            }

            return { *match, trns, crv };
        }

        // ------------------------------------------------
        
        void reset() { 
            base = {};
            transition = 0;
            curve = {};
            states.clear();
        }

        // ------------------------------------------------

        void interpret(const basic_json& theme, auto interpret, View::State state = View::State::Default, const ExpressionParser::FunctionMap& funs = {}) {
            const auto parseCurve = [&](auto& curve, std::string_view expression) {

                if (expression == "ease-in") curve = [](double x) { return Easing::easeInCubic(x); };
                else if (expression == "ease-out") curve = [](double x) { return Easing::easeOutCubic(x); };
                else if (expression == "ease-in-out") curve = [](double x) { return Easing::easeInOutCubic(x); };
                else if (expression == "ease") curve = [](double x) { return Easing::easeInOutCubic(x); };
                else if (expression == "ease-in-sine") curve = [](double x) { return Easing::easeInSine(x); };
                else if (expression == "ease-out-sine") curve = [](double x) { return Easing::easeOutSine(x); };
                else if (expression == "ease-in-out-sine") curve = [](double x) { return Easing::easeInOutSine(x); };
                else if (expression == "ease-in-quad") curve = [](double x) { return Easing::easeInQuad(x); };
                else if (expression == "ease-out-quad") curve = [](double x) { return Easing::easeOutQuad(x); };
                else if (expression == "ease-in-out-quad") curve = [](double x) { return Easing::easeInOutQuad(x); };
                else if (expression == "ease-in-cubic") curve = [](double x) { return Easing::easeInCubic(x); };
                else if (expression == "ease-out-cubic") curve = [](double x) { return Easing::easeOutCubic(x); };
                else if (expression == "ease-in-out-cubic") curve = [](double x) { return Easing::easeInOutCubic(x); };
                else if (expression == "ease-in-quart") curve = [](double x) { return Easing::easeInQuart(x); };
                else if (expression == "ease-out-quart") curve = [](double x) { return Easing::easeOutQuart(x); };
                else if (expression == "ease-in-out-quart") curve = [](double x) { return Easing::easeInOutQuart(x); };
                else if (expression == "ease-in-quint") curve = [](double x) { return Easing::easeInQuint(x); };
                else if (expression == "ease-out-quint") curve = [](double x) { return Easing::easeOutQuint(x); };
                else if (expression == "ease-in-out-quint") curve = [](double x) { return Easing::easeInOutQuint(x); };
                else if (expression == "ease-in-expo") curve = [](double x) { return Easing::easeInExpo(x); };
                else if (expression == "ease-out-expo") curve = [](double x) { return Easing::easeOutExpo(x); };
                else if (expression == "ease-in-out-expo") curve = [](double x) { return Easing::easeInOutExpo(x); };
                else if (expression == "ease-in-circ") curve = [](double x) { return Easing::easeInCirc(x); };
                else if (expression == "ease-out-circ") curve = [](double x) { return Easing::easeOutCirc(x); };
                else if (expression == "ease-in-out-circ") curve = [](double x) { return Easing::easeInOutCirc(x); };
                else if (expression == "ease-in-back") curve = [](double x) { return Easing::easeInBack(x); };
                else if (expression == "ease-out-back") curve = [](double x) { return Easing::easeOutBack(x); };
                else if (expression == "ease-in-out-back") curve = [](double x) { return Easing::easeInOutBack(x); };
                else if (expression == "ease-in-elastic") curve = [](double x) { return Easing::easeInElastic(x); };
                else if (expression == "ease-out-elastic") curve = [](double x) { return Easing::easeOutElastic(x); };
                else if (expression == "ease-in-out-elastic") curve = [](double x) { return Easing::easeInOutElastic(x); };
                else {
                    auto fun = ExpressionParser::parseFunction(expression, funs);
                    if (fun.nofArgs != 1) return; // Invalid expression

                    curve = [fun = std::move(fun)](double v) -> double {
                        std::vector<float> vals{ static_cast<float>(v) };
                        return static_cast<double>(fun.f(vals));
                    };
                }
            };
            
            if (state == View::State::Default) {
                if (theme.contains<basic_json::number_t>("transition")) {
                    transition = theme["transition"].as<double>();

                    if (theme.contains("value")) 
                        interpret(base, theme["value"], state);

                    if (theme.contains<basic_json::string_t>("curve")) {
                        parseCurve(curve, theme["curve"].as<std::string_view>());
                    }
                } else if (theme.contains("value")) {
                    interpret(base, theme["value"], state);
                } else {
                    interpret(base, theme, state);
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
                        if (theme.contains<basic_json::number_t>("transition")) {
                            if (theme.contains("value")) interpret(result, theme["value"], state);
                            State& s = states.emplace_back(state, std::move(result));
                            s.transition = theme["transition"].as<double>();
                            if (theme.contains<basic_json::string_t>("curve")) {
                                parseCurve(s.curve, theme["curve"].as<std::string_view>());
                            }
                        } else if (theme.contains("value")) {
                            interpret(result, theme["value"], state);
                            State& s = states.emplace_back(state, std::move(result));
                            if (theme.contains<basic_json::string_t>("curve")) {
                                parseCurve(s.curve, theme["curve"].as<std::string_view>());
                            }
                        } else if (interpret(result, theme, state)) {
                            State& s = states.emplace_back(state, std::move(result));
                        }
                    }
                });
            } else {
                Ty result;
                if (theme.contains<basic_json::number_t>("transition")) {
                    if (theme.contains("value")) interpret(result, theme["value"], state);
                    State& s = states.emplace_back(state, std::move(result));
                    s.transition = theme["transition"].as<double>();
                    if (theme.contains<basic_json::string_t>("curve")) {
                        parseCurve(s.curve, theme["curve"].as<std::string_view>());
                    }
                } else if (theme.contains("value")) {
                    interpret(result, theme["value"], state);
                    State& s = states.emplace_back(state, std::move(result));
                    if (theme.contains<basic_json::string_t>("curve")) {
                        parseCurve(s.curve, theme["curve"].as<std::string_view>());
                    }
                } else if (interpret(result, theme, state)) {
                    State& s = states.emplace_back(state, std::move(result));
                }
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
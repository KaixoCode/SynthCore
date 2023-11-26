#include "Kaixo/Core/Theme/Stateful.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Stateful::Stateful(std::unique_ptr<Interface> graphics) 
        : m_Graphics(std::move(graphics)) 
    {}

    // ------------------------------------------------

    void Stateful::draw(juce::Graphics& g, const Rect<float>& pos, View::State state, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, pos, state, align);
    }

    // ------------------------------------------------

    void StatefulElement::State::interpret(const basic_json& in) {

        // ------------------------------------------------

        basic_json theme = in;

        // ------------------------------------------------

        if (in.contains("extends")) {
            theme.merge(in["extends"]);
            in["extends"].foreach([&](const basic_json& val) { theme.merge(val); });
        }

        // ------------------------------------------------
        
        auto layer = [&](const basic_json& theme) {

            // ------------------------------------------------

            Layer& layer = layers.emplace_back(self);
            if (theme.contains("image", basic_json::String)) {
                layer.id = self->registerImage(theme["image"].as<basic_json::string>());
            }

            // ------------------------------------------------

            layer.clip = interpretClip(theme);

            // ------------------------------------------------

            if (auto _tiles = interpretTiles(theme)) {
                layer.isTiled = true;
                layer.tiles = _tiles.value();
            }

            // ------------------------------------------------

            if (theme.contains("font")) {
                layer.font.interpret(self->variableOrValue(theme["font"]));
            }
                
            // ------------------------------------------------

            if (theme.contains("text", basic_json::String)) {
                layer.text = theme["text"].as<basic_json::string>();
            }

            // ------------------------------------------------

            if (theme.contains("text-color")) {
                layer.fillAlphaWithColor = true;
                layer.textColor.interpret(self->variableOrValue(theme["text-color"]));
            }

            // ------------------------------------------------

            std::array<int, 2> arr2;
            if (theme.try_get("text-offset", arr2)) {
                layer.textOffset.x(arr2[0]);
                layer.textOffset.y(arr2[1]);
            }

            // ------------------------------------------------
                
            layer.textAlign = Align::Center;
            if (theme.contains("text-align", basic_json::String)) {
                auto& align = theme["text-align"].as<basic_json::string>();
                layer.textAlign = alignFromString(align);
            }

            // ------------------------------------------------

            if (theme.contains("background-color")) {
                layer.hasBackgroundColor = true;
                layer.backgroundColor.interpret(self->variableOrValue(theme["background-color"]));
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        layers.clear(); // remove previous layers
        layer(theme);
        if (theme.contains("layers", basic_json::Array)) {
            for (auto& l : theme["layers"].as<basic_json::array>())
                layer(l);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    bool StatefulElement::State::Layer::draw(juce::Graphics& g, const Rect<float>& pos, Align align) const {
         bool didDraw = false;

        if (hasBackgroundColor) {
            g.setColour(backgroundColor);
            g.fillRect(pos);
            didDraw = true;
        }

        if (auto image = self->image(id)) {
            image.draw(ClippedInstruction{
                .graphics = g,
                .tiled = isTiled ? &tiles : nullptr,
                .clip = clip,
                .align = align,
                .position = pos
            });
            didDraw = true;
        }

        if (!text.empty()) {
            Point<float> at = pointFromAlign(textAlign, pos) + textOffset.toFloat();

            if (fillAlphaWithColor) g.setColour(textColor);
            font.draw(g, at, text, textAlign, fillAlphaWithColor);
            didDraw = true;
        }

        return didDraw;
    }

    // ------------------------------------------------

    bool StatefulElement::State::draw(juce::Graphics& g, const Rect<float>& pos, Align align) const {
        bool didDraw = false;
        for (auto& layer : layers) {
            if (layer.draw(g, pos, align)) didDraw = true;
        }
        return didDraw;
    }

    // ------------------------------------------------

    void StatefulElement::interpret(const basic_json& in) {

        // ------------------------------------------------

        if (!in.is(basic_json::Object)) return;

        // ------------------------------------------------

        basic_json theme = in;

        // ------------------------------------------------
        
        states.clear();

        // ------------------------------------------------
        
        for (auto& [str, state] : interpretState(theme)) {
            theme[str].merge(theme);
            State& s = states.emplace_back();
            s.state = state;
            s.self = self;
            s.interpret(theme[str]);
        }

        // ------------------------------------------------

        State& base = states.emplace_back();
        base.state = View::State::Default;
        base.self = self;
        base.interpret(theme);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void StatefulElement::draw(juce::Graphics& g, const Rect<float>& pos, View::State state, Align align) const {
        for (auto& s : states) {
            if ((s.state & state) == s.state) {
                if (s.draw(g, pos, align)) return;
            }
        }
    }
    
    // ------------------------------------------------
    
    StatefulElement::operator Stateful() const {
        struct Implementation : public Stateful::Interface {
            Implementation(const StatefulElement* self) : self(self) {}

            const StatefulElement* self;

            void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self->draw(g, pos, state, align);
            }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------
    
    std::vector<std::pair<std::string, View::State>> interpretState(const basic_json& theme) {

        // ------------------------------------------------

        using enum View::State;
        constexpr std::array<std::pair<std::string_view, View::State>, 10> map{ {
            { "hovering", Hovering },
            { "pressed", Pressed },
            { "enabled", Enabled },
            { "disabled", Disabled },
            { "selected", Selected },
            { "focused", Focused },
        } };

        // ------------------------------------------------

        std::vector<std::pair<std::string, View::State>> states;
        std::vector<std::size_t> precedence;

        // ------------------------------------------------

        for (std::size_t i = 1; i < 64; ++i) {

            // ------------------------------------------------

            std::string key{};
            View::State state = Default;

            // ------------------------------------------------

            bool first = true;
            for (std::size_t j = 0; j < map.size(); ++j) {
                if (i & (1ull << j)) {
                    if (!first) key += '-'; // delimiter
                    key += map[j].first;
                    state |= map[j].second;
                    first = false;
                }
            }

            // ------------------------------------------------

            auto& obj = theme.as<basic_json::object>();
            auto it = obj.find(key);
            if (it == obj.end()) continue;

            // ------------------------------------------------

            bool added = false;
            auto index = std::distance(obj.begin(), it);
            for (std::size_t j = 0; j < states.size(); ++j) {
                if (index < precedence[j]) {
                    states.insert(states.begin() + j, { key, state });
                    precedence.insert(precedence.begin() + j, index);
                    added = true;
                    break;
                }
            }

            // ------------------------------------------------

            if (!added) {
                states.push_back({ key, state });
                precedence.push_back(index);
            }

            // ------------------------------------------------

        }

        // ------------------------------------------------

        return states;

        // ------------------------------------------------

    }

    // ------------------------------------------------

}
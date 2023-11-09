#include "Kaixo/Core/Theme/Stateful.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"

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

    void StatefulElement::State::interpret(const json& theme, ZoomMultiplier zoom) {
        
        // ------------------------------------------------

        if (theme.contains("image", json::String)) {
            auto& path = theme["image"].as<json::string>();
            id = self->registerImage(path, zoom);
        } else {
            id = NoImage;
        }

        // ------------------------------------------------

        state = interpretState(theme);
        clip = interpretClip(theme);

        // ------------------------------------------------

        if (auto _tiles = interpretTiles(theme)) {
            isTiled = true;
            tiles = _tiles.value();
        } else {
            isTiled = false;
            tiles = {};
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    bool StatefulElement::State::draw(juce::Graphics& g, const Rect<float>& pos, Align align) const {
        if (auto image = self->image(id)) {
            image.draw(ClippedInstruction{
                .graphics = g,
                .tiled = isTiled ? &tiles : nullptr,
                .clip = clip,
                .align = align,
                .position = pos
            });

            return true;
        }
        return false;
    }

    // ------------------------------------------------

    void StatefulElement::interpret(const json& theme) {
        if (!theme.is(json::Array)) return;
        auto& arr = theme.as<json::array>();
        zoomLevel.clear();
        for (auto& el : arr) {
            ZoomMultiplier zoom = el.contains("zoom", json::Floating) ? el["zoom"].as<json::floating>() : 1;
            State state{ self };
            state.interpret(el, zoom);
            if (state.id != NoImage)
                zoomLevel[zoom].states.push_back(std::move(state));
        }
    }

    // ------------------------------------------------

    void StatefulElement::draw(juce::Graphics& g, const Rect<float>& pos, View::State state, Align align) const {
        if (auto _level = self->pickZoom(zoomLevel)) {
            for (auto& s : _level->states) {
                if ((s.state & state) == s.state) {
                    if (s.draw(g, pos, align)) return;
                }
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
    
    View::State interpretState(const json& theme) {

        // ------------------------------------------------
        
        View::State state = View::State::Default;
        if (theme.contains("states", json::Array)) {
            for (auto& s : theme["states"].as<json::array>()) {
                if (!s.is(json::String)) continue;
                if (s.as<json::string>() == "disabled") state |= View::State::Disabled;
                if (s.as<json::string>() == "hovering") state |= View::State::Hovering;
                if (s.as<json::string>() == "pressed")  state |= View::State::Pressed;
                if (s.as<json::string>() == "enabled")  state |= View::State::Enabled;
                if (s.as<json::string>() == "selected") state |= View::State::Selected;
                if (s.as<json::string>() == "focused")  state |= View::State::Focused;
            }
        }

        // ------------------------------------------------
        
        return state;

        // ------------------------------------------------

    }

    // ------------------------------------------------

}
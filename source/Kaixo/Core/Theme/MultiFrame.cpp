#include "Kaixo/Core/Theme/MultiFrame.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Parameter.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    MultiFrame::MultiFrame(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    void MultiFrame::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, i, pos, state, align);
    }

    void MultiFrame::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, i, pos, state, align);
    }

    // ------------------------------------------------

    void MultiFrame::link(ParamID id) { if (m_Graphics) m_Graphics->link(id); }

    // ------------------------------------------------

    void MultiFrameElement::State::interpret(const basic_json& theme, MultiFrameElement* parent) {

        // ------------------------------------------------
        
        auto layer = [&](const basic_json& theme) {

            // ------------------------------------------------

            Layer& layer = layers.emplace_back(self, parent);
            if (theme.contains("image", basic_json::String)) {
                layer.id = self->registerImage(theme["image"].as<basic_json::string>());
            }

            // ------------------------------------------------
            
            std::array<int, 2> arr2;
            if (theme.try_get("image-position", arr2)) {
                layer.imagePosition.x(arr2[0]);
                layer.imagePosition.y(arr2[1]);
            }

            // ------------------------------------------------

            layer.description = interpretMultiFrame(theme);
            layer.clip = interpretClip(theme);

            // ------------------------------------------------

            if (auto _tiles = interpretTiles(theme)) {
                layer.isTiled = true;
                layer.tiles = _tiles.value();
            }

            // ------------------------------------------------

            if (theme.contains("font")) {
                layer.font.interpret(theme["font"]);
            }
                
            // ------------------------------------------------

            if (theme.contains("text", basic_json::String)) {
                layer.text.push_back(theme["text"].as<basic_json::string>());
            } else if (theme.contains("text", basic_json::Array)) {
                for (auto& text : theme["text"].as<basic_json::array>()) {
                    if (text.is(basic_json::String)) {
                        layer.text.push_back(text.as<basic_json::string>());
                    }
                }
            }

            // ------------------------------------------------

            if (theme.contains("text-color")) {
                layer.fillAlphaWithColor = true;
                layer.textColor.interpret(theme["text-color"]);
            }

            // ------------------------------------------------

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

        frames = interpretMultiFrame(theme).numFrames;

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
        
    bool MultiFrameElement::State::Layer::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align imageAlign) const {
        bool didDraw = false;

        if (hasBackgroundColor) {
            g.setColour(backgroundColor);
            g.fillRect(pos);
            didDraw = true;
        }

        if (auto image = self->image(id)) {
            image.draw(FrameInstruction{
                .graphics = g,
                .description = description,
                .tiled = isTiled ? &tiles : nullptr,
                .clip = clip,
                .align = imageAlign,
                .frame = normalToIndex(i, description.numFrames),
                .position = imagePosition.topLeft(),
                .bounds = pos,
            });
            didDraw = true;
        }

        if (!text.empty()) {
            Point<float> at = pointFromAlign(textAlign, pos) + textOffset.toFloat();

            auto format = [&](std::string value) {
                replace_str(value, "$frame", std::to_string(normalToIndex(i, description.numFrames) + 1));
                replace_str(value, "$0frame", std::to_string(normalToIndex(i, description.numFrames)));
                replace_str(value, "$normalized-value", std::to_string(i));
                if (parent->parameter == NoParam) return value;
                auto& param = Kaixo::parameter(parent->parameter);
                replace_str(value, "$value", param.toString(i));
                replace_str(value, "$name", param.name);
                replace_str(value, "$short-name", param.shortName);
                replace_str(value, "$identifier", param.identifier);
                replace_str(value, "$short-identifier", param.shortIdentifier);
                replace_str(value, "$var-name", param.varName);
                replace_str(value, "$full-var-name", param.fullVarName);
                replace_str(value, "$steps", std::to_string(param.steps));
                return value;
            };

            std::size_t index = Math::clamp(normalToIndex(i, text.size()), 0, text.size() - 1);
            if (fillAlphaWithColor) g.setColour(textColor);
            font.draw(g, at, format(text[index]), textAlign, fillAlphaWithColor);
            didDraw = true;
        }

        return didDraw;
    }

    // ------------------------------------------------

    bool MultiFrameElement::State::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, Align align) const {
        bool didDraw = false;
        for (auto& layer : layers) {
            if (layer.draw(g, i, pos, align)) didDraw = true;
        }
        return didDraw;
    }

    bool MultiFrameElement::State::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, Align align) const {
        return draw(g, static_cast<ParamValue>(i) / (frames - 1), pos, align);
    }

    // ------------------------------------------------

    void MultiFrameElement::interpret(const basic_json& in) {

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
            s.interpret(theme[str], this);
        }

        // ------------------------------------------------

        State& base = states.emplace_back();
        base.state = View::State::Default;
        base.self = self;
        base.interpret(theme, this);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void MultiFrameElement::draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state, Align align) const {
        for (auto& s : states) {
            if ((s.state & state) == s.state) {
                if (s.draw(g, i, pos, align)) return;
            }
        }
    }

    void MultiFrameElement::draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state, Align align) const {
        for (auto& s : states) {
            if ((s.state & state) == s.state) {
                if (s.draw(g, i, pos, align)) return;
            }
        }
    }

    // ------------------------------------------------

    MultiFrameElement::operator MultiFrame() {
        struct Implementation : public MultiFrame::Interface {
            Implementation(MultiFrameElement* self) : self(self) {}

            MultiFrameElement* self;

            void draw(juce::Graphics& g, std::size_t i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self->draw(g, i, pos, state, align);
            }

            void draw(juce::Graphics& g, ParamValue i, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self->draw(g, i, pos, state, align);
            }

            void link(ParamID id) override { self->parameter = id; }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

    void MultiFrameElement::Index::draw(juce::Graphics& g, const Rect<float>& pos, View::State state, Align align) const {
        m_Self->draw(g, m_Index, pos, state, align);
    }

    // ------------------------------------------------
    
    MultiFrameElement::Index::operator Stateful() const {
        struct Implementation : public Stateful::Interface {
            Implementation(Index self) : self(self) {}

            Index self;

            void draw(juce::Graphics& g, const Rect<float>& pos, View::State state = View::State::Default, Align align = Align::Center) const override {
                self.draw(g, pos, state, align);
            }
        };

        return { std::make_unique<Implementation>(*this) };
    }

    // ------------------------------------------------

    MultiFrameDescription interpretMultiFrame(const basic_json& theme) {
        MultiFrameDescription result{};

        if (theme.contains("frames", basic_json::Number))
            result.numFrames = theme["frames"].as<std::size_t>();

        if (theme.contains("frames-per-row", basic_json::Number))
            result.framesPerRow = theme["frames-per-row"].as<std::size_t>();

        return result;
    }

    // ------------------------------------------------

}
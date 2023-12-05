#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    void DrawableElement::ImageElement::interpret(Theme& self, const basic_json& json) {

        // ------------------------------------------------

        std::array<std::size_t, 4> edg4{};
        std::array<std::size_t, 2> edg2{};
        std::array<int, 4> arr4{};
        std::array<int, 2> arr2{};
        std::string str;

        // ------------------------------------------------

        bool containsImage = json.contains("image", basic_json::Object);

        // ------------------------------------------------

        if (json.try_get("image", str) ||
            containsImage && json["image"].try_get("source", str)) 
        {
            image = self.registerImage(str);
        }

        // ------------------------------------------------

        if (json.contains("frames", basic_json::Number)) {
            std::size_t frames = json["frames"].as<std::size_t>();
            std::size_t fprow = 1;
            json.try_get("frames-per-row", fprow);
            multiframe = MultiFrameDescription{ frames, fprow };
        } else if (containsImage && json["image"].contains("frames", basic_json::Number)) {
            std::size_t frames = json["image"]["frames"].as<std::size_t>();
            std::size_t fprow = 1;
            json["image"].try_get("frames-per-row", fprow);
            multiframe = MultiFrameDescription{ frames, fprow };
        }
        
        // ------------------------------------------------

        if (json.try_get("edges", edg2)) {
            tiled = TiledDescription{ edg2[0], edg2[1], edg2[0], edg2[1] };
        } else if (json.try_get("edges", edg4)) {
            tiled = TiledDescription{ edg4[0], edg4[1], edg4[2], edg4[3] };
        }

        // ------------------------------------------------

        if (json.try_get("image-position", arr2) || 
            containsImage && json["image"].try_get("position", arr2)) 
        {
            position = Point{ arr2[0], arr2[1] };
        }

        // ------------------------------------------------

        if (json.try_get("image-clip", arr4) ||
            containsImage && json["image"].try_get("clip", arr4)) 
        {
            clip = Rect{ arr4[0], arr4[1], arr4[2], arr4[3] };
        }

        // ------------------------------------------------

        if (json.try_get("image-align", str) ||
            containsImage && json["image"].try_get("align", str))
        {
            align = alignFromString(str);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    bool DrawableElement::ImageElement::draw(Theme& self, Drawable::Instruction instr) {

        // ------------------------------------------------
        
        if (!image) return false;

        // ------------------------------------------------

        if (auto img = self.image(image)) {

            // ------------------------------------------------

            auto _align = align ? *align : Align::TopLeft;
            auto _position = position ? *position : Point{ 0, 0 };

            // ------------------------------------------------

            if (multiframe) {

                // ------------------------------------------------

                int _width = img->getWidth() / multiframe->framesPerRow;
                int _height = img->getHeight() / Math::ceil(multiframe->numFrames / static_cast<float>(multiframe->framesPerRow));
                auto _clip = clip ? *clip : Rect{ 0, 0, _width, _height };

                // ------------------------------------------------

                auto _index = 0ull;
                if (instr.index != npos) _index = instr.index;
                else if (instr.value != -1) _index = normalToIndex(instr.value, multiframe->numFrames);

                // ------------------------------------------------

                img.draw(FrameInstruction{
                    .graphics = instr.graphics,
                    .description = *multiframe,
                    .tiled = tiled.get(),
                    .clip = _clip,
                    .align = _align,
                    .frame = _index,
                    .position = Rect<float>{
                        _position.x() + instr.position.x(),
                        _position.y() + instr.position.y(),
                        instr.position.width(), instr.position.height(),
                    },
                });

                // ------------------------------------------------

            } else {

                // ------------------------------------------------

                int _width = img->getWidth();
                int _height = img->getHeight();
                auto _clip = clip ? *clip : Rect{ 0, 0, _width, _height };

                // ------------------------------------------------

                img.draw(ClippedInstruction{
                    .graphics = instr.graphics,
                    .tiled = tiled.get(),
                    .clip = _clip,
                    .align = _align,
                    .position = Rect<float>{
                        _position.x() + instr.position.x(),
                        _position.y() + instr.position.y(),
                        instr.position.width(), instr.position.height(),
                    },
                });

                // ------------------------------------------------

            }

            return true;
        }

        return false;
    }

    // ------------------------------------------------

    void DrawableElement::TextElement::interpret(Theme& self, const basic_json& json) {

        // ------------------------------------------------
        
        bool containsText = json.contains("text", basic_json::Object);

        // ------------------------------------------------

        auto parseContent = [&](const basic_json& json) {
            switch (json.type()) {
            case basic_json::Array:
                content = Content(true);
                json.foreach([&](const basic_json& text) {
                    if (text.is(basic_json::String)) {
                        content->text.push_back(text.as<basic_json::string>());
                    }
                });
                break;
            case basic_json::String:
                content = Content(false);
                content->text.push_back(json.as<basic_json::string>());
                break;
            }
        };

        if (json.contains("text", basic_json::String) || 
            json.contains("text", basic_json::Array)) 
        {
            parseContent(json["text"]);
        } else if (containsText && json["text"].contains("content")) {
            parseContent(json["text"]["content"]);
        }

        // ------------------------------------------------

        std::array<int, 2> arr2{};
        if (json.try_get("text-position", arr2) || 
            containsText && json["text"].try_get("position", arr2))
        {
            position = Point{ arr2[0], arr2[1] };
        }

        // ------------------------------------------------
        
        std::string str;
        if (json.try_get("text-align", str) ||
            containsText && json["text"].try_get("align", str))
        {
            align = alignFromString(str);
        }
        
        // ------------------------------------------------
        
        std::size_t num;
        if (containsText && json["text"].try_get("frames", num))
        {
            frames = num;
        }

        // ------------------------------------------------
        
        if (json.contains("text-color")) {
            color = ColorElement{ &self };
            color->interpret(json["text-color"]);
        } else if (containsText && json["text"].contains("color")) {
            color = ColorElement{ &self };
            color->interpret(json["text"]["color"]);
        }

        // ------------------------------------------------
                
        if (json.contains("font")) {
            font = FontElement{ &self };
            font->interpret(json["font"]);
        } else if (containsText && json["text"].contains("font")) {
            font = FontElement{ &self };
            font->interpret(json["text"]["font"]);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    bool DrawableElement::TextElement::draw(Theme& self, Drawable::Instruction instr) {

        // ------------------------------------------------

        if (!(font && content)) return false;

        // ------------------------------------------------

        auto _align = align ? *align : Align::Center;
        auto _position = position ? *position : Point{ 0, 0 };

        // ------------------------------------------------

        auto _frames = 1ull;
        if (frames) _frames = *frames;
        else if (content->wasArray) _frames = content->text.size();

        // ------------------------------------------------

        auto _index = 0ull;
        if (instr.index != npos) _index = instr.index;
        else if (instr.value != -1) _index = normalToIndex(instr.value, _frames);

        // ------------------------------------------------

        auto _value = _index / static_cast<float>(_frames);
        if (instr.value != -1) _value = instr.value;

        // ------------------------------------------------
                
        auto _text = 0;
        if (content->wasArray) {
            if (instr.index != npos) _text = Math::clamp(instr.index, 0, content->text.size() - 1);
            else _text = normalToIndex(_value, content->text.size());
        }

        // ------------------------------------------------

        auto format = [&](std::string value) {
            replace_str(value, "$frame", std::to_string(_index + 1));
            replace_str(value, "$0frame", std::to_string(_index));
            replace_str(value, "$normalized-value", std::to_string(_value));
            if (instr.parameter == NoParam) return value;
            auto& param = Kaixo::parameter(instr.parameter);
            replace_str(value, "$value", param.toString(_value));
            replace_str(value, "$name", param.name);
            replace_str(value, "$short-name", param.shortName);
            replace_str(value, "$identifier", param.identifier);
            replace_str(value, "$short-identifier", param.shortIdentifier);
            replace_str(value, "$var-name", param.varName);
            replace_str(value, "$full-var-name", param.fullVarName);
            replace_str(value, "$steps", std::to_string(param.steps));
            return value;
        };

        // ------------------------------------------------

        if (color) instr.graphics.setColour(*color);

        // ------------------------------------------------

        Point<float> at = pointFromAlign(_align, instr.position) + _position.toFloat();

        // ------------------------------------------------

        font->draw(instr.graphics, at, format(content->text[_text]), _align, color.hasValue());

        // ------------------------------------------------
        
        return true;

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::BackgroundColorElement::interpret(Theme& self, const basic_json& json) {

        // ------------------------------------------------

        if (json.contains("background-color")) {
            color = ColorElement{ &self };
            color->interpret(json["background-color"]);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    bool DrawableElement::BackgroundColorElement::draw(Theme& self, Drawable::Instruction instr) {
        if (!color) return false;
        instr.graphics.setColour(*color);
        instr.graphics.fillRect(instr.position);
        return true;
    }

    // ------------------------------------------------

    void DrawableElement::Layer::interpret(Theme& self, const basic_json& theme) {

        // ------------------------------------------------

        if (!theme.is(basic_json::Object)) return;

        // ------------------------------------------------

        image.interpret(self, theme);
        text.interpret(self, theme);
        backgroundColor.interpret(self, theme);

        // ------------------------------------------------

    }

    // ------------------------------------------------
            
    bool DrawableElement::Layer::draw(Theme& self, Drawable::Instruction instr) {

        // ------------------------------------------------

        return
            backgroundColor.draw(self, instr) |
            image.draw(self, instr) |
            text.draw(self, instr);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::State::interpret(Theme& self, const basic_json& theme) {

        // ------------------------------------------------

        if (!theme.is(basic_json::Object)) return;

        // ------------------------------------------------

        layers.emplace_back("__base").interpret(self, theme);

        // ------------------------------------------------
                
        if (theme.contains("layers")) {
            theme["layers"].foreach([&](std::string_view key, const basic_json& layer) {
                layers.emplace_back(std::string(key)).interpret(self, layer);
            });
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------
        
    void DrawableElement::interpret(const basic_json& theme) {

        // ------------------------------------------------

        states.clear();

        // ------------------------------------------------

        if (!theme.is(basic_json::Object)) return;

        // ------------------------------------------------

        states.emplace_back(View::State::Default).interpret(*self, theme);

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
                states.emplace_back(state).interpret(*self, theme);
            }
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::draw(Drawable::Instruction instr) {

        // ------------------------------------------------

        std::vector<Layer> toDraw;

        for (auto& state : states) {

            bool matches = state.state & instr.state || state.state == View::State::Default;
            if (!matches) continue;

            for (auto& layer : state.layers) {

                bool layerAlreadyExists = false;
                for (auto& drawLayer : toDraw) {
                    if (drawLayer.identifier == layer.identifier) {
                        drawLayer = layer;
                        layerAlreadyExists = true;
                        break;
                    }
                }

                if (!layerAlreadyExists) {
                    toDraw.emplace_back() = layer;
                }
            }
        }

        // ------------------------------------------------

        for (auto& drawLayer : toDraw) {
            drawLayer.draw(*self, instr);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    DrawableElement::operator Drawable() {
        struct Implementation : public Drawable::Interface {
            Implementation(DrawableElement* self) : self(self) {}

            DrawableElement* self;

            void draw(Drawable::Instruction instr) const override { self->draw(instr); }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

    Drawable DrawableElement::operator[](std::size_t i) {
        struct Implementation : public Drawable::Interface {
            Implementation(DrawableElement* self, std::size_t index) : self(self), index(index) {}

            std::size_t index;
            DrawableElement* self;

            void draw(Drawable::Instruction instr) const override { 
                instr.index = index; // Override index
                self->draw(instr); 
            }
        };

        return { std::make_unique<Implementation>(this, i) };
    }

    // ------------------------------------------------

}
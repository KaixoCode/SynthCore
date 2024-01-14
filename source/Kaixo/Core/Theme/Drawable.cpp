#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    void DrawableElement::ImagePart::interpret(const basic_json& json, View::State state) {

        // ------------------------------------------------

        std::array<std::size_t, 4> edg4{};
        std::array<std::size_t, 2> edg2{};
        std::array<int, 4> arr4{};
        std::array<int, 2> arr2{};
        std::string str;

        // ------------------------------------------------

        bool containsImage = json.contains("image", basic_json::Object);

        // ------------------------------------------------

        image.interpret(json, [&](auto& image, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get(str) || 
                json.try_get("image", str) ||
                containsImage && json["image"].try_get("source", str)) 
            {
                image = self->registerImage(str);
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------

        multiframe.interpret(json, [&](auto& multiframe, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.contains("frames", basic_json::Number)) {
                std::size_t frames = json["frames"].as<std::size_t>();
                std::size_t fprow = 1;
                json.try_get("frames-per-row", fprow);
                multiframe = MultiFrameDescription{ frames, fprow };
                return true;
            } else if (containsImage && json["image"].contains("frames", basic_json::Number)) {
                std::size_t frames = json["image"]["frames"].as<std::size_t>();
                std::size_t fprow = 1;
                json["image"].try_get("frames-per-row", fprow);
                multiframe = MultiFrameDescription{ frames, fprow };
                return true;
            }

            return false;
        }, state);
        
        // ------------------------------------------------

        tiled.interpret(json, [&](auto& tiled, const basic_json& json) {
            if (json.try_get("edges", edg2)) {
                tiled = TiledDescription{ edg2[0], edg2[1], edg2[0], edg2[1] };
                return true;
            } else if (json.try_get("edges", edg4)) {
                tiled = TiledDescription{ edg4[0], edg4[1], edg4[2], edg4[3] };
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------

        position.interpret(json, [&](auto& position, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get("image-position", arr2) || 
                containsImage && json["image"].try_get("position", arr2)) 
            {
                position = Point{ arr2[0], arr2[1] };
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------

        offset.interpret(json, [&](auto& offset, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get("image-clip", arr4) ||
                containsImage && json["image"].try_get("clip", arr4)) 
            {
                offset = Point{ arr4[0], arr4[1] };
                return true;
            } else {
                if (json.try_get("image-offset", arr2) ||
                    containsImage && json["image"].try_get("offset", arr2))
                {
                    offset = Point{ arr2[0], arr2[1] };
                    return true;
                }
            }
            return false;
        }, state);
        
        size.interpret(json, [&](auto& size, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get("image-clip", arr4) ||
                containsImage && json["image"].try_get("clip", arr4)) 
            {
                size = Point{ arr4[2], arr4[3] };
                return true;
            } else {
                if (json.try_get("image-size", arr2) ||
                    containsImage && json["image"].try_get("size", arr2))
                {
                    size = Point{ arr2[0], arr2[1] };
                    return true;
                }
            }
            return false;
        }, state);

        // ------------------------------------------------

        align.interpret(json, [&](Align& align, const basic_json& json) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get("image-align", str) ||
                containsImage && json["image"].try_get("align", str))
            {
                align = alignFromString(str);
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::ImagePart::reset() {
        image = { NoImage };
        offset = { { 0, 0 } };
        size = {};
        position = { { 0, 0 } };
        align = { Align::TopLeft };
        multiframe = {};
        tiled = {};
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::link(ImagePart& part) {
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::draw(Drawable::Instruction instr, Theme& self, ImagePart& part) {

        // ------------------------------------------------
        
        auto& image = part.image[instr.state];
        auto& align = part.align[instr.state];
        auto& position = part.position[instr.state];
        auto& multiframe = part.multiframe[instr.state];
        auto& tiled = part.tiled[instr.state];
        auto& offset = part.offset[instr.state];
        auto& size = part.size[instr.state];

        // ------------------------------------------------

        if (auto img = self.image(image)) {

            // ------------------------------------------------

            if (multiframe) {

                // ------------------------------------------------

                int _width = img->getWidth() / multiframe->framesPerRow;
                int _height = img->getHeight() / Math::ceil(multiframe->numFrames / static_cast<float>(multiframe->framesPerRow));
                auto _size = size ? *size : Point{ _width, _height };
                auto _clip = Rect{ offset.x(), offset.y(), _size.x(), _size.y() };

                // ------------------------------------------------

                auto _index = 0ull;
                if (instr.index != npos) _index = instr.index;
                else if (instr.value != -1) _index = normalToIndex(instr.value, multiframe->numFrames);

                // ------------------------------------------------

                img.draw(FrameInstruction{
                    .graphics = instr.graphics,
                    .description = *multiframe,
                    .tiled = tiled ? &tiled.value() : nullptr,
                    .clip = _clip,
                    .align = align,
                    .frame = _index,
                    .position = position,
                    .bounds = instr.bounds
                });

                // ------------------------------------------------
            
            } else {

                // ------------------------------------------------

                int _width = img->getWidth();
                int _height = img->getHeight();
                auto _size = size ? *size : Point{ _width, _height };
                auto _clip = Rect{ offset.x(), offset.y(), _size.x(), _size.y() };

                // ------------------------------------------------

                if (tiled) {
                    img.draw(TiledInstruction{
                        .graphics = instr.graphics,
                        .description = *tiled,
                        .clip = _clip,
                        .bounds = instr.bounds
                    });
                } else {
                    img.draw(ClippedInstruction{
                        .graphics = instr.graphics,
                        .tiled = nullptr,
                        .clip = _clip,
                        .align = align,
                        .position = position,
                        .bounds = instr.bounds,
                    });
                }

                // ------------------------------------------------

            }
        }
    }

    // ------------------------------------------------

    bool DrawableElement::ImageDrawable::changing() const { return false; };

    // ------------------------------------------------

    void DrawableElement::TextPart::interpret(const basic_json& json, View::State state) {

        // ------------------------------------------------

        bool containsText = json.contains("text", basic_json::Object);

        // ------------------------------------------------
        
        bool contentWasArray = false;
        content.interpret(json, [&](auto& content, const basic_json& json) {
            bool containsText = json.contains("text", basic_json::Object);
            auto parseContent = [&](const basic_json& json) {
                switch (json.type()) {
                case basic_json::Array:
                    contentWasArray = true;
                    content = Content(true);
                    json.foreach([&](const basic_json& text) {
                        if (text.is(basic_json::String)) {
                            content.text.push_back(text.as<basic_json::string>());
                        }
                    });
                    break;
                case basic_json::String:
                    content = Content(false);
                    content.text.push_back(json.as<basic_json::string>());
                    break;
                }
            };

            if (json.contains("text", basic_json::String) || 
                json.contains("text", basic_json::Array)) 
            {
                parseContent(json["text"]);
                return true;
            } else if (containsText && json["text"].contains("content")) {
                parseContent(json["text"]["content"]);
                return true;
            }

            contentWasArray = false;
            return false;
        }, state);
        
        // ------------------------------------------------
        
        std::array<int, 2> arr2{};
        std::string str;
        std::size_t num;

        // ------------------------------------------------

        position.interpret(json, [&](auto& position, const basic_json& json) {
            bool containsText = json.contains("text", basic_json::Object);
            if (json.try_get("text-position", arr2) || 
                containsText && json["text"].try_get("position", arr2))
            {
                position = Point{ arr2[0], arr2[1] };
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------
        
        align.interpret(json, [&](auto& align, const basic_json& json) {
            bool containsText = json.contains("text", basic_json::Object);
            if (json.try_get("text-align", str) ||
                containsText && json["text"].try_get("align", str))
            {
                align = alignFromString(str);
                return true;
            }

            return false;
        }, state);
        
        
        // ------------------------------------------------
        
        frames.interpret(json, [&](auto& frames, const basic_json& json) {
            bool containsText = json.contains("text", basic_json::Object);
            if (json.try_get("frames", str) ||
                containsText && json["text"].try_get("frames", num))
            {
                frames = num;
                return true;
            }

            return false;
        }, state);


        // ------------------------------------------------
        
        if (json.contains("text-color")) {
            color.interpret(json["text-color"], state);
        } else if (containsText && json["text"].contains("color")) {
            color.interpret(json["text"]["color"], state);
        }

        // ------------------------------------------------
                
        if (json.contains("font")) {
            font.interpret(json["font"]);
        } else if (containsText && json["text"].contains("font")) {
            font.interpret(json["text"]["font"]);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::TextDrawable::link(TextPart& part) {
        color = part.color;
        font = part.font;
    }

    // ------------------------------------------------

    void DrawableElement::TextDrawable::draw(Drawable::Instruction instr, Theme& self, TextPart& part) {

        // ------------------------------------------------
        
        auto& content = part.content[instr.state];
        auto& align = part.align[instr.state];
        auto& position = part.position[instr.state];
        auto& frames = part.frames[instr.state];

        // ------------------------------------------------
        
        if (content.text.empty()) return;

        // ------------------------------------------------

        auto _frames = 1ull;
        if (frames) _frames = *frames;
        else if (content.wasArray) _frames = content.text.size();

        // ------------------------------------------------

        auto _index = 0ull;
        if (instr.index != npos) _index = instr.index;
        else if (instr.value != -1) _index = normalToIndex(instr.value, _frames);

        // ------------------------------------------------

        auto _value = _index / static_cast<float>(_frames);
        if (instr.value != -1) _value = instr.value;

        // ------------------------------------------------
                
        auto _text = 0;
        if (content.wasArray) {
            if (instr.index != npos) _text = Math::clamp(instr.index, 0, content.text.size() - 1);
            else _text = normalToIndex(_value, content.text.size());
        }

        // ------------------------------------------------

        auto format = [&](std::string value) {
            replace_str(value, "$frame", std::to_string(_index + 1));
            replace_str(value, "$0frame", std::to_string(_index));
            replace_str(value, "$normalized-value", std::to_string(_value));
            replace_str(value, "$text", instr.text);
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

        instr.graphics.setColour(color.get(instr.state));

        // ------------------------------------------------

        Point<float> at = pointFromAlign(align, instr.bounds) + position.toFloat();

        // ------------------------------------------------

        font.draw(instr.graphics, at, format(content.text[_text]), align, true);

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    void DrawableElement::TextPart::reset() {
        font = { self };
        color = { self };
        content = {};
        position = { { 0, 0 } };
        frames = {};
        align = { Align::Center };
    }

    // ------------------------------------------------

    bool DrawableElement::TextDrawable::changing() const { 
        return color.changing();
    }

    // ------------------------------------------------

    void DrawableElement::BackgroundColorPart::interpret(const basic_json& json, View::State state) {

        // ------------------------------------------------

        if (json.contains("background-color")) {
            color.interpret(json["background-color"], state);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::BackgroundColorPart::reset() {
        color = { self };
    }

    // ------------------------------------------------

    void DrawableElement::BackgroundColorDrawable::link(BackgroundColorPart& part) {
        color = part.color;
    }

    // ------------------------------------------------

    void DrawableElement::BackgroundColorDrawable::draw(Drawable::Instruction instr, Theme& self, BackgroundColorPart& part) {
        instr.graphics.setColour(color.get(instr.state));
        instr.graphics.fillRect(instr.bounds);
    }

    // ------------------------------------------------

    bool DrawableElement::BackgroundColorDrawable::changing() const {
        return color.changing();
    }

    // ------------------------------------------------

    void DrawableElement::Layer::interpret(const basic_json& theme) {

        // ------------------------------------------------

        image.reset();
        text.reset();
        backgroundColor.reset();

        image.interpret(theme);
        text.interpret(theme);
        backgroundColor.interpret(theme);

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
                image.interpret(theme, state);
                text.interpret(theme, state);
                backgroundColor.interpret(theme, state);
            }
        });

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::LayerDrawable::link(Layer& part) {
        identifier = part.identifier;
        backgroundColor.link(part.backgroundColor);
        image.link(part.image);
        text.link(part.text);
    }

    // ------------------------------------------------
            
    void DrawableElement::LayerDrawable::draw(Drawable::Instruction instr, Theme& self, Layer& part) {

        // ------------------------------------------------

        backgroundColor.draw(instr, self, part.backgroundColor);
        image.draw(instr, self, part.image);
        text.draw(instr, self, part.text);

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    bool DrawableElement::LayerDrawable::changing() const {
        return backgroundColor.changing()
            || image.changing()
            || text.changing();
    }

    // ------------------------------------------------

    void DrawableElement::interpret(const basic_json& theme) {

        // ------------------------------------------------

        layers.clear();
        auto& l = layers.emplace_back(self); 
        l.identifier = "__base";
        l.interpret(theme);

        // ------------------------------------------------
                
        if (theme.contains("layers")) {
            theme["layers"].foreach([&](std::string_view key, const basic_json& layer) {
                auto& l = layers.emplace_back(self);
                l.identifier = key;
                l.interpret(layer);
            });
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    struct Implementation : public Drawable::Interface {
        Implementation(DrawableElement* self) 
            : self(self) 
        {}

        DrawableElement* self;
        std::vector<DrawableElement::LayerDrawable> layers{};

        void resync() {
            bool changed = layers.size() != self->layers.size();

            if (!changed) {
                for (std::size_t i = 0; i < self->layers.size(); ++i) {
                    if (self->layers[i].identifier != layers[i].identifier) {
                        changed = true;
                        break;
                    }
                }
            }

            if (changed) {
                layers.clear();

                for (auto& layer : self->layers) {
                    layers.emplace_back().link(layer);
                }
            }
        }

        void draw(Drawable::Instruction instr) override {
            resync();
            for (std::size_t i = 0; i < self->layers.size(); ++i) {
                layers[i].draw(instr, *self->self, self->layers[i]);
            }
        }

        bool changing() const override { 
            for (auto& layer : layers) {
                if (layer.changing()) return true;
            }
            return false;
        }
    };

    DrawableElement::operator Drawable() {
        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

    struct Implementation2 : public Implementation {
        Implementation2(DrawableElement* self, std::size_t index) 
            : Implementation(self), index(index) 
        {}

        std::size_t index;

        void draw(Drawable::Instruction instr) override {
            instr.index = index; // Override index
            Implementation::draw(instr);
        }
    };

    Drawable DrawableElement::operator[](std::size_t i) {
        return { std::make_unique<Implementation2>(this, i) };
    }

    // ------------------------------------------------

}
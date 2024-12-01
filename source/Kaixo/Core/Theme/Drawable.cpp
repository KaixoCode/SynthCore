
// ------------------------------------------------

#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    void DrawableElement::RectPart::reset() {
        fill = { self };
        stroke = { self };
        strokeWeight.reset();
        position.reset();
        align = {};
    }

    void DrawableElement::RectPart::interpret(const basic_json& theme, View::State state) {
        const auto parseAlign = [&](auto& align, const basic_json& json, View::State state) {
            std::string str;
            if (json.try_get(str)) return align = alignFromString(str), true;
            return false;
        };

        if (theme.contains("rect", basic_json::Object)) {
            auto& rect = theme["rect"];
            if (rect.contains("align")) align.interpret(rect["align"], parseAlign, state);
            if (rect.contains("fill")) fill.interpret(rect["fill"], state);
            if (rect.contains("stroke")) stroke.interpret(rect["stroke"], state);
            if (rect.contains("stroke-weight")) strokeWeight.interpret(rect["stroke-weight"], state);
            if (rect.contains("dimensions")) position.interpret(rect["dimensions"], state);
            if (rect.contains("position")) position.interpretPosition(rect["position"], state);
            if (rect.contains("size")) position.interpretSize(rect["size"], state);
            if (rect.contains("x")) position.interpretX(rect["x"], state);
            if (rect.contains("y")) position.interpretY(rect["y"], state);
            if (rect.contains("width")) position.interpretWidth(rect["width"], state);
            if (rect.contains("height")) position.interpretHeight(rect["height"], state);
        }

        if (theme.contains("rect-align")) align.interpret(theme["rect-align"], parseAlign, state);
        if (theme.contains("rect-fill")) fill.interpret(theme["rect-fill"], state);
        if (theme.contains("rect-stroke")) stroke.interpret(theme["rect-stroke"], state);
        if (theme.contains("rect-stroke-weight")) strokeWeight.interpret(theme["rect-stroke-weight"], state);
        if (theme.contains("rect-dimensions")) position.interpret(theme["rect-dimensions"], state);
        if (theme.contains("rect-position")) position.interpretPosition(theme["rect-position"], state);
        if (theme.contains("rect-size")) position.interpretSize(theme["rect-size"], state);
        if (theme.contains("rect-x")) position.interpretX(theme["rect-x"], state);
        if (theme.contains("rect-y")) position.interpretY(theme["rect-y"], state);
        if (theme.contains("rect-width")) position.interpretWidth(theme["rect-width"], state);
        if (theme.contains("rect-height")) position.interpretHeight(theme["rect-height"], state);
    }

    // ------------------------------------------------

    // ------------------------------------------------

    void DrawableElement::RectDrawable::link(RectPart& part) {
        fill = part.fill;
        stroke = part.stroke;
        position = part.position;
        strokeWeight = part.strokeWeight;
    }

    void DrawableElement::RectDrawable::draw(const Drawable::Instruction& instr, Theme& self, RectPart& part) {
        auto& align = part.align[instr.state];
        auto weight = strokeWeight.get(instr.state, instr.values);
        auto strokeColor = stroke.get(instr.state, instr.values);
        auto fillColor = fill.get(instr.state, instr.values);
        auto pos = position.get(instr.state, instr.values);

        switch (align & Align::X) {
        case Align::CenterX: pos.x(pos.x() + instr.bounds.centerX() - pos.width() / 2); break;
        case Align::Right: pos.x(pos.x() + instr.bounds.right() - pos.width()); break;
        default: pos.x(pos.x() + instr.bounds.left()); break;
        }

        switch (align & Align::Y) {
        case Align::CenterY: pos.y(pos.y() + instr.bounds.centerY() - pos.height() / 2); break;
        case Align::Bottom: pos.y(pos.y() + instr.bounds.bottom() - pos.height()); break;
        default: pos.y(pos.y() + instr.bounds.top()); break;
        }

        if (part.fill.hasValue()) {
            instr.graphics.setColour(fillColor);
            instr.graphics.fillRect(pos);
        }

        if (weight != 0 && part.stroke.hasValue()) {
            instr.graphics.setColour(strokeColor);
            instr.graphics.drawRect(pos, weight);
        }
    }

    bool DrawableElement::RectDrawable::changing() const {
        return fill.changing() 
            || position.changing() 
            || stroke.changing() 
            || strokeWeight.changing();
    }

    // ------------------------------------------------

    void DrawableElement::ImagePart::interpret(const basic_json& json, View::State state) {

        // ------------------------------------------------

        const auto parseTiled = [&](auto& tiled, const basic_json& json, View::State state) {
            std::array<std::size_t, 4> edg4{};
            std::array<std::size_t, 2> edg2{};
            if (json.try_get(edg4)) return tiled = TiledDescription{ edg4[0], edg4[1], edg4[2], edg4[3] }, true;
            if (json.try_get(edg2)) return tiled = TiledDescription{ edg2[0], edg2[1], edg2[0], edg2[1] }, true;
            return false;
        };

        const auto parseImage = [&](auto& image, const basic_json& json, View::State state) {
            std::string str;
            if (json.try_get(str)) return image = self->registerImage(str), true;
            return false;
        };

        const auto parseMultiFrame = [&](auto& multiframe, const basic_json& json, View::State state) {
            if (json.contains("frames", basic_json::Number)) {
                std::size_t frames = json["frames"].as<std::size_t>();
                std::size_t fprow = 1;
                std::size_t repeat = 1;
                json.try_get("frames-per-row", fprow);
                json.try_get("frames-repeat", repeat);
                multiframe = MultiFrameDescription{ frames, fprow, repeat };
                return true;
            }

            return false;
        };

        const auto parseAlign = [&](auto& align, const basic_json& json, View::State state) {
            std::string str;
            if (json.try_get(str)) return align = alignFromString(str), true;
            return false;
        };

        // ------------------------------------------------

        if (json.contains("image", basic_json::Object)) {
            auto& img = json["image"];
            if (img.contains("clip")) clip.interpret(img["clip"], state);
            if (img.contains("offset")) clip.interpretPosition(img["offset"], state);
            if (img.contains("offset-x")) clip.interpretX(img["offset-x"], state);
            if (img.contains("offset-y")) clip.interpretY(img["offset-y"], state);
            if (img.contains("size")) clip.interpretSize(img["size"], state);
            if (img.contains("width")) clip.interpretWidth(img["width"], state);
            if (img.contains("height")) clip.interpretHeight(img["height"], state);
            if (img.contains("position")) position.interpret(img["position"], state);
            if (img.contains("position-x")) position.interpretX(img["position-x"], state);
            if (img.contains("position-y")) position.interpretY(img["position-y"], state);
            if (img.contains("position-width")) position.interpretWidth(img["position-width"], state);
            if (img.contains("position-height")) position.interpretHeight(img["position-height"], state);
            if (img.contains("edges")) tiled.interpret(img["edges"], parseTiled, state);
            if (img.contains("align")) align.interpret(img["align"], parseAlign, state);
            if (img.contains("source")) image.interpret(img["source"], parseImage, state);
            multiframe.interpret(img, parseMultiFrame, state);
        }

        if (json.contains("image-clip")) clip.interpret(json["image-clip"], state);
        if (json.contains("image-offset")) clip.interpretPosition(json["image-offset"], state);
        if (json.contains("image-offset-x")) clip.interpretX(json["image-offset-x"], state);
        if (json.contains("image-offset-y")) clip.interpretY(json["image-offset-y"], state);
        if (json.contains("image-size")) clip.interpretSize(json["image-size"], state);
        if (json.contains("image-width")) clip.interpretWidth(json["image-width"], state);
        if (json.contains("image-height")) clip.interpretHeight(json["image-height"], state);
        if (json.contains("image-position")) position.interpret(json["image-position"], state);
        if (json.contains("image-position-x")) position.interpretX(json["image-position-x"], state);
        if (json.contains("image-position-y")) position.interpretY(json["image-position-y"], state);
        if (json.contains("image-position-width")) position.interpretWidth(json["image-position-width"], state);
        if (json.contains("image-position-height")) position.interpretHeight(json["image-position-height"], state);
        if (json.contains("image-edges")) tiled.interpret(json["image-edges"], parseTiled, state);
        if (json.contains("image-align")) align.interpret(json["image-align"], parseAlign, state);
        if (json.contains("image")) image.interpret(json["image"], parseImage, state);
        multiframe.interpret(json, parseMultiFrame, state);
        image.interpret(json, parseImage, state);

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::ImagePart::reset() {
        clip.reset();
        position.reset();
        image = { NoImage };
        align = { Align::TopLeft };
        multiframe = {};
        tiled = {};
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::link(ImagePart& part) {
        clip = part.clip;
        position = part.position;
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::draw(const Drawable::Instruction& instr, Theme& self, ImagePart& part) {

        // ------------------------------------------------
        
        auto& image = part.image[instr.state];
        auto& align = part.align[instr.state];
        auto dimensions = clip.get(instr.state, instr.values);
        auto pos = position.get(instr.state, instr.values);
        auto& multiframe = part.multiframe[instr.state];
        auto& tiled = part.tiled[instr.state];

        // ------------------------------------------------

        if (auto img = self.image(image)) {

            // ------------------------------------------------

            if (multiframe) {

                // ------------------------------------------------

                int _width = img->getWidth() / multiframe->framesPerRow;
                int _height = img->getHeight() / Math::ceil(multiframe->numFrames / static_cast<float>(multiframe->framesPerRow));
                auto _clip = Rect{ 
                    part.clip.hasX(instr.state) ? dimensions.x() : 0, 
                    part.clip.hasY(instr.state) ? dimensions.y() : 0, 
                    part.clip.hasWidth(instr.state) ? dimensions.width() : _width,
                    part.clip.hasHeight(instr.state) ? dimensions.height() : _height
                };

                // ------------------------------------------------

                auto _index = 0ull;
                if (instr.index != npos) _index = instr.index;
                else if (instr.value != -1) _index = normalToIndex(instr.value, multiframe->numFrames * multiframe->repeat) % multiframe->numFrames;

                // ------------------------------------------------

                img.draw(FrameInstruction{
                    .graphics = instr.graphics,
                    .description = *multiframe,
                    .tiled = tiled ? &tiled.value() : nullptr,
                    .clip = _clip,
                    .align = align,
                    .frame = _index,
                    .position = {
                        part.position.hasX(instr.state) ? pos.x() : 0,
                        part.position.hasY(instr.state) ? pos.y() : 0,
                        part.position.hasWidth(instr.state) ? pos.width() : _clip.width(),
                        part.position.hasHeight(instr.state) ? pos.height() : _clip.height()
                    },
                    .bounds = instr.bounds
                });

                // ------------------------------------------------
            
            } else {

                // ------------------------------------------------

                int _width = img->getWidth();
                int _height = img->getHeight();
                auto _clip = Rect{
                    part.clip.hasX(instr.state) ? dimensions.x() : 0,
                    part.clip.hasY(instr.state) ? dimensions.y() : 0,
                    part.clip.hasWidth(instr.state) ? dimensions.width() : _width,
                    part.clip.hasHeight(instr.state) ? dimensions.height() : _height
                };

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
                        .position = {
                            part.position.hasX(instr.state) ? pos.x() : 0,
                            part.position.hasY(instr.state) ? pos.y() : 0,
                            part.position.hasWidth(instr.state) ? pos.width() : _clip.width(),
                            part.position.hasHeight(instr.state) ? pos.height() : _clip.height()
                        },
                        .bounds = instr.bounds,
                    });
                }

                // ------------------------------------------------

            }
        }
    }

    // ------------------------------------------------

    bool DrawableElement::ImageDrawable::changing() const { 
        return clip.changing() || position.changing();
    }

    // ------------------------------------------------

    void DrawableElement::TextPart::interpret(const basic_json& json, View::State state) {

        // ------------------------------------------------

        bool containsText = json.contains("text", basic_json::Object);

        // ------------------------------------------------
        
        bool contentWasArray = false;
        content.interpret(json, [&](auto& content, const basic_json& json, View::State myState) {
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

        overflow == Overflow::Visible;
        if (json.try_get("text-overflow", str) || 
            containsText && json["text"].try_get("overflow", str)) 
        {
            if (str == "visible") overflow = Overflow::Visible;
            else if (str == "dots") overflow = Overflow::Dots;
        }
        
        roundMode == RoundMode::Trunc;
        if (json.try_get("text-round-position", str) || 
            containsText && json["text"].try_get("round-position", str)) 
        {
            if (str == "floor") roundMode = RoundMode::Floor;
            else if (str == "ceil") roundMode = RoundMode::Ceil;
            else if (str == "round") roundMode = RoundMode::Round;
            else if (str == "trunc") roundMode = RoundMode::Trunc;
            else if (str == "none") roundMode = RoundMode::None;
        }

        // ------------------------------------------------

        if (json.contains("text", basic_json::Object)) {
            auto& text = json["text"];
            if (text.contains("position")) position.interpret(text["position"], state);
        }

        if (json.contains("text-position")) position.interpret(json["text-position"], state);
        if (json.contains("text-position-x")) position.interpretX(json["text-position-x"], state);
        if (json.contains("text-position-y")) position.interpretY(json["text-position-y"], state);

        // ------------------------------------------------
        
        align.interpret(json, [&](auto& align, const basic_json& json, View::State state) {
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
        
        frames.interpret(json, [&](auto& frames, const basic_json& json, View::State state) {
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
        position = part.position;
    }

    // ------------------------------------------------

    void DrawableElement::TextDrawable::draw(const Drawable::Instruction& instr, Theme& self, TextPart& part) {

        // ------------------------------------------------
        
        auto& content = part.content[instr.state];
        auto& align = part.align[instr.state];
        auto pos = position.get(instr.state, instr.values);
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
            for (auto& [var, replace] : instr.text) {
                replace_str(value, var, replace);
            }
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

        Kaixo::Point<float> at = pointFromAlign(align, instr.bounds) + pos;

        switch (part.roundMode) {
        case TextPart::RoundMode::Ceil: at = { Math::ceil(at.x()), Math::ceil(at.y()) }; break;
        case TextPart::RoundMode::Floor: at = { Math::floor(at.x()), Math::floor(at.y()) }; break;
        case TextPart::RoundMode::Round: at = { Math::round(at.x()), Math::round(at.y()) }; break;
        case TextPart::RoundMode::Trunc: at = { Math::trunc(at.x()), Math::trunc(at.y()) }; break;
        case TextPart::RoundMode::None: break;
        }

        // ------------------------------------------------

        auto string = format(content.text[_text]);
        if (part.overflow == TextPart::Overflow::Dots) {
            string = part.font.fitWithinWidth(string, instr.bounds.x() + instr.bounds.width() - at.x());
            font.draw(instr.graphics, at, string, align, true);
        } else {
            font.draw(instr.graphics, at, string, align, true);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    void DrawableElement::TextPart::reset() {
        font = { self };
        color = { self };
        content = {};
        position = { self };
        frames = {};
        align = { Align::Center };
        overflow = Overflow::Visible;
    }

    // ------------------------------------------------

    bool DrawableElement::TextDrawable::changing() const { 
        return color.changing() || position.changing();
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

    void DrawableElement::BackgroundColorDrawable::draw(const Drawable::Instruction& instr, Theme& self, BackgroundColorPart& part) {
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

        conditional = {};
        if (theme.contains("if", basic_json::String)) {
            conditional = ExpressionParser::parse(theme["if"].as<std::string_view>());
        }

        linked = {};
        if (theme.contains("link", basic_json::String)) {
            linked = theme["link"].as<std::string>();
        }

        image.reset();
        text.reset();
        backgroundColor.reset();
        rect.reset();

        image.interpret(theme);
        text.interpret(theme);
        backgroundColor.interpret(theme);
        rect.interpret(theme);

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
                rect.interpret(theme, state);
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
        rect.link(part.rect);
    }

    // ------------------------------------------------
            
    void DrawableElement::LayerDrawable::draw(const Drawable::Instruction& instr, Theme& self, Layer& part) {

        // ------------------------------------------------

        backgroundColor.draw(instr, self, part.backgroundColor);
        image.draw(instr, self, part.image);
        text.draw(instr, self, part.text);
        rect.draw(instr, self, part.rect);

        // ------------------------------------------------

    }

    // ------------------------------------------------
    
    bool DrawableElement::LayerDrawable::changing() const {
        return backgroundColor.changing()
            || image.changing()
            || rect.changing()
            || text.changing();
    }

    // ------------------------------------------------

    void DrawableElement::interpret(const basic_json& theme) {

        // ------------------------------------------------

        loadIndex++;
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

        std::size_t loadIndex = npos;
        DrawableElement* self;
        std::vector<DrawableElement::LayerDrawable> layers{};

        std::unique_ptr<Interface> copy() const override {
            return std::make_unique<Implementation>(self);
        }

        void resync() {
            if (loadIndex != self->loadIndex) {
                loadIndex = self->loadIndex;
                layers.clear();

                for (auto& layer : self->layers) {
                    layers.emplace_back().link(layer);
                }
            }
        }

        void draw(Drawable::Instruction instr) override {
            resync();
            float value = instr.value;
            std::size_t index = instr.index;
            for (std::size_t i = 0; i < self->layers.size(); ++i) {
                if (self->layers[i].conditional && !self->layers[i].conditional(instr.values)) continue;

                instr.value = value;
                instr.index = index;
                if (self->layers[i].linked) {
                    std::string_view variable = self->layers[i].linked.value();
                    auto it = instr.values.find(variable);
                    if (it != instr.values.end()) {
                        instr.value = it->second;

                        // Clearing the index is a decision because the index takes
                        // precedence over the value, and if this drawable was
                        // indexed explicitly this would disallow other layers
                        // that are linked explicitly to function.
                        instr.index = npos;
                    }
                }

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
            Implementation::draw(std::move(instr));
        }
    };

    Drawable DrawableElement::operator[](std::size_t i) {
        return { std::make_unique<Implementation2>(this, i) };
    }

    // ------------------------------------------------

}
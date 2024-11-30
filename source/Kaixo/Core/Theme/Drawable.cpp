
// ------------------------------------------------

#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    void DrawableElement::RectPart::reset() {
        x = {};
        y = {};
        w = {};
        h = {};
    }

    void DrawableElement::RectPart::interpret(const basic_json& theme, View::State state) {
        const auto parser = [&](auto& value, const basic_json& json, View::State state) -> bool {
            if (json.is(basic_json::Number)) return value = [value = json.as<int>()](auto&) { return value; }, true;
            if (json.is(basic_json::String)) return value = ExpressionParser::parse(json.as<std::string_view>()), true;
            return false;
        };

        std::size_t index = 0;
        const auto parserIdx = [&](auto& value, const basic_json& json, View::State state) -> bool {
            if (!json.is(basic_json::Array) || index >= json.size()) return false;
            if (json[index].is(basic_json::Number)) return value = [value = json[index].as<int>()](auto&) { return value; }, true;
            if (json[index].is(basic_json::String)) return value = ExpressionParser::parse(json[index].as<std::string_view>()), true;
            return false;
        };

        if (theme.contains("rect", basic_json::Object)) {
            auto& rect = theme["rect"];
            if (rect.contains("dimensions")) {
                index = 0, x.interpret(rect["dimensions"], parserIdx, state);
                index = 1, y.interpret(rect["dimensions"], parserIdx, state);
                index = 2, w.interpret(rect["dimensions"], parserIdx, state);
                index = 3, h.interpret(rect["dimensions"], parserIdx, state);
            }

            if (rect.contains("position")) {
                index = 0, x.interpret(rect["position"], parserIdx, state);
                index = 1, y.interpret(rect["position"], parserIdx, state);
            }

            if (rect.contains("size")) {
                index = 0, w.interpret(rect["size"], parserIdx, state);
                index = 1, h.interpret(rect["size"], parserIdx, state);
            }

            if (rect.contains("x")) x.interpret(rect["x"], parser, state);
            if (rect.contains("y")) y.interpret(rect["y"], parser, state);
            if (rect.contains("width")) w.interpret(rect["width"], parser, state);
            if (rect.contains("height")) h.interpret(rect["height"], parser, state);

            if (rect.contains("fill")) fill.interpret(rect["fill"], state);
        }

        if (theme.contains("rect-dimensions")) {
            index = 0, x.interpret(theme["rect-dimensions"], parserIdx, state);
            index = 1, y.interpret(theme["rect-dimensions"], parserIdx, state);
            index = 2, w.interpret(theme["rect-dimensions"], parserIdx, state);
            index = 3, h.interpret(theme["rect-dimensions"], parserIdx, state);
        }
        
        if (theme.contains("rect-position")) {
            index = 0, x.interpret(theme["rect-position"], parserIdx, state);
            index = 1, y.interpret(theme["rect-position"], parserIdx, state);
        }
        
        if (theme.contains("rect-size")) {
            index = 0, w.interpret(theme["rect-size"], parserIdx, state);
            index = 1, h.interpret(theme["rect-size"], parserIdx, state);
        }
        
        if (theme.contains("rect-x")) x.interpret(theme["rect-x"], parser, state);
        if (theme.contains("rect-y")) y.interpret(theme["rect-y"], parser, state);
        if (theme.contains("rect-width")) w.interpret(theme["rect-width"], parser, state);
        if (theme.contains("rect-height")) h.interpret(theme["rect-height"], parser, state);

        if (theme.contains("rect-fill")) fill.interpret(theme["rect-fill"], state);
    }

    // ------------------------------------------------

    // ------------------------------------------------

    void DrawableElement::RectDrawable::link(RectPart& part) {
        fill = part.fill;
    }

    void DrawableElement::RectDrawable::draw(const Drawable::Instruction& instr, Theme& self, RectPart& part) {
        auto xe = part.x[instr.state];
        auto ye = part.y[instr.state];
        auto we = part.w[instr.state];
        auto he = part.h[instr.state];
        if (xe.value) x = { xe.value(instr.values), xe.transition };
        if (ye.value) y = { ye.value(instr.values), ye.transition };
        if (we.value) w = { we.value(instr.values), we.transition };
        if (he.value) h = { he.value(instr.values), he.transition };

        changingCache = x.changing() || y.changing() || w.changing() || h.changing();

        auto color = fill.get(instr.state, instr.values);

        Rect<float> position{
            instr.bounds.x() + x.get(),
            instr.bounds.y() + y.get(),
            w.get(), h.get(),
        };

        instr.graphics.setColour(color);
        instr.graphics.fillRect(position);
    }

    bool DrawableElement::RectDrawable::changing() const {
        return changingCache;
    }

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

        image.interpret(json, [&](auto& image, const basic_json& json, View::State state) {
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

        multiframe.interpret(json, [&](auto& multiframe, const basic_json& json, View::State state) {
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

        tiled.interpret(json, [&](auto& tiled, const basic_json& json, View::State state) {
            bool containsImage = json.contains("image", basic_json::Object);
            if (json.try_get("image-edges", edg4) || containsImage && json["image"].try_get("edges", edg4)) {
                tiled = TiledDescription{ edg4[0], edg4[1], edg4[2], edg4[3] };
                return true;
            } else if (json.try_get("image-edges", edg2) || containsImage && json["image"].try_get("edges", edg2)) {
                tiled = TiledDescription{ edg2[0], edg2[1], edg2[0], edg2[1] };
                return true;
            }

            return false;
        }, state);

        // ------------------------------------------------

        bool success = false;
        const auto parseArr24F2 = [&](auto& val, const basic_json& json, View::State state) {
            if (json.try_get(arr4)) return success = true, val = Point{ arr4[0], arr4[1] }, true;
            if (json.try_get(arr2)) return success = true, val = Point{ arr2[0], arr2[1] }, true;
            return false;
        };

        const auto parseArr2 = [&](auto& val, const basic_json& json, View::State state) {
            if (json.try_get(arr2)) return success = true, val = Point{ arr2[0], arr2[1] }, true;
            return false;
        };

        const auto parseArr4 = [&](auto& val, const basic_json& json, View::State state) {
            if (json.try_get(arr4)) return success = true, val = Rect{ arr4[0], arr4[1], arr4[2], arr4[3] }, true;
            return false;
        };

        const auto parseArr4F2 = [&](auto& val, const basic_json& json, View::State state) {
            if (json.try_get(arr4)) return success = true, val = Point{ arr4[0], arr4[1] }, true;
            return false;
        };

        const auto parseArr4B2 = [&](auto& val, const basic_json& json, View::State state) {
            if (json.try_get(arr4)) return success = true, val = Point{ arr4[2], arr4[3] }, true;
            return false;
        };
        
        const auto parseAlign = [&](auto& align, const basic_json& json, View::State state) {
            if (json.try_get(str)) return success = true, align = alignFromString(str), true;
            return false;
        };

        // ------------------------------------------------

        if (json.contains("image", basic_json::Object)) {
            auto& image = json["image"];

            if (image.contains("position")) {
                positionOffset.interpret(image["position"], parseArr24F2, state);
                success = false;
                positionSize.interpret(image["position"], parseArr4B2, state);
                if (success) hasPositionSize = true;
            }

            if (image.contains("clip")) {
                offset.interpret(image["clip"], parseArr4F2, state);
                size.interpret(image["clip"], parseArr4B2, state);
                hasSize = true;
            }

            if (image.contains("offset")) {
                offset.interpret(image["offset"], parseArr2, state);
            }

            if (image.contains("size")) {
                size.interpret(image["size"], parseArr2, state);
                hasSize = true;
            }

            if (image.contains("align")) {
                align.interpret(image["align"], parseAlign, state);
            }
        }

        if (json.contains("image-position")) {
            positionOffset.interpret(json["image-position"], parseArr24F2, state);
            success = false;
            positionSize.interpret(json["image-position"], parseArr4B2, state);
            if (success) hasPositionSize = true;
        }

        if (json.contains("image-clip")) {
            offset.interpret(json["image-clip"], parseArr4F2, state);
            size.interpret(json["image-clip"], parseArr4B2, state);
            hasSize = true;
        }

        if (json.contains("image-offset")) {
            offset.interpret(json["image-offset"], parseArr2, state);
        }

        if (json.contains("image-size")) {
            size.interpret(json["image-size"], parseArr2, state);
            hasSize = true;
        }
        
        if (json.contains("image-align")) {
            align.interpret(json["image-align"], parseAlign, state);
        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

    void DrawableElement::ImagePart::reset() {
        hasSize = false;
        hasPositionSize = false;
        image = { NoImage };
        offset = { { 0, 0 } };
        size = { { 0, 0 } };
        positionOffset = { { 0, 0 } };
        positionSize = { { 0, 0 } };
        align = { Align::TopLeft };
        multiframe = {};
        tiled = {};
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::link(ImagePart& part) {
    }

    // ------------------------------------------------

    void DrawableElement::ImageDrawable::draw(const Drawable::Instruction& instr, Theme& self, ImagePart& part) {

        // ------------------------------------------------

        if (state != instr.state) {
            state = instr.state;
            positionOffsetValue = part.positionOffset[instr.state];
            positionSizeValue = part.positionSize[instr.state];
            offsetValue = part.offset[instr.state];
            sizeValue = part.size[instr.state];
        }

        changingCache = positionOffsetValue.changing()
                     || positionSizeValue.changing()
                     || sizeValue.changing()
                     || offsetValue.changing();

        // ------------------------------------------------
        
        auto& image = part.image[instr.state];
        auto& align = part.align[instr.state];
        auto posOffset = positionOffsetValue.get();
        auto posSize = positionSizeValue.get();
        auto& multiframe = part.multiframe[instr.state];
        auto& tiled = part.tiled[instr.state];
        auto offset = offsetValue.get();
        auto size = sizeValue.get();

        // ------------------------------------------------

        if (auto img = self.image(image)) {

            // ------------------------------------------------

            if (multiframe) {

                // ------------------------------------------------

                int _width = img->getWidth() / multiframe->framesPerRow;
                int _height = img->getHeight() / Math::ceil(multiframe->numFrames / static_cast<float>(multiframe->framesPerRow));
                auto _size = part.hasSize ? size : Point{ _width, _height };
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
                    .position = { 
                        posOffset.x(),
                        posOffset.y(),
                        part.hasPositionSize ? posSize.x() : _clip.width(),
                        part.hasPositionSize ? posSize.y() : _clip.height(),
                    },
                    .bounds = instr.bounds
                });

                // ------------------------------------------------
            
            } else {

                // ------------------------------------------------

                int _width = img->getWidth();
                int _height = img->getHeight();
                auto _size = part.hasSize ? size : Point{ _width, _height };
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
                        .position = {
                            posOffset.x(),
                            posOffset.y(),
                            part.hasPositionSize ? posSize.x() : _clip.width(),
                            part.hasPositionSize ? posSize.y() : _clip.height(),
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
        return changingCache;
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

        position.interpret(json, [&](auto& position, const basic_json& json, View::State state) {
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
    }

    // ------------------------------------------------

    void DrawableElement::TextDrawable::draw(const Drawable::Instruction& instr, Theme& self, TextPart& part) {

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

        Point<float> at = pointFromAlign(align, instr.bounds) + position.toFloat();

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
        position = { { 0, 0 } };
        frames = {};
        align = { Align::Center };
        overflow = Overflow::Visible;
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
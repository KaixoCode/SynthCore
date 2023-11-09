#include "Kaixo/Core/Theme/FontMap.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Font::Font(std::unique_ptr<Interface> graphics)
        : m_Graphics(std::move(graphics))
    {}

    // ------------------------------------------------

    float Font::fontSize() const {
        if (m_Graphics) return m_Graphics->fontSize();
        return 0;
    }
    
    // ------------------------------------------------

    float Font::stringWidth(std::string_view str) const {
        if (m_Graphics) return m_Graphics->stringWidth(str);
        return 0;
    }

    float Font::charWidth(char c, char before, char after) const {
        if (m_Graphics) return m_Graphics->charWidth(c, before, after);
        return 0;
    }

    // ------------------------------------------------

    void Font::draw(juce::Graphics& g, const Point<float>& pos, std::string_view str, Align align) const {
        if (m_Graphics) m_Graphics->draw(g, pos, str, align);
    }

    // ------------------------------------------------

    std::int64_t FontElement::Letter::calcPreSpacing(char before, char after) const {
        for (auto& e : exceptions) {
            if (e.after.contains(before) && e.preSpacing != NoSpacing) {
                return e.preSpacing;
            }

            if (e.before.contains(after) && e.preSpacing != NoSpacing) {
                return e.preSpacing;
            }
        }

        return preSpacing;
    }

    // ------------------------------------------------

    std::int64_t FontElement::Letter::calcPostSpacing(char before, char after) const {
        for (auto& e : exceptions) {
            if (e.after.contains(before) && e.postSpacing != NoSpacing) {
                return e.postSpacing;
            }

            if (e.before.contains(after) && e.postSpacing != NoSpacing) {
                return e.postSpacing;
            }
        }

        return postSpacing;
    }

    // ------------------------------------------------

    std::optional<json> getFontDescription(Theme* self, const std::string& description) {
        std::filesystem::path path = description;
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            auto abspath = std::filesystem::absolute(path);

            std::ifstream file{ abspath };
            if (!file.is_open()) return nullptr;
            
            return json::parse(file_to_string(file));
        } else if (self->hasVariable(description)) {
            return self->variable(description);
        } else {
            return nullptr;
        }
    }

    // ------------------------------------------------

    void FontElement::interpret(const json& theme) {
        maxHeight = 0;
        auto loadZoomLevel = [&](ZoomMultiplier zoom, const json& theme) {
            zoomLevel[zoom].charMap.clear();

            if (theme.contains("map", json::String)) { // Load font-map
                zoomLevel[zoom].id = self->registerImage(theme["map"].as<json::string>());
            }

            json json;
            std::string description;
            if (theme.contains("description", json::String)) { // Load description from file
                description = theme["description"].as<json::string>();
                auto optional_json = getFontDescription(self, description);
                if (!optional_json) return;
                json = optional_json.value();
            } else if (theme.contains("description", json::Object)) { // Load description from json
                json = theme["description"];
            }

            if (json.is(json::Object)) {
                for (auto& [key, val] : json.as<json::object>()) {
                    if (key.size() != 1) continue;

                    Rect<int> clip{ 0, 0, 0, 0 };
                    if (val.contains("location", json::Array) && val["location"].size() == 4 &&
                        val["location"][0].is(json::Unsigned) && val["location"][1].is(json::Unsigned) &&
                        val["location"][2].is(json::Unsigned) && val["location"][3].is(json::Unsigned))
                    {
                        clip = {
                            static_cast<int>(val["location"][0].as<json::unsigned_integral>()),
                            static_cast<int>(val["location"][1].as<json::unsigned_integral>()),
                            static_cast<int>(val["location"][2].as<json::unsigned_integral>()),
                            static_cast<int>(val["location"][3].as<json::unsigned_integral>()),
                        };
                    }

                    auto& mapped = zoomLevel[zoom].charMap[key[0]] = Letter{ .clip = clip, };

                    if (maxHeight < clip.height() / zoom) {
                        maxHeight = clip.height() / zoom;
                    }

                    if (val.contains("pre-spacing")) {
                        if (val["pre-spacing"].is(json::Integral)) mapped.preSpacing = val["pre-spacing"].as<json::integral>();
                        if (val["pre-spacing"].is(json::Unsigned)) mapped.preSpacing = val["pre-spacing"].as<json::unsigned_integral>();
                    }

                    if (val.contains("post-spacing")) {
                        if (val["post-spacing"].is(json::Integral)) mapped.postSpacing = val["post-spacing"].as<json::integral>();
                        if (val["post-spacing"].is(json::Unsigned)) mapped.postSpacing = val["post-spacing"].as<json::unsigned_integral>();
                    }

                    if (val.contains("exceptions", json::Array)) {
                        auto& arr = val["exceptions"].as<json::array>();

                        for (auto& e : arr) {
                            if (!e.is(json::Object)) continue;

                            auto& exception = mapped.exceptions.emplace_back();

                            if (e.contains("pre-spacing")) {
                                if (e["pre-spacing"].is(json::Integral)) exception.preSpacing = e["pre-spacing"].as<json::integral>();
                                if (e["pre-spacing"].is(json::Unsigned)) exception.preSpacing = e["pre-spacing"].as<json::unsigned_integral>();
                            }

                            if (e.contains("post-spacing")) {
                                if (e["post-spacing"].is(json::Integral)) exception.postSpacing = e["post-spacing"].as<json::integral>();
                                if (e["post-spacing"].is(json::Unsigned)) exception.postSpacing = e["post-spacing"].as<json::unsigned_integral>();
                            }

                            if (e.contains("after", json::Array)) {
                                auto& arr = e["after"].as<json::array>();

                                for (auto& c : arr) {
                                    if (c.is(json::String) && c.size() == 1) {
                                        exception.after.emplace(c.as<json::string>()[0]);
                                    }
                                }
                            }

                            if (e.contains("before", json::Array)) {
                                auto& arr = e["before"].as<json::array>();

                                for (auto& c : arr) {
                                    if (c.is(json::String) && c.size() == 1) {
                                        exception.before.emplace(c.as<json::string>()[0]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };

        // ------------------------------------------------

        zoomLevel.clear();

        if (theme.is(json::Object)) {
            loadZoomLevel(1, theme);
        } else if (theme.is(json::Array)) {
            auto& arr = theme.as<json::array>();
            for (auto& el : arr) {
                float zoom = el.contains("zoom", json::Floating) ? el["zoom"].as<json::floating>() : 1;
                loadZoomLevel(zoom, el);
            }
        }
    }

    // ------------------------------------------------

    void FontElement::draw(juce::Graphics& g, const Point<float>& pos, std::string_view str, Align align) const {
        if (auto _level = self->pickZoom(zoomLevel)) {

            float height = maxHeight;
            float width = stringWidth(str);
            float x = pos.x();
            float y = pos.y();

            if ((align & X) == Left) x = pos.x();
            if ((align & X) == Right) x = pos.x() - width;
            if ((align & X) == CenterX) x = pos.x() - Math::trunc(width / 2);
            if ((align & Y) == Top) y = pos.y();
            if ((align & Y) == Bottom) y = pos.y() - height;
            if ((align & Y) == CenterY) y = pos.y() - Math::trunc(height / 2);

            auto _image = self->image(_level->id);

            for (std::size_t i = 0; i < str.size(); ++i) {
                auto _before = i > 0 ? str[i - 1] : '\0';
                auto _c = str[i];
                auto _after = i < str.size() - 1 ? str[i + 1] : '\0';

                auto it = _level->charMap.find(_c);
                if (it != _level->charMap.end()) {
                    auto& _letter = it->second;

                    x += _letter.calcPreSpacing(_before, _after);

                    _image.draw(ClippedInstruction{
                        .graphics = g,
                        .clip = _letter.clip,
                        .position = { x, y, 
                            static_cast<float>(_letter.clip.width()), 
                            static_cast<float>(_letter.clip.height()) 
                        }
                    });


                    x += _letter.clip.width() + _letter.calcPostSpacing(_before, _after);
                } else {
                    // TODO: missing character rendering
                }
            }
        }
    }

    // ------------------------------------------------

    float FontElement::charWidth(char c, char before, char after) const {
        auto& render = zoomLevel.at(1);

        bool shouldSpace = before != '\0' && after != '\0';

        float total = 0;
        auto it = render.charMap.find(c);
        if (it != render.charMap.end()) {
            auto& letter = it->second;
            if (shouldSpace) total += letter.calcPreSpacing(before, after);
            else total += letter.preSpacing;
            total += letter.clip.width();
            if (shouldSpace) total += letter.calcPostSpacing(before, after);
            else total += letter.postSpacing;
        }

        return total;
    }

    // ------------------------------------------------

    float FontElement::stringWidth(std::string_view str) const {
        auto& render = zoomLevel.at(1);

        float total = 0;
        float finalPostSpacing = 0;

        for (std::size_t i = 0; i < str.size(); ++i) {
            auto before = i > 0 ? str[i - 1] : '\0';
            auto c = str[i];
            auto after = i < str.size() - 1 ? str[i + 1] : '\0';
            auto it = render.charMap.find(c);
            if (it != render.charMap.end()) {
                auto& letter = it->second;
                total += letter.calcPreSpacing(before, after);
                total += letter.clip.width();
                finalPostSpacing = letter.calcPostSpacing(before, after);
                total += finalPostSpacing;
            }
        }

        // Remove final post-spacing from end of string, as it's not necessary
        if (total != 0) total -= finalPostSpacing;

        return total;
    }

    // ------------------------------------------------
    
    std::string FontElement::fitWithinWidth(std::string_view str, float maxWidth, std::string_view d) const {
        std::string path{ str.begin(), str.end() };
        std::string delim{ d };

        while (stringWidth(path) > maxWidth && path.size() > 9) {
            path = path.substr(0, 3) + delim + path.substr(8);
        }

        return path;
    }

    // ------------------------------------------------

    FontElement::operator Font() const {
        struct Implementation : public Font::Interface {
            Implementation(const FontElement* self) : self(self) {}

            const FontElement* self;

            void draw(juce::Graphics& g, const Point<float>& pos, std::string_view str, Align align = Align::TopLeft) const override {
                self->draw(g, pos, str, align);
            }

            float stringWidth(std::string_view str) const override {
                return self->stringWidth(str);
            }

            float charWidth(char c, char before = '\0', char after = '\0') const override {
                return self->charWidth(c, before, after);
            }

            float fontSize() const override {
                return self->maxHeight;
            }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}
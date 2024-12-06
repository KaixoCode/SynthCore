#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

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

    // ------------------------------------------------

    void Font::draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align, bool fillAlphaWithColor) const {
        if (m_Graphics) m_Graphics->draw(g, pos, str, align, fillAlphaWithColor);
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

    basic_json::parser::result<basic_json> getFontDescription(Theme* self, const std::string& description) {
        std::filesystem::path path = description;
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            auto abspath = std::filesystem::absolute(path);

            std::ifstream file{ abspath };
            if (!file.is_open()) return std::string_view{ "Cannot open file" };
            
            return basic_json::parse(file_to_string(file));
        } else if (self->hasVariable(description)) {
            return self->variable(description);
        } else {
            return std::string_view{ "File does not exist" };
        }
    }

    // ------------------------------------------------

    void FontElement::interpret(const basic_json& theme) {
        maxHeight = 0;
        size = 0;
        id = NoImage;
        font = NoFont;
        charMap.clear();

        if (theme.contains("font", basic_json::String)) { // Font file
            font = self->registerFont(theme["font"].as<basic_json::string>());
        }
        
        if (theme.contains("size", basic_json::Number)) { // Font file
            size = theme["size"].as<float>();
            maxHeight = size;
        }

        if (theme.contains("map", basic_json::String)) { // Load font-map
            id = self->registerImage(theme["map"].as<basic_json::string>());
        }

        basic_json json;
        std::string description;
        if (theme.contains("description", basic_json::String)) { // Load description from file
            description = theme["description"].as<basic_json::string>();
            auto optional_json = getFontDescription(self, description);
            if (!optional_json) return;
            json = optional_json.value();
        } else if (theme.contains("description", basic_json::Object)) { // Load description from json
            json = theme["description"];
        }

        if (json.is(basic_json::Object)) {
            if (json.contains("default-spacing", basic_json::Number)) {
                defaultSpacing = json["default-spacing"].as<std::int64_t>();
            } else {
                defaultSpacing = 2;
            }

            for (auto& [key, val] : json.as<basic_json::object>()) {
                if (key.size() != 1) continue;

                Rect<int> clip{ 0, 0, 0, 0 };
                std::array<int, 4> arr4;
                if (val.try_get("location", arr4)) {
                    clip = { arr4[0], arr4[1], arr4[2], arr4[3] };
                }

                auto& mapped = charMap[key[0]] = Letter{ .clip = clip, };

                if (maxHeight < clip.height()) {
                    maxHeight = clip.height();
                }

                val.try_get("pre-spacing", mapped.preSpacing);
                val.try_get_or_default("post-spacing", mapped.postSpacing, defaultSpacing);

                if (val.contains("exceptions", basic_json::Array)) {
                    auto& arr = val["exceptions"].as<basic_json::array>();

                    for (auto& e : arr) {
                        if (!e.is(basic_json::Object)) continue;

                        auto& exception = mapped.exceptions.emplace_back();

                        e.try_get("pre-spacing", exception.preSpacing);
                        e.try_get("post-spacing", exception.postSpacing);

                        if (e.contains("after", basic_json::Array)) {
                            auto& arr = e["after"].as<basic_json::array>();

                            for (auto& c : arr) {
                                if (c.is(basic_json::String) && c.size() == 1) {
                                    exception.after.emplace(c.as<basic_json::string>()[0]);
                                }
                            }
                        }

                        if (e.contains("before", basic_json::Array)) {
                            auto& arr = e["before"].as<basic_json::array>();

                            for (auto& c : arr) {
                                if (c.is(basic_json::String) && c.size() == 1) {
                                    exception.before.emplace(c.as<basic_json::string>()[0]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ------------------------------------------------

    void FontElement::draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align, bool fillAlphaWithColor) const {
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

        if (font != NoFont) {
            auto _font = self->font(font).withHeight(size);
            g.setFont(_font);
            g.drawSingleLineText(std::string(str), x, y + _font.getAscent());
        } else if (auto _image = self->image(id)) {
            for (std::size_t i = 0; i < str.size(); ++i) {
                auto _before = i > 0 ? str[i - 1] : '\0';
                auto _c = str[i];
                auto _after = i < str.size() - 1 ? str[i + 1] : '\0';

                auto it = charMap.find(_c);
                if (it != charMap.end()) {
                    auto& _letter = it->second;

                    x += _letter.calcPreSpacing(_before, _after);

                    _image.draw(ClippedInstruction{
                        .graphics = g,
                        .clip = _letter.clip,
                        .bounds = { x, y, 
                            static_cast<float>(_letter.clip.width()), 
                            static_cast<float>(_letter.clip.height()) 
                        },
                        .fillAlphaWithColor = fillAlphaWithColor
                    });


                    x += _letter.clip.width() + _letter.calcPostSpacing(_before, _after);
                } else {
                    g.fillRect(Rect{ x, y, maxHeight * 0.6, maxHeight });
                    x += Math::trunc(maxHeight * 0.6) + 2;
                }
            }
        }
    }

    // ------------------------------------------------

    float FontElement::stringWidth(std::string_view str) const {
        if (font != NoFont) {
            auto f = self->font(font).withHeight(size);
            return f.getStringWidthFloat(juce::String(std::string(str)));
        }

        float total = 0;
        float finalPostSpacing = 0;

        for (std::size_t i = 0; i < str.size(); ++i) {
            auto before = i > 0 ? str[i - 1] : '\0';
            auto c = str[i];
            auto after = i < str.size() - 1 ? str[i + 1] : '\0';
            auto it = charMap.find(c);
            if (it != charMap.end()) {
                auto& letter = it->second;
                total += letter.calcPreSpacing(before, after);
                total += letter.clip.width();
                finalPostSpacing = letter.calcPostSpacing(before, after);
                total += finalPostSpacing;
            } else {
                total += Math::trunc(maxHeight * 0.6) + 2;
                finalPostSpacing = 2;
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
        std::size_t delimSize = delim.size();
        std::size_t minStartSize = 3;
        std::size_t removeNPerIter = 2;
        std::size_t minSize = delimSize + minStartSize + removeNPerIter;

        while (stringWidth(path) > maxWidth && path.size() >= minSize) {
            path = path.substr(0, minStartSize) + delim + path.substr(minSize);
        }

        return path;
    }

    // ------------------------------------------------

    FontElement::operator Font() const {
        struct Implementation : public Font::Interface {
            Implementation(const FontElement* self) : self(self) {}

            const FontElement* self;

            void draw(juce::Graphics& g, const Kaixo::Point<float>& pos, std::string_view str, Align align = Align::TopLeft, bool fillAlphaWithColor = false) const override {
                self->draw(g, pos, str, align, fillAlphaWithColor);
            }

            float stringWidth(std::string_view str) const override {
                return self->stringWidth(str);
            }

            float fontSize() const override {
                return self->maxHeight;
            }
        };

        return { std::make_unique<Implementation>(this) };
    }

    // ------------------------------------------------

}
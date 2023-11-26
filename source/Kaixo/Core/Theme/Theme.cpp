#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include <DefaultTheme.hpp>

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Theme::Theme() : Container(this) {
        std::string asString;
        asString.reserve(DefaultThemeBytes);
        for (auto& line : DefaultTheme) {
            asString += trim(line);
        }

        if (auto val = basic_json::parse(asString)) {
            m_DefaultTheme = val.value();
        }
    }

    // ------------------------------------------------
    
    const basic_json& Theme::variableOrValue(const basic_json& value) {
        if (value.is(basic_json::String)) {
            const std::string& _value = value.as<basic_json::string>();
            if (self->hasVariable(_value)) {
                return self->variable(_value);
            }
        }
        return value;
    }

    // ------------------------------------------------

    void Theme::openDefault() {
        open(m_DefaultTheme, Default);
        m_OpenedPath = Default;
    }

    bool Theme::reopen() {
        m_LoadedImagesByKey.clear();
        m_LoadedImages.clear();
        m_Variables.clear();
        m_IsThemeOpened = false;

        if (name() == Default) {
            openDefault();
            return true;
        } else {
            std::filesystem::path _backup = path();
            openDefault();
            return open(_backup);
        }
    }

    bool Theme::open(const std::filesystem::path& path) {
        std::ifstream _file{ std::filesystem::absolute(path) };
        
        if (!_file.is_open()) return false;

        if (auto _json = basic_json::parse(file_to_string(_file))) {
            ScopedCurrentPath _{ path.parent_path() };

            open(_json.value(), path.string());

            m_OpenedPath = path;
            return true;
        }

        return false;
    }

    // ------------------------------------------------

    ImageID Theme::registerImage(const std::filesystem::path& path, ZoomMultiplier zoom) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            std::string _absolutePath = std::filesystem::absolute(path).string();

            auto it = m_LoadedImagesByKey.find(_absolutePath);
            if (it != m_LoadedImagesByKey.end()) return it->second;
            else {
                juce::File _file{ _absolutePath };
                auto _image = juce::ImageFileFormat::loadFrom(_file);
                if (!_image.isValid()) return NoImage;
                ImageID id = m_LoadedImages.size();
                m_LoadedImagesByKey.emplace(std::move(_absolutePath), id);
                m_LoadedImages[id] = { std::move(_image), zoom };
                return id;
            }
        } else {
            auto it = m_LoadedImagesByKey.find(path.string());
            if (it != m_LoadedImagesByKey.end()) return it->second;
        }
        return NoImage;
    }

    ImageID Theme::registerImage(std::string_view key, std::string_view base64, ZoomMultiplier zoom) {
        // Remove from loaded images if key already loaded
        ImageID id = NoImage;
        auto it = m_LoadedImagesByKey.find(key);
        if (it != m_LoadedImagesByKey.end()) {
            id = it->second;
        } else {
            id = nextImageID();
        }

        std::string _decoded = base64_decode(base64);

        char* _raw = _decoded.data();
        std::size_t _bytes = _decoded.size();

        auto _image = juce::ImageFileFormat::loadFrom(_raw, _bytes);
        if (!_image.isValid()) return NoImage;
        m_LoadedImagesByKey.emplace(key, id);
        m_LoadedImages[id] = { std::move(_image), zoom };
        return id;
    }

    Image Theme::image(ImageID id) const {
        if (id == NoImage) return {};
        if (!m_LoadedImages.contains(id)) return {};
        return m_LoadedImages.at(id);
    }
    
    // ------------------------------------------------

    FontID Theme::registerFont(const std::filesystem::path& path, float size) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            std::string _absolutePath = std::filesystem::absolute(path).string();

            auto it = m_LoadedFontsByKey.find(_absolutePath);
            if (it != m_LoadedFontsByKey.end()) return it->second;
            else {
                std::ifstream _file{ _absolutePath, std::ios::binary };
                auto _data = file_to_string(_file);
                auto _font = juce::Typeface::createSystemTypefaceFor(_data.data(), _data.size());
                if (_font) return NoFont;
                FontID id = m_LoadedFonts.size();
                m_LoadedFontsByKey.emplace(std::move(_absolutePath), id);
                m_LoadedFonts[id] = std::move(_font);
                return id;
            }
        } else {
            auto it = m_LoadedFontsByKey.find(path.string());
            if (it != m_LoadedFontsByKey.end()) return it->second;
        }
        return NoFont;
    }

    FontID Theme::registerFont(std::string_view key, std::string_view base64, float size) {
        // Remove from loaded images if key already loaded
        FontID id = NoFont;
        auto it = m_LoadedFontsByKey.find(key);
        if (it != m_LoadedFontsByKey.end()) {
            id = it->second;
        } else {
            id = nextFontID();
        }

        std::string _decoded = base64_decode(base64);

        char* _raw = _decoded.data();
        std::size_t _bytes = _decoded.size();

        auto _font = juce::CustomTypeface::createSystemTypefaceFor(_raw, _bytes);
        if (!_font) return NoFont;
        m_LoadedFontsByKey.emplace(key, id);
        m_LoadedFonts[id] = std::move(_font);
        return id;
    }

    juce::Font Theme::font(FontID id) const {
        if (id == NoFont) return {};
        if (!m_LoadedFonts.contains(id)) return {};
        return m_LoadedFonts.at(id);
    }

    // ------------------------------------------------

    ImageID Theme::nextImageID() {
        ImageID id = 0;
        for (auto& [i, _] : m_LoadedImages) {
            if (id <= i) id = i + 1;
        }
        return id;
    }

    FontID Theme::nextFontID() {
        FontID id = 0;
        for (auto& [i, _] : m_LoadedFonts) {
            if (id <= i) id = i + 1;
        }
        return id;
    }

    // ------------------------------------------------

    void Theme::findVariables(basic_json& json) {
        if (json.contains("variables", basic_json::Object)) {
            auto& obj = json["variables"].as<basic_json::object>();
            for (auto& [key, val] : obj) {
                m_Variables.emplace(key, val);
            }
        }

        if (json.contains("images", basic_json::Object)) {
            auto& obj = json["images"].as<basic_json::object>();
            for (auto& [key, val] : obj) {
                if (val.is(basic_json::String)) {
                    registerImage(key, val.as<basic_json::string>());
                }
            }
        }

        if (json.contains("fonts", basic_json::Object)) {
            auto& obj = json["fonts"].as<basic_json::object>();
            for (auto& [key, val] : obj) {
                if (val.is(basic_json::String)) {
                    registerFont(key, val.as<basic_json::string>());
                }
            }
        }

        // Replace all occurences of variables
        json.forall([&](this auto& self, basic_json& value) -> void {
            if (value.is(basic_json::String)) {
                auto& val = value.as<basic_json::string>();
                if (hasVariable(val)) {
                    value = variable(val);
                    value.forall(self);
                }
            }
        });

        json.foreach([&](this auto& self, const std::string& key, basic_json& value) -> void {
            switch (value.type()) {
            case basic_json::Object: {
                // Recurse
                value.foreach(self);
                // Merge with extended variables
                while (value.contains("extends")) {
                    basic_json extends = value["extends"];
                    value.as<basic_json::object>().erase("extends"); // Remove after extending
                    value.merge(extends);
                    extends.foreach([&](const basic_json& val) { value.merge(val); });
                }
                break;
            }
            }
        });
    }

    void Theme::open(basic_json& json, std::string_view name) {
        if (json.contains("theme-name", basic_json::String)) {
            m_OpenedThemeName = json["theme-name"].as<basic_json::string>();
        } else {
            m_OpenedThemeName = name;
        }

        findVariables(json);
        interpret(json);
    }

    // ------------------------------------------------

}

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Theme::GeneratedTheme T{};

    // ------------------------------------------------

}

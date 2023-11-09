#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    Theme::Theme() : Container(this) {}

    // ------------------------------------------------

    void Theme::openDefault() {
        open(SYNTH_ThemeFile);
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
            auto& _backup = path();
            openDefault();
            return open(_backup);
        }
    }

    bool Theme::open(const std::filesystem::path& path) {
        std::ifstream _file{ std::filesystem::absolute(path) };
        
        if (!_file.is_open()) return false;

        if (auto _json = json::parse(file_to_string(_file))) {
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
                m_LoadedImages.emplace_back(std::move(_image), zoom);
                return id;
            }
        } else {
            auto it = m_LoadedImagesByKey.find(path.string());
            if (it != m_LoadedImagesByKey.end()) return it->second;
        }
        return NoImage;
    }

    ImageID Theme::registerImage(std::string_view key, std::string_view base64, ZoomMultiplier zoom) {
        std::string _decoded = base64_decode(base64);

        char* _raw = _decoded.data();
        std::size_t _bytes = _decoded.size();

        auto _image = juce::ImageFileFormat::loadFrom(_raw, _bytes);
        if (!_image.isValid()) return NoImage;
        ImageID id = m_LoadedImages.size();
        m_LoadedImagesByKey.emplace(key, id);
        m_LoadedImages.emplace_back(std::move(_image), zoom);
        return id;
    }

    Image Theme::image(ImageID id) const {
        if (id == NoImage) return {};
        if (id >= m_LoadedImages.size()) return {};
        return m_LoadedImages[id];
    }

    // ------------------------------------------------

    void Theme::findVariables(const json& json) {
        if (json.contains("variables", json::Object)) {
            auto& obj = json["variables"].as<json::object>();
            for (auto& [key, val] : obj) {
                m_Variables.emplace(key, val);
            }
        }

        if (json.contains("images", json::Object)) {
            auto& obj = json["images"].as<json::object>();
            for (auto& [key, val] : obj) {
                if (val.is(json::String)) {
                    registerImage(key, val.as<json::string>());
                }
            }
        }
    }

    void Theme::open(json& json, std::string_view name) {
        if (json.contains("theme-name", json::String)) {
            m_OpenedThemeName = json["theme-name"].as<json::string>();
        } else {
            m_OpenedThemeName = name;
        }

        findVariables(json);
        interpret(json);
    }

    // ------------------------------------------------

}
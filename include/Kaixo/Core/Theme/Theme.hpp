#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/Image.hpp"
#include "Kaixo/Core/Theme/Color.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/TextArea.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    constexpr static std::string_view Default = "Default";

    // ------------------------------------------------

    class Theme : public Container {
    public:

        // ------------------------------------------------

        Theme();

        // ------------------------------------------------

        bool hasVariable(std::string_view name) const { return m_Variables.contains(name); };
        const basic_json& variable(std::string_view name) { return m_Variables.find(name)->second; };
        const basic_json& variableOrValue(const basic_json& value);

        // ------------------------------------------------
        
        void initialize();

        void openDefault();
        bool reopen();
        bool open(const std::filesystem::path& path);
        bool opened() const { return m_IsThemeOpened; }

        std::string_view lastError() const { return m_LastErrorMessage; }

        std::string_view name() const { return m_OpenedThemeName; }
        const std::filesystem::path& path() const { return m_OpenedPath; }

        // ------------------------------------------------
        
        ImageID registerImage(const std::filesystem::path& path, ZoomMultiplier zoom = 1);
        ImageID registerImage(std::string_view id, std::string_view base64, ZoomMultiplier zoom = 1);

        Image image(ImageID id) const;

        // ------------------------------------------------

        FontID registerFont(const std::filesystem::path& path, float size = 1);
        FontID registerFont(std::string_view id, std::string_view base64, float size = 1);

        juce::Font font(FontID id) const;

        // ------------------------------------------------
        
        ZoomMultiplier currentZoom() const { return m_Zoom; }

        // ------------------------------------------------

        template<class Ty>
        const Ty* pickZoom(const std::map<ZoomMultiplier, Ty>& zoomLevels) const;

        // ------------------------------------------------

    private:
        mutable std::recursive_mutex m_Mutex{};
        basic_json m_DefaultTheme{};
        std::map<std::string, basic_json, std::less<>> m_Variables{};
        std::map<std::string, ImageID, std::less<void>> m_LoadedImagesByKey{};
        std::map<ImageID, Image> m_LoadedImages;
        std::map<std::string, FontID, std::less<void>> m_LoadedFontsByKey{};
        std::map<FontID, juce::Font> m_LoadedFonts;
        std::filesystem::path m_OpenedPath;
        std::string m_OpenedThemeName;
        std::string m_LastErrorMessage;
        ZoomMultiplier m_Zoom = 1;
        bool m_IsThemeOpened = false;

        // ------------------------------------------------
        
        ImageID nextImageID();
        FontID nextFontID();

        // ------------------------------------------------

        void findVariables(basic_json& json);
        bool open(basic_json& json, std::string_view name);
        
        // ------------------------------------------------
        
        void clearCache();

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty>
    const Ty* Theme::pickZoom(const std::map<ZoomMultiplier, Ty>& zoomLevels) const {

        if (zoomLevels.size() == 0) return nullptr;

        // Find zoom that's closest to current zoom;
        auto _zoom = currentZoom();
        auto _selected = _zoom;
        double _dist = 100;
        for (auto& [level, _] : zoomLevels) {
            double _diff = Math::abs(_zoom - level);
            if (_diff < _dist) {
                _selected = level;
                _dist = _diff;
            }
        }

        return &zoomLevels.at(_selected);
    }

    // ------------------------------------------------

}

#include <GeneratedTheme.hpp>
#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/Image.hpp"

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
        const json& variable(std::string_view name) { return m_Variables.find(name)->second; };

        // ------------------------------------------------
        
        void openDefault();
        bool reopen();
        bool open(const std::filesystem::path& path);
        bool opened() const { return m_IsThemeOpened; }

        std::string_view name() const { return m_OpenedThemeName; }
        const std::filesystem::path& path() const { return m_OpenedPath; }

        // ------------------------------------------------
        
        ImageID registerImage(const std::filesystem::path& path, ZoomMultiplier zoom = 1);
        ImageID registerImage(std::string_view id, std::string_view base64, ZoomMultiplier zoom = 1);

        Image image(ImageID id) const;

        // ------------------------------------------------
        
        ZoomMultiplier currentZoom() const { return m_Zoom; }

        // ------------------------------------------------

        template<class Ty>
        const Ty* pickZoom(const std::map<ZoomMultiplier, Ty>& zoomLevels) const;

        // ------------------------------------------------

    private:
        std::map<std::string, json, std::less<>> m_Variables{};
        std::map<std::string, ImageID> m_LoadedImagesByKey{};
        std::vector<Image> m_LoadedImages;
        std::filesystem::path m_OpenedPath;
        std::string m_OpenedThemeName;
        ZoomMultiplier m_Zoom = 1;
        bool m_IsThemeOpened = false;

        // ------------------------------------------------

        void findVariables(const json& json);
        void open(json& json, std::string_view name);
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
#pragma once
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"

namespace Kaixo::Gui {

    // ------------------------------------------------

    class Tooltip : public View {
    public:

        // ------------------------------------------------

        Tooltip(Context c);

        // ------------------------------------------------

        struct Settings {
            std::string string = "";
            Point<> position{ 0, 0 }; // Should be global position (screen)
            Point<> padding{ 4, 4 };
            Theme::Font font;
            Theme::Basic background;
            Theme::Align align = Theme::Align::Center;
        };

        // ------------------------------------------------

        void open(Settings settings);
        void update(std::string_view str);
        void close();

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------
        
        void font(Theme::Font font);
        void background(Theme::Basic bg);

        // ------------------------------------------------

    private:
        Settings m_Settings{};
        Theme::Basic m_Background{};
        Theme::Font m_Font{};

        // ------------------------------------------------
        
        void updatePosition();

        // ------------------------------------------------

    };
}
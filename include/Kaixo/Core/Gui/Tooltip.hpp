#pragma once
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"
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
            Theme::Color textColor;
            Theme::Drawable background;
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
        void background(Theme::Drawable bg);
        void textColor(Theme::Color clr);

        // ------------------------------------------------

    private:
        Settings m_Settings{};
        Theme::Drawable m_Background{};
        Theme::Font m_Font{};
        Theme::Color m_TextColor{};

        // ------------------------------------------------
        
        void updatePosition();

        // ------------------------------------------------

    };
}
#include "Kaixo/Core/Gui/Tooltip.hpp"

namespace Kaixo::Gui {

    // ------------------------------------------------

    Tooltip::Tooltip(Context c) : View(c) {
        setInterceptsMouseClicks(false, false);
        close();
    }

    // ------------------------------------------------

    void Tooltip::open(Settings settings) {
        m_Settings = std::move(settings);
        updatePosition();
        setVisible(true);
        repaint();
    }

    void Tooltip::update(std::string_view str) {
        m_Settings.string = str;
        updatePosition();
        repaint();
    }

    void Tooltip::close() {
        setVisible(false);
    }

    // ------------------------------------------------

    void Tooltip::paint(juce::Graphics& g) {
        auto& font = m_Settings.font ? m_Settings.font : m_Font;
        auto& background = m_Settings.background ? m_Settings.background : m_Background;
        auto& color = m_Settings.textColor ? m_Settings.textColor : m_TextColor;
        bool useColor = m_Settings.textColor || m_TextColor;

        if (useColor) g.setColour(color);
        background.draw(g, localDimensions());
        font.draw(g, { 
            m_Settings.padding.x(), 
            m_Settings.padding.y()
        }, m_Settings.string, Theme::Align::TopLeft, useColor);
    }

    // ------------------------------------------------

    void Tooltip::font(Theme::Font font) { m_Font = std::move(font); updatePosition(); }
    void Tooltip::background(Theme::Basic bg) { m_Background = std::move(bg); }
    void Tooltip::textColor(Theme::Color clr) { m_TextColor = std::move(clr); }

    // ------------------------------------------------

    void Tooltip::updatePosition() {
        auto window = getParentComponent();
        if (!window) return;

        auto& font = m_Settings.font ? m_Settings.font : m_Font;
        auto& background = m_Settings.background ? m_Settings.background : m_Background;
        
        Point position = window->getLocalPoint(nullptr, m_Settings.position);

        float x = position.x();
        float y = position.y();
        float width = font.stringWidth(m_Settings.string) + m_Settings.padding.x() * 2;
        float height = font.fontSize() + m_Settings.padding.y() * 2;

        if ((m_Settings.align & Theme::Align::X) == Theme::Align::Left) x = x;
        if ((m_Settings.align & Theme::Align::X) == Theme::Align::Right) x = x - width;
        if ((m_Settings.align & Theme::Align::X) == Theme::Align::CenterX) x = x - Math::trunc(width / 2);
        if ((m_Settings.align & Theme::Align::Y) == Theme::Align::Top) y = y;
        if ((m_Settings.align & Theme::Align::Y) == Theme::Align::Bottom) y = y - height;
        if ((m_Settings.align & Theme::Align::Y) == Theme::Align::CenterY) y = y - Math::trunc(height / 2);

        Coord windowPadding = 4;

        if (x < windowPadding) x = windowPadding;
        if (y < windowPadding) y = windowPadding;
        if (x + width > window->getWidth() - windowPadding) x = window->getWidth() - width - windowPadding;
        if (y + height > window->getHeight() - windowPadding) y = window->getHeight() - height - windowPadding;

        dimensions(Rect{ x, y, width, height });
    }
    
    // ------------------------------------------------


}
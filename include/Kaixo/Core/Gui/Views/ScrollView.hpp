#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class ScrollView : public View {
    public:

        // ------------------------------------------------
        
        enum class Type {
            Vertical, Horizontal
        };

        struct Settings {
            Type type = Type::Vertical;
            Theme::Drawable background;
            Theme::Drawable scrollbar;
            Rect<> margin{ 4, 4, 4, 4 };     // Margin around the contained views
            Coord gap = 4;                   // Gap between contained views
            Coord minSize = 20;              // Minimum scroll bar size
            Coord barThickness = 10;         // Thickness of the scroll bar
            Rect<> barPadding{ 4, 4, 4, 4 }; // Padding around the scroll bar
            bool keepBarSpace = true;        // Keep the space of the scroll bar unused
            Theme::Align alignChildren = Theme::Align::Center; // Alignment of children
        } settings;

        // ------------------------------------------------

        ScrollView(Context c, Settings s = {});

        // ------------------------------------------------

        template<class Ty, class ...As> Ty& add(As&& ...as);
        template<class Ty, class ...As> Ty& add(UnevaluatedPoint size, As&& ...as);
        template<class Ty> Ty& add(Ty::Settings settings);
        template<class Ty> Ty& add(UnevaluatedPoint size, Ty::Settings settings);

        // ------------------------------------------------

        void mouseMove     (const juce::MouseEvent& event) override;
        void mouseEnter    (const juce::MouseEvent& event) override;
        void mouseExit     (const juce::MouseEvent& event) override;
        void mouseDrag     (const juce::MouseEvent& event) override;
        void mouseDown     (const juce::MouseEvent& event) override;
        void mouseUp       (const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;
        void paintOverChildren(juce::Graphics& g) override;

        // ------------------------------------------------
        
        void updateDimensions() override;

        // ------------------------------------------------

    protected:
        Coord m_HighestCoord = 0;
        float m_Scrolled = 0;
        float m_PressScrolled = 0;

        // ------------------------------------------------

        bool scrollbarNecessary() { return scrollSpace() > 0; }
        
        Coord scrollSpace();
        Coord barSize();
        Coord barPosition();
        Rect<> scrollbar();
        Rect<> childrenRect();
        void positionChildren();

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

    template<class Ty, class ...As>
    Ty& ScrollView::add(As&& ...as) {
        auto& value = View::add<Ty>(std::forward<As>(as)...);
        value.useDimensions(false);
        positionChildren();
        return value;
    }

    template<class Ty, class ...As>
    Ty& ScrollView::add(UnevaluatedPoint size, As&& ...as) {
        auto& value = View::add<Ty>(UnevaluatedRect{ 0, 0, size.x, size.y }, std::forward<As>(as)...);
        value.useDimensions(false);
        positionChildren();
        return value;
    }

    template<class Ty>
    Ty& ScrollView::add(Ty::Settings settings) {
        auto& value = View::add<Ty>(std::move(settings));
        value.useDimensions(false);
        positionChildren();
        return value;
    }

    template<class Ty>
    Ty& ScrollView::add(UnevaluatedPoint size, Ty::Settings settings) {
        auto& value = View::add<Ty>(UnevaluatedRect{ 0, 0, size.x, size.y }, std::move(settings));
        value.useDimensions(false);
        positionChildren();
        return value;
    }

    // ------------------------------------------------

}
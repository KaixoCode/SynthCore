#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

     // ------------------------------------------------

    struct TextArea {

        // ------------------------------------------------

        Font font;
        Color textColor;
        Color selectionColor;
        Color caretColor;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class TextAreaElement : public Element {
    public:

        // ------------------------------------------------

        using Element::Element;

        // ------------------------------------------------

        FontElement font{ self };
        ColorElement textColor{ self };
        ColorElement selectionColor{ self };
        ColorElement caretColor{ self };

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator TextArea() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
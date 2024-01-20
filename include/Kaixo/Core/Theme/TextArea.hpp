#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Theme/Element.hpp"
#include "Kaixo/Core/Theme/Container.hpp"
#include "Kaixo/Core/Theme/ZoomMultiplier.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

     // ------------------------------------------------

    struct TextArea : Animation {

        // ------------------------------------------------
        
        using Interface = void; // No interface, just collection of other element

        // ------------------------------------------------

        Font font;
        Color textColor;
        Color placeholderColor;
        Color selectionColor;
        Color caretColor;

        // ------------------------------------------------
        
        bool changing() const override;

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
        ColorElement placeholderColor{ self };
        ColorElement selectionColor{ self };
        ColorElement caretColor{ self };

        // ------------------------------------------------

        void interpret(const basic_json& theme) override;

        // ------------------------------------------------

        operator TextArea() const;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<>
    inline DynamicElement::operator TextArea() {
        if (auto textarea = dynamic_cast<TextAreaElement*>(m_Element)) {
            return *textarea;
        }

        return {};
    }

    // ------------------------------------------------

}
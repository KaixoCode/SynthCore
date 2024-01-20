#include "Kaixo/Core/Theme/TextArea.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    bool TextArea::changing() const {
        return textColor.changing() 
            || caretColor.changing() 
            || selectionColor.changing();
    }

    // ------------------------------------------------

    void TextAreaElement::interpret(const basic_json& theme) {
        if (theme.contains("font")) font.interpret(theme["font"]);
        if (theme.contains("placeholder-color")) placeholderColor.interpret(theme["placeholder-color"]);
        if (theme.contains("text-color")) textColor.interpret(theme["text-color"]);
        if (theme.contains("caret-color")) caretColor.interpret(theme["caret-color"]);
        if (theme.contains("selection-color")) selectionColor.interpret(theme["selection-color"]);
    }

    // ------------------------------------------------

    TextAreaElement::operator TextArea() const {
        TextArea result;
        result.font = font;
        result.placeholderColor = placeholderColor;
        result.textColor = textColor;
        result.selectionColor = selectionColor;
        result.caretColor = caretColor;
        return result;
    }

    // ------------------------------------------------

}
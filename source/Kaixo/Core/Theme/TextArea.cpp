#include "Kaixo/Core/Theme/TextArea.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    void TextAreaElement::interpret(const basic_json& theme) {
        if (theme.contains("font")) font.interpret(theme["font"]);
        if (theme.contains("text-color")) textColor.interpret(theme["text-color"]);
        if (theme.contains("caret-color")) caretColor.interpret(theme["caret-color"]);
        if (theme.contains("selection-color")) selectionColor.interpret(theme["selection-color"]);
    }

    // ------------------------------------------------

    TextAreaElement::operator TextArea() const {
        return TextArea{ 
            .font = font,
            .textColor = textColor,
            .selectionColor = selectionColor,
            .caretColor = caretColor,
        };
    }

    // ------------------------------------------------

}
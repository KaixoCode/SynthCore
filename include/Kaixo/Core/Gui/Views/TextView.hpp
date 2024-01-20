#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"
#include "Kaixo/Core/Theme/FontMap.hpp"
#include "Kaixo/Core/Theme/Color.hpp"
#include "Kaixo/Core/Theme/TextArea.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class TextView : public View {
    public:

        // ------------------------------------------------

        struct Settings {
            Theme::Drawable background;
            Theme::TextArea graphics;

            Point<> padding{ 8, 8 };

            bool multiline = false;
            bool editable = true;
            bool center = false;

            Coord lineHeight = 20;

            std::size_t maxSize = 32; // Maximum amount of characters
            std::size_t maxLines = 3; // Maximum number of lines

            std::string placeholder = "";
            std::string text = "";
        } settings;

        // ------------------------------------------------

        TextView(Context c, Settings s = {});

        // ------------------------------------------------
        
        void focusGained(FocusChangeType cause) override { View::focusGained(cause); repaint(); };
        void focusLost(FocusChangeType cause) override { View::focusLost(cause); repaint(); };

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        bool keyPressed(const juce::KeyPress& key) override;

        // ------------------------------------------------
        
        void addCallback(std::function<void(std::string_view)> c) { m_Callbacks.push_back(std::move(c)); }

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------
        
        bool empty() const { return settings.text.empty(); }
        bool length() const { return settings.text.length(); }

        std::string& content() { return settings.text; }

        // ------------------------------------------------

        Rect<> paddedDimensions() const;

        // ------------------------------------------------

        std::vector<std::string_view> linesFrom(std::string_view string) const;
        std::vector<std::string_view> lines() const;
        std::string_view selection() const;

        // ------------------------------------------------

        void changed();
        void setText(std::string_view string, bool withNotify = false);
        void insert(char c, bool withDelete = true);
        void insert(std::string_view c, bool withDelete = true);
        void deleteIndex(std::int64_t index);
        void deleteRange(std::int64_t begin, std::int64_t end);

        // ------------------------------------------------

        bool hasSelection() const;
        void centerText();
        void moveOffsetToFitCaret();
        void deleteSelection();
        void setCaret(std::int64_t position, bool moveEnd = true);
        void selectAll();

        enum class CharacterCategory { Alnum, Symbol, Whitespace };

        CharacterCategory characterizeChar(char c) const;
        CharacterCategory characterizeAtIndex(std::int64_t i) const;

        std::int64_t findEndOfWord(int dir) const;
        std::int64_t findEndOfWordFrom(int dir, std::int64_t index) const;
        std::int64_t findEndOfWordLeft() const;
        std::int64_t findEndOfWordLeftFrom(std::int64_t index) const;
        std::int64_t findEndOfWordRight() const;
        std::int64_t findEndOfWordRightFrom(std::int64_t index) const;

        void moveToWord(int dir, bool moveEnd = true);
        void moveToWordLeft(bool moveEnd = true);
        void moveToWordRight(bool moveEnd = true);

        void selectWord(int dir);
        void selectWordLeft();
        void selectWordRight();

        void move(int dir, bool word = false, bool moveEnd = true);
        void moveLeft(bool word = false, bool moveEnd = true);
        void moveRight(bool word = false, bool moveEnd = true);

        // ------------------------------------------------

        void doBackspace(bool word = false);
        void doDelete(bool word = false);
        void doLeft(bool word = false, bool moveEnd = true);
        void doRight(bool word = false, bool moveEnd = true);

        // ------------------------------------------------

        float stringWidth(std::string_view str) const;

        std::int64_t yToLine(float y) const;
        std::int64_t xToIndex(std::string_view line, float x) const;

        std::int64_t positionToIndex(Point<int> pos) const;
        Point<int> indexToPosition(std::int64_t index) const;

        // ------------------------------------------------

    private:
        std::vector<std::function<void(std::string_view)>> m_Callbacks;
        Point<float> m_Offset{ 0, 0 };
        std::int64_t m_Caret = 0;
        std::int64_t m_CaretEnd = 0;

        // ------------------------------------------------

    };
}
#include "Kaixo/Core/Gui/Views/TextView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    TextView::TextView(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        setWantsKeyboardFocus(true);
        setMouseCursor(juce::MouseCursor::IBeamCursor);
        animation(settings.graphics);
    }
    
    // ------------------------------------------------

    void TextView::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        setCaret(positionToIndex({ event.x, event.y }));
    }
    
    void TextView::mouseDoubleClick(const juce::MouseEvent& event) {
        View::mouseDoubleClick(event);
        std::size_t index = positionToIndex({ event.x, event.y });
        std::size_t start = findEndOfWordLeftFrom(index);
        std::size_t end = findEndOfWordRightFrom(start);
        m_Caret = start;
        m_CaretEnd = end;
        repaint();
    }

    void TextView::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);
        setCaret(positionToIndex({ event.x, event.y }), false);
        repaint();
    }

    bool TextView::keyPressed(const juce::KeyPress& key) {
        if (!focused()) return false;

        bool ctrl = key.getModifiers().isCtrlDown();
        bool shft = key.getModifiers().isShiftDown();

        repaint();

        auto code = key.getKeyCode();

        if (code == juce::KeyPress::returnKey) { focused(false); return true; }
        if (code == juce::KeyPress::escapeKey) { focused(false); return true; }
        if (code == juce::KeyPress::leftKey) { doLeft(ctrl, !shft); return true; }
        if (code == juce::KeyPress::rightKey) { doRight(ctrl, !shft); return true; }
        if (code == juce::KeyPress::upKey) { return true; }
        if (code == juce::KeyPress::downKey) { return true; }
        if (code == juce::KeyPress::tabKey) { focusSibling(!shft); return true; };
        if (code == juce::KeyPress::endKey) { m_Caret = settings.text.size(); return true; }
        if (code == juce::KeyPress::homeKey) { m_Caret = 0; return true; }

        if (!settings.editable) return false;

        if (code == juce::KeyPress::deleteKey) { doDelete(ctrl); return true; }
        if (code == juce::KeyPress::backspaceKey) { doBackspace(ctrl); return true; }

        if (code == 'A' && ctrl) { selectAll(); return true; }
        if (code == 'C' && ctrl) { juce::SystemClipboard::copyTextToClipboard(std::string{ selection() }); return true; }
        if (code == 'V' && ctrl) { insert(juce::SystemClipboard::getTextFromClipboard().toStdString()); return true; }
        if (code == 'X' && ctrl) {
            juce::SystemClipboard::copyTextToClipboard(std::string{ selection() });
            deleteSelection();
            return true;
        }

        auto character = key.getTextCharacter();

        if (juce::CharacterFunctions::isPrintable(character) && character < 0xFF) {
            return insert((char)(character & 0xFF));
        }

        return false;
    }

    // ------------------------------------------------

    void TextView::paint(juce::Graphics& g) {
        settings.graphics.background.draw({
            .graphics = g, 
            .bounds = localDimensions(),
            .state = state()
        });

        auto v = paddedDimensions().topLeft().toFloat() - m_Offset;
        if (settings.alignToPixel) v = v.toInt().toFloat();
        if (settings.text.empty()) {
            auto lines = this->linesFrom(settings.placeholder);
            if (settings.graphics.placeholderColor) {
                g.setColour(settings.graphics.placeholderColor.get(state()));
            } else {
                g.setColour(settings.graphics.textColor.get(state()));
            }
            for (auto& line : lines) {
                settings.graphics.font.draw(g, v, line, Theme::TopLeft, true);
                v.y += settings.lineHeight;
            }
        }

        if (focused()) {
            std::int64_t begin = std::min(m_Caret, m_CaretEnd);
            std::int64_t end = std::max(m_Caret, m_CaretEnd);

            Point beginPos = indexToPosition(begin);
            Point endPos = indexToPosition(end);
            Point caretPos = m_Caret < m_CaretEnd ? beginPos : endPos;

            auto v = paddedDimensions();
            g.setColour(settings.graphics.selectionColor.get(state()));
            if (beginPos.y() == endPos.y()) {
                g.fillRect(Rect{
                    beginPos.x(),
                    beginPos.y(),
                    endPos.x() - beginPos.x() + 2,
                    settings.lineHeight
                });
            } else {
                g.fillRect(Rect{
                    beginPos.x(),
                    beginPos.y(),
                    v.right() - beginPos.x(),
                    settings.lineHeight
                });

                g.fillRect(Rect{
                    v.x(),
                    beginPos.y() + settings.lineHeight,
                    v.width(),
                    endPos.y() - beginPos.y() - settings.lineHeight
                });

                g.fillRect(Rect{
                    v.x(),
                    endPos.y(),
                    endPos.x() - v.x() + 2,
                    settings.lineHeight
                });
            }
            
            g.setColour(settings.graphics.caretColor.get(state()));
            g.fillRect(Rect{ caretPos.x(), caretPos.y(), 2, settings.lineHeight });
        }

         if (!settings.text.empty()) {
            auto lines = this->lines();
            g.setColour(settings.graphics.textColor.get(state()));
            for (auto& line : lines) {
                settings.graphics.font.draw(g, v, line, Theme::TopLeft, true);
                v.y += settings.lineHeight;
            }
        }
    }

    // ------------------------------------------------

    Rect<> TextView::paddedDimensions() const {
        return {
            settings.padding.x(),
            settings.padding.y(),
            width() - settings.padding.x() * 2,
            height() - settings.padding.y() * 2,
        };
    }

    // ------------------------------------------------

    std::vector<std::string_view> TextView::linesFrom(std::string_view string) const {
        if (settings.multiline) {
            auto v = paddedDimensions();
            std::vector<std::string_view> lines;
            std::string_view line = string;
            std::size_t index = 0;
            while (index < string.size()) {
                line = string.substr(index);

                while (stringWidth(trim(line)) > v.getWidth()) {
                    std::string_view view = line;
                    std::size_t found = std::min(view.find_last_of(" "), view.find_last_of("\n"));
                    if (found == npos || found == 0) break;
                    line = view.substr(0, found);
                }

                index += line.size();
                lines.push_back(trimBegin(line));
            }

            if (lines.size() == 0)
                lines.push_back(string);

            return lines;
        } else {
            return { string };
        }
    }

    std::vector<std::string_view> TextView::lines() const {
        return linesFrom(settings.text);
    }

    std::string_view TextView::selection() const {
        std::int64_t begin = std::min(m_Caret, m_CaretEnd);
        std::int64_t end = std::max(m_Caret, m_CaretEnd);
        return std::string_view{ settings.text }.substr(begin, end - begin);
    }

    // ------------------------------------------------


    bool TextView::hasFilter() const {
        return !settings.allowedCharacters.empty();
    }

    bool TextView::allowed(char c) const {
        return !hasFilter() || oneOf(c, settings.allowedCharacters);
    }

    std::string TextView::filter(std::string_view text) const {
        if (!hasFilter()) return std::string{ text };
        std::stringstream out;
        for (char c : text) {
            if (allowed(c)) out << c;
        }
        return out.str();
    }

    // ------------------------------------------------

    void TextView::changed() {
        repaint();
        for (auto& callback : m_Callbacks)
            callback(settings.text);
    }

    void TextView::setText(std::string_view string, bool withNotify) {
        m_Offset = { 0, 0 };
        settings.text = string;
        setCaret(string.size());
        if (withNotify) changed();
    }

    bool TextView::insert(char c, bool withDelete) {
        if (withDelete) deleteSelection();
        if (settings.text.size() >= settings.maxSize) return false;
        if (!allowed(c)) return false;
        settings.text.insert(settings.text.begin() + m_Caret, c);
        if (lines().size() > settings.maxLines) {
            deleteIndex(m_Caret);
        } else {
            setCaret(m_Caret + 1);
            changed();
        }

        return true;
    }

    bool TextView::insert(std::string_view c, bool withDelete) {
        if (withDelete) deleteSelection();
        std::string filtered;
        if (hasFilter()) {
            filtered = filter(c);
            c = filtered;

            if (c.empty()) return false;
        }

        auto insertN = std::min(settings.maxSize - settings.text.size(), c.size());
        settings.text.insert(settings.text.begin() + m_Caret, c.begin(), c.begin() + insertN);
        if (lines().size() > settings.maxLines) {
            deleteRange(m_Caret, m_Caret + insertN);
        } else {
            setCaret(m_Caret + insertN);
            changed();
        }

        return true;
    }

    void TextView::deleteIndex(std::int64_t index) {
        if (index < 0 || index >= settings.text.size()) return;
        settings.text.erase(settings.text.begin() + index);
        changed();
    }

    void TextView::deleteRange(std::int64_t begin, std::int64_t end) {
        begin = Math::Fast::clamp(begin, 0, settings.text.size());
        end = Math::Fast::clamp(end, 0, settings.text.size());
        settings.text.erase(settings.text.begin() + begin, settings.text.begin() + end);
        changed();
    }

    // ------------------------------------------------

    bool TextView::hasSelection() const { return m_Caret != m_CaretEnd; }

    void TextView::centerText() {
        auto v = paddedDimensions();
        auto w = stringWidth(settings.text);
        if (w < v.getWidth()) {
            m_Offset.x(w * 0.5 - 0.5 * v.getWidth());
        }
    }

    void TextView::moveOffsetToFitCaret() {
        auto v = paddedDimensions();

        // If no overflow && center && not multiline
        if (settings.center && !settings.multiline) {
            centerText();
        }

        auto p = indexToPosition(m_Caret);

        if (p.x() > v.right()) m_Offset.x(m_Offset.x() + p.x() - v.right());
        if (p.x() < v.left()) m_Offset.x(m_Offset.x() + p.x() - v.left());
        if (settings.multiline) {
            if (p.y() + settings.lineHeight > v.bottom()) m_Offset.y(m_Offset.y() + p.y() + settings.lineHeight - v.bottom());
            if (p.y() < v.top()) m_Offset.y(m_Offset.y() + p.y() - v.top());
        }
    }

    void TextView::deleteSelection() {
        std::int64_t begin = std::min(m_Caret, m_CaretEnd);
        std::int64_t end = std::max(m_Caret, m_CaretEnd);
        deleteRange(begin, end);
        setCaret(begin);
    }

    void TextView::setCaret(std::int64_t position, bool moveEnd) {
        std::int64_t constrained = Math::Fast::clamp(position, 0, settings.text.size());
        m_Caret = constrained;
        if (moveEnd) m_CaretEnd = constrained;
        moveOffsetToFitCaret();
    }

    void TextView::selectAll() {
        setCaret(settings.text.size(), false);
        m_CaretEnd = 0;
    }

    TextView::CharacterCategory TextView::characterizeChar(char c) const {
        if (std::isalnum(c)) return CharacterCategory::Alnum;
        else if (std::isspace(c)) return CharacterCategory::Whitespace;
        else return CharacterCategory::Symbol;
    }

    TextView::CharacterCategory TextView::characterizeAtIndex(std::int64_t i) const {
        return characterizeChar(settings.text[i]);
    }

    std::int64_t TextView::findEndOfWord(int dir) const { return findEndOfWordFrom(dir, m_Caret); }
    std::int64_t TextView::findEndOfWordFrom(int dir, std::int64_t index) const {
        return dir == 1 ? findEndOfWordRightFrom(index) : findEndOfWordLeftFrom(index);
    }

    std::int64_t TextView::findEndOfWordLeft() const { return findEndOfWordLeftFrom(m_Caret); }
    std::int64_t TextView::findEndOfWordLeftFrom(std::int64_t index) const {
        using enum CharacterCategory;
        auto category = Whitespace;
        for (std::int64_t i = index; i != 0;) {
            auto newCategory = characterizeAtIndex(--i);
            if (category != Whitespace && newCategory != category) return i + 1;
            category = newCategory;
        }
        return 0;
    }

    std::int64_t TextView::findEndOfWordRight() const { return findEndOfWordRightFrom(m_Caret); }
    std::int64_t TextView::findEndOfWordRightFrom(std::int64_t index) const {
        using enum CharacterCategory;
        auto size = settings.text.size();
        auto category = index == size ? Whitespace : characterizeAtIndex(index);
        for (std::int64_t i = index; i != size; ++i) {
            auto newCategory = characterizeAtIndex(i);
            if (newCategory != Whitespace && newCategory != category) return i;
            category = newCategory;
        }
        return settings.text.size();
    }

    void TextView::moveToWord(int dir, bool moveEnd) { setCaret(findEndOfWord(dir), moveEnd); }
    void TextView::moveToWordLeft(bool moveEnd) { moveToWord(-1, moveEnd); }
    void TextView::moveToWordRight(bool moveEnd) { moveToWord(1, moveEnd); }

    void TextView::selectWord(int dir) { moveToWord(dir, false); }
    void TextView::selectWordLeft() { moveToWordLeft(false); }
    void TextView::selectWordRight() { moveToWordRight(false); }

    void TextView::move(int dir, bool word, bool moveEnd) {
        if (word) moveToWord(dir, moveEnd);
        else setCaret(m_Caret + dir, moveEnd);
    }

    void TextView::moveLeft(bool word, bool moveEnd) { move(-1, word, moveEnd); }
    void TextView::moveRight(bool word, bool moveEnd) { move(1, word, moveEnd); }

    // ------------------------------------------------

    void TextView::doBackspace(bool word) {
        if (hasSelection()) deleteSelection();
        else if (word) { 
            selectWordLeft(); 
            deleteSelection();
        } else {
            deleteIndex(m_Caret - 1);
            setCaret(m_Caret - 1);
        }
    }

    void TextView::doDelete(bool word) {
        if (hasSelection()) deleteSelection();
        else if (word) {
            selectWordRight();
            deleteSelection();
        } else {
            deleteIndex(m_Caret);
        }
    }

    void TextView::doLeft(bool word, bool moveEnd) {
        if (hasSelection() && moveEnd) setCaret(std::min(m_Caret, m_CaretEnd));
        else moveLeft(word, moveEnd);
    }

    void TextView::doRight(bool word, bool moveEnd) {
        if (hasSelection() && moveEnd) setCaret(std::max(m_Caret, m_CaretEnd));
        else moveRight(word, moveEnd);
    }

    // ------------------------------------------------

    float TextView::stringWidth(std::string_view str) const {
        return settings.graphics.font.stringWidth(str);
    }

    std::int64_t TextView::yToLine(float y) const {
        auto v = paddedDimensions();
        auto localY = y - v.y() + m_Offset.y();
        return localY / settings.lineHeight;
    }

    std::int64_t TextView::xToIndex(std::string_view line, float x) const {
        auto v = paddedDimensions();
        auto localX = x - v.x() + m_Offset.x();
        auto width = 0.;
        for (std::int64_t i = 0; i < line.size(); ++i) {
            auto before = i > 0 ? line[i - 1] : '\0';
            auto c = line[i];
            auto after = i < line.size() - 1 ? line[i + 1] : '\0';

            width = stringWidth(line.substr(0, i + 1));
            auto cw = width - stringWidth(line.substr(0, i));
            if (localX <= width - cw * 0.5) return i;
        }

        return line.size();
    }

    std::int64_t TextView::positionToIndex(Point<int> pos) const {
        auto lines = this->lines();
        auto lineIndex = Math::Fast::clamp(yToLine(pos.y()), 0, lines.size() - 1);
        auto line = lines[lineIndex];
        auto indexInLine = xToIndex(line, pos.x());
        auto index = std::distance(settings.text.data(), line.data() + indexInLine);
        return index;
    }

    Point<int> TextView::indexToPosition(std::int64_t index) const {
        auto v = paddedDimensions();
        std::string_view string = settings.text;
        string = string.substr(0, findEndOfWordRightFrom(index));
        auto lines = this->linesFrom(string);

        const char* endOfLastLine = lines.back().data() + lines.back().size();
        std::int64_t indexOfEndOfLine = std::distance(string.data(), endOfLastLine);
        std::int64_t difference = indexOfEndOfLine - index;
        std::int64_t indexInLastLine = lines.back().size() - difference;
        std::string_view line = lines.back().substr(0, indexInLastLine);

        Coord y = (lines.size() - 1) * settings.lineHeight;
        Coord x = stringWidth(line);

        return { x + v.x() - m_Offset.x(), y + v.y() - m_Offset.y() };
    }

    // ------------------------------------------------

}
#include "Kaixo/Core/Theme/Image.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------
    
    void drawTiledImage(juce::Graphics& g, const juce::Image& img, const Rect<float>& pos, const TiledDescription& tiles, bool fillAlphaWithColor = false) {

        // ------------------------------------------------

        auto ox0 = 0;
        auto ox1 = tiles.left;
        auto ox2 = pos.width() - tiles.right;
        auto ix2 = img.getWidth() - tiles.right;

        auto oy0 = 0;
        auto oy1 = tiles.top;
        auto oy2 = pos.height() - tiles.bottom;
        auto iy2 = img.getHeight() - tiles.bottom;

        auto ow0 = Math::max(tiles.left, 0);
        auto ow1 = Math::max(pos.width() - tiles.left - tiles.right, 0);
        auto iw1 = Math::max(img.getWidth() - tiles.left - tiles.right, 0);
        auto ow2 = Math::max(tiles.right, 0);

        auto oh0 = Math::max(tiles.top, 0);
        auto oh1 = Math::max(pos.height() - tiles.top - tiles.bottom, 0);
        auto ih1 = Math::max(img.getHeight() - tiles.top - tiles.bottom, 0);
        auto oh2 = Math::max(tiles.bottom, 0);

        // ------------------------------------------------

        g.setOpacity(1.0);
        g.drawImage(img, pos.x() + ox0, pos.y() + oy0, ow0, oh0, ox0, oy0, ow0, oh0, fillAlphaWithColor); //    top left
        g.drawImage(img, pos.x() + ox0, pos.y() + oy1, ow0, oh1, ox0, oy1, ow0, ih1, fillAlphaWithColor); // middle left
        g.drawImage(img, pos.x() + ox0, pos.y() + oy2, ow0, oh2, ox0, iy2, ow0, oh2, fillAlphaWithColor); // bottom left

        g.drawImage(img, pos.x() + ox1, pos.y() + oy0, ow1, oh0, ox1, oy0, iw1, oh0, fillAlphaWithColor); //    top middle
        g.drawImage(img, pos.x() + ox1, pos.y() + oy1, ow1, oh1, ox1, oy1, iw1, ih1, fillAlphaWithColor); // middle middle
        g.drawImage(img, pos.x() + ox1, pos.y() + oy2, ow1, oh2, ox1, iy2, iw1, oh2, fillAlphaWithColor); // bottom middle

        g.drawImage(img, pos.x() + ox2, pos.y() + oy0, ow2, oh0, ix2, oy0, ow2, oh0, fillAlphaWithColor); //    top right
        g.drawImage(img, pos.x() + ox2, pos.y() + oy1, ow2, oh1, ix2, oy1, ow2, ih1, fillAlphaWithColor); // middle right
        g.drawImage(img, pos.x() + ox2, pos.y() + oy2, ow2, oh2, ix2, iy2, ow2, oh2, fillAlphaWithColor); // bottom right

        // ------------------------------------------------

    }

    // ------------------------------------------------

    juce::RectanglePlacement getPlacement(Align align) {

        // ------------------------------------------------

        std::size_t _placementI = juce::RectanglePlacement::doNotResize;

        // ------------------------------------------------

        if ((align & X) == Left) _placementI |= juce::RectanglePlacement::xLeft;
        if ((align & X) == Right) _placementI |= juce::RectanglePlacement::xRight;
        if ((align & X) == CenterX) _placementI |= juce::RectanglePlacement::xMid;

        // ------------------------------------------------

        if ((align & Y) == Top) _placementI |= juce::RectanglePlacement::yTop;
        if ((align & Y) == Bottom) _placementI |= juce::RectanglePlacement::yBottom;
        if ((align & Y) == CenterY) _placementI |= juce::RectanglePlacement::yMid;

        // ------------------------------------------------

        return _placementI;

        // ------------------------------------------------

    }

    // ------------------------------------------------

    Point<int> getSize(Rect<int>& clip, Rect<float>& position) {
        if (clip.width() == 0 && clip.height() == 0) return {
            static_cast<int>(position.width()),
            static_cast<int>(position.height())
        };
        else return { clip.width(), clip.height() };
    }

    // ------------------------------------------------

    Rect<int> getClippedArea(Rect<int>& clip, Rect<float>& position) {
        auto _size = getSize(clip, position);
        return { clip.x(), clip.y(), _size.x(), _size.y() };
    }

    // ------------------------------------------------

    void Image::draw(TiledInstruction i) const {
        auto _clipped = m_Image.getClippedImage(getClippedArea(i.clip, i.bounds));

        drawTiledImage(i.graphics, _clipped, i.bounds, i.description, i.fillAlphaWithColor);
    }

    void Image::draw(ClippedInstruction i) const {
        auto _clipped = m_Image.getClippedImage(getClippedArea(i.clip, i.bounds));

        if (i.tiled) {
            drawTiledImage(i.graphics, _clipped, i.bounds, *i.tiled, i.fillAlphaWithColor);
        } else {
            if (!i.fillAlphaWithColor) i.graphics.setOpacity(1.0);
            i.graphics.saveState(); // Save state before clip
            i.graphics.reduceClipRegion(i.bounds.getSmallestIntegerContainer());
            // TODO: add proper placement using align and position
            // alignment should be relative to position, which itself is relative to bounds
            i.graphics.drawImage(_clipped, i.bounds + i.position, getPlacement(i.align), i.fillAlphaWithColor);
            i.graphics.restoreState();
        }
    }
    
    void Image::draw(FrameInstruction i) const {

        int col = static_cast<int>(i.frame % i.description.framesPerRow);
        int row = static_cast<int>(i.frame / i.description.framesPerRow);

        auto _frame = Rect<int>{
            i.clip.width() * col + i.clip.x(),
            i.clip.height() * row + i.clip.y(),
            Math::min(i.clip.width(), static_cast<int>(i.position.width())),
            Math::min(i.clip.height(), static_cast<int>(i.position.height()))
        };

        auto _clipped = m_Image.getClippedImage(_frame);

        // Draw tiled frame
        if (i.tiled) {
            drawTiledImage(i.graphics, _clipped, i.bounds, *i.tiled, i.fillAlphaWithColor);
        } else {
            i.graphics.setOpacity(1.0);
            i.graphics.saveState(); // Save state before clip
            i.graphics.reduceClipRegion(i.bounds.getSmallestIntegerContainer());
            // TODO: add proper placement using align and position
            // alignment should be relative to position, which itself is relative to bounds
            i.graphics.drawImage(_clipped, i.bounds + i.position.position(), getPlacement(i.align), i.fillAlphaWithColor);
            i.graphics.restoreState();
        }

    }

    // ------------------------------------------------

}
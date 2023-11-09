#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    ImageView::ImageView(Context c, Settings s)
        : View(c), settings(std::move(s))
    {}

    // ------------------------------------------------

    void ImageView::paint(juce::Graphics& g) {
        settings.image.draw(g, localDimensions());
    }

    // ------------------------------------------------

}
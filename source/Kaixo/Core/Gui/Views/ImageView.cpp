#include "Kaixo/Core/Gui/Views/ImageView.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    ImageView::ImageView(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        if (!settings.enableMouse) {
            setInterceptsMouseClicks(false, false);
        }
    }

    // ------------------------------------------------

    void ImageView::paint(juce::Graphics& g) {
        settings.image.draw({
            .graphics = g,
            .bounds = localDimensions(),
            .text{ { "$text", settings.text } },
            .state = state()
        });
    }

    // ------------------------------------------------

}
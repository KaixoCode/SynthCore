#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class ImageView : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Theme::Drawable image;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        ImageView(Context c, Settings s = {});

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
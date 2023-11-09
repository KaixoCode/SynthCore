#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class ImageView : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Theme::Basic image;

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
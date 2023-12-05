#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"
#include "Kaixo/Core/Gui/Knob.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class PitchKnob : public Knob {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Knob::Settings knob;

            // ------------------------------------------------

            UnevaluatedRect signLocation     {             0, Y,            10, Height };
            UnevaluatedRect transposeLocation{            10, Y, Width / 2 - 4, Height };
            UnevaluatedRect detuneLocation   { Width / 2 + 4, Y, Width / 2 - 4, Height };

            // ------------------------------------------------

            Theme::Drawable sign;
            Theme::Drawable transpose;
            Theme::Drawable detune;

            // ------------------------------------------------

            int min = -48;
            int max = 48;

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        PitchKnob(Context c, Settings settings = {});

        // ------------------------------------------------

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
 
        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------
    protected:
        bool m_IsTranspose = false;

        // ------------------------------------------------

    };
}
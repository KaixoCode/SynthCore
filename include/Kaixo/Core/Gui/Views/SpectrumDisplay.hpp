#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Color.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Processing/Interface.hpp"
#include "Kaixo/Core/Processing/CircularBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SpectrumDisplay;

    // ------------------------------------------------
    
    class SpectrumInterface : public Processing::Interface {
    public:

        // ------------------------------------------------

        virtual Processing::Stereo read() = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

    class SpectrumDisplay : public View {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            Theme::Color fill;
            Theme::Color stroke;

            // ------------------------------------------------
            
            std::size_t fftSize = 4096;

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        SpectrumDisplay(Context c, Settings s = {}) 
            : View(c), settings(std::move(s)) 
        {}

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override {

        }

        // ------------------------------------------------

    private:

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
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
    
    class SpectrumInterface : public Processing::Interface {
    public:

        // ------------------------------------------------

        virtual const Processing::CircularBuffer& buffer() = 0;

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
            
            float mindB = -45;
            float maxdB = 20;

            // ------------------------------------------------
            
            std::size_t fftSize = 8192;

            // ------------------------------------------------
            
            Processing::InterfaceStorage<SpectrumInterface> interface;

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        SpectrumDisplay(Context c, Settings s = {});

        // ------------------------------------------------
        
        void onIdle() override;
        void paint(juce::Graphics& g) override;
 
        // ------------------------------------------------

    private:
        std::vector<std::complex<float>> m_Data{};
        std::vector<float> m_Levels{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
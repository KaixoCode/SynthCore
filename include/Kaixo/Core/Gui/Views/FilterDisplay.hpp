#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Views/PointsDisplay.hpp"

#include "Kaixo/Core/Processing/Filter.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class FilterDisplay : public PointsDisplay {
    public:

        // ------------------------------------------------

        struct Filter {

            // ------------------------------------------------

            ParamID frequency = NoParam;
            ParamID resonance = NoParam;
            ParamID gain      = NoParam;
            ParamID type      = NoParam;
            ParamID enable    = NoParam;

            // ------------------------------------------------
            
            ParamValue resetValueFrequency = 0.5;
            ParamValue resetValueResonance = 0.2;
            ParamValue resetValueGain = 0.5;
            ParamValue resetValueType = 0;
            ParamValue resetValueEnable = 1.0;

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        struct Settings {

            // ------------------------------------------------

            PointsDisplay::Settings pointsDisplay{};

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        FilterDisplay(Context c, Settings s = {});

        // ------------------------------------------------

        float at(float x) override;
        std::size_t nofPoints() const override;
        Point getPoint(std::size_t i) override;
        void setPoint(std::size_t i, Point point) override;
        void resetPoint(std::size_t i) override;

        // ------------------------------------------------
        
        void addFilter(Filter settings);

        // ------------------------------------------------
        
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

        // ------------------------------------------------
        
    private:
        struct Entry : ParameterListener {

            // ------------------------------------------------
            
            Entry(Filter s, FilterDisplay* f);
            ~Entry();

            // ------------------------------------------------

            Filter settings;
            FilterDisplay* self;

            // ------------------------------------------------

            Processing::Biquad<> filter{};

            // ------------------------------------------------
            
            ParamValue frequency = settings.resetValueFrequency;
            ParamValue resonance = settings.resetValueResonance;
            ParamValue gain = settings.resetValueGain;
            ParamValue type = settings.resetValueType;
            ParamValue enable = settings.resetValueEnable;

            // ------------------------------------------------

            void parameterChanged(ParamID id, ParamValue val) override;

            // ------------------------------------------------
        
            void reset();

            // ------------------------------------------------
        
            float x() const;
            float y() const;

            void addZ(ParamValue delta);

            void x(ParamValue v);
            void y(ParamValue v);

            // ------------------------------------------------
        
            float decibelsAt(float x);

            // ------------------------------------------------
            
            void setFrequency(ParamValue v);
            void setResonance(ParamValue v);
            void setGain(ParamValue v);
            void setType(ParamValue v);
            void setEnable(ParamValue v);

            // ------------------------------------------------

            void resetFrequency();
            void resetResonance();
            void resetGain();
            void resetType();
            void resetEnable();

            // ------------------------------------------------

        };

        std::vector<std::unique_ptr<Entry>> m_Filters{};

        // ------------------------------------------------

    };



    // ------------------------------------------------

}
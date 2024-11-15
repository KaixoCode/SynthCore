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
            ParamID passes    = NoParam;

            // ------------------------------------------------
            
            ParamValue resetValueFrequency = 0.5;
            ParamValue resetValueResonance = 0.2;
            ParamValue resetValueGain = 0.5;
            ParamValue resetValueType = 0;
            ParamValue resetValueEnable = 1.0;
            ParamValue resetValuePasses = 1;

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
        
        template<class FilterType>
        void addFilter(Filter settings);

        // ------------------------------------------------
        
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

        // ------------------------------------------------
        
    private:

        // ------------------------------------------------

        struct Entry : ParameterListener {

            // ------------------------------------------------

            virtual void reset() = 0;

            // ------------------------------------------------

            virtual float x() const = 0;
            virtual float y() const = 0;

            virtual void addZ(ParamValue delta) = 0;

            virtual void x(ParamValue v) = 0;
            virtual void y(ParamValue v) = 0;

            // ------------------------------------------------

            virtual float decibelsAt(float x) = 0;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        template<class FilterType>
        struct TypedEntry : Entry {

            // ------------------------------------------------
            
            TypedEntry(Filter s, FilterDisplay* f);
            ~TypedEntry();

            // ------------------------------------------------

            Filter settings;
            FilterDisplay* self;

            // ------------------------------------------------

            FilterType filter{};

            // ------------------------------------------------
            
            ParamValue frequency = settings.resetValueFrequency;
            ParamValue resonance = settings.resetValueResonance;
            ParamValue gain = settings.resetValueGain;
            ParamValue type = settings.resetValueType;
            ParamValue enable = settings.resetValueEnable;
            ParamValue passes = settings.resetValuePasses;

            // ------------------------------------------------

            void parameterChanged(ParamID id, ParamValue val) override;

            // ------------------------------------------------
        
            void reset() override;

            // ------------------------------------------------
        
            float x() const override;
            float y() const override;

            void addZ(ParamValue delta) override;

            void x(ParamValue v) override;
            void y(ParamValue v) override;

            // ------------------------------------------------
        
            void recalculate();

            // ------------------------------------------------
        
            float decibelsAt(float x) override;

            // ------------------------------------------------
            
            void setFrequency(ParamValue v);
            void setResonance(ParamValue v);
            void setGain(ParamValue v);
            void setType(ParamValue v);
            void setEnable(ParamValue v);
            void setPasses(ParamValue v);

            // ------------------------------------------------

            void resetFrequency();
            void resetResonance();
            void resetGain();
            void resetType();
            void resetEnable();
            void resetPasses();

            // ------------------------------------------------

        };

        std::vector<std::unique_ptr<Entry>> m_Filters{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::addFilter(Filter settings) {
        m_Filters.push_back(std::make_unique<TypedEntry<FilterType>>(std::move(settings), this));
    }

    // ------------------------------------------------
    
    template<class FilterType>
    FilterDisplay::TypedEntry<FilterType>::TypedEntry(Filter s, FilterDisplay* f)
        : settings(std::move(s)), self(f)
    {
        self->context.listener(this);
    }

    template<class FilterType>
    FilterDisplay::TypedEntry<FilterType>::~TypedEntry() {}

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::parameterChanged(ParamID id, ParamValue val) {
        bool _changed = false;

        if (id == settings.frequency)      frequency = val, _changed = true;
        else if (id == settings.resonance) resonance = val, _changed = true;
        else if (id == settings.gain)      gain = val,      _changed = true;
        else if (id == settings.type)      type = val,      _changed = true;
        else if (id == settings.enable)    enable = val,    _changed = true;
        else if (id == settings.passes)    passes = val,    _changed = true;

        if (_changed) {
            self->repaint();
        }
    }

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::reset() {
        resetFrequency();
        resetResonance();
        resetGain();
        resetType();
        resetEnable();
        resetPasses();
    }

    // ------------------------------------------------

    template<class FilterType>
    float FilterDisplay::TypedEntry<FilterType>::x() const { return frequency; }

    template<class FilterType>
    float FilterDisplay::TypedEntry<FilterType>::y() const {
        using enum Processing::FilterType;
        switch (filter.type()) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
        case Notch:
        case BandPass:
        case AllPass:
            return resonance;
        default:
            return gain;
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::addZ(ParamValue delta) {
        using enum Processing::FilterType;
        switch (filter.type()) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
        case Notch:
        case BandPass:
        case AllPass:
            break;
        default:
            setResonance(Math::clamp1(resonance + delta));
            break;
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::x(ParamValue v) { setFrequency(Math::clamp1(v)); }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::y(ParamValue v) {
        using enum Processing::FilterType;
        switch (filter.type()) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
        case Notch:
        case BandPass:
        case AllPass:
            setResonance(Math::clamp1(v));
            break;
        default:
            setGain(Math::clamp1(v));
            break;
        }
    }

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::recalculate() {
        filter.sampleRate(48000);

        if (settings.frequency != NoParam) {
            filter.frequency(parameter(settings.frequency).transform.transform(frequency));
        } else {
            filter.frequency(Math::magnitude_to_log<20.f, 20000.f>(frequency));
        }

        if (settings.gain != NoParam) {
            filter.gain(parameter(settings.gain).transform.transform(gain));
        } else {
            filter.gain(30 * gain - 15);
        }

        if (settings.passes != NoParam) {
            filter.passes(parameter(settings.passes).transform.transform(passes));
        } else {
            filter.passes(1);
        }

        filter.resonance(resonance);
        filter.type(type);
        filter.bypass = enable < 0.5;
    }

    // ------------------------------------------------

    template<class FilterType>
    float FilterDisplay::TypedEntry<FilterType>::decibelsAt(float x) {
        recalculate();
        if (filter.bypass) return 0;
        auto& coeff = filter.getCoefficients();
        float freq = settings.frequency == NoParam
            ? Math::Fast::magnitude_to_log<20.f, 20000.f>(x)
            : parameter(settings.frequency).transform.transform(x);
        float o2 = Math::powN<2>(Math::sin(std::numbers::pi * freq / 48000));
        float p1 = (coeff.b[0] + coeff.b[1] + coeff.b[2]) / 2;
        float p2 = (coeff.a[0] + coeff.a[1] + coeff.a[2]) / 2;
        float q1 = (4 * coeff.b[0] * coeff.b[2] * (1 - o2) + coeff.b[1] * (coeff.b[0] + coeff.b[2]));
        float q2 = (4 * coeff.a[0] * coeff.a[2] * (1 - o2) + coeff.a[1] * (coeff.a[0] + coeff.a[2]));
        float db = 10 * Math::log10((p1 * p1) - o2 * q1) - 10 * Math::log10((p2 * p2) - o2 * q2);
        return filter.passes() * db;
    }

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setFrequency(ParamValue v) {
        frequency = v;
        if (settings.frequency != NoParam) {
            self->context.beginEdit(settings.frequency);
            self->context.performEdit(settings.frequency, v);
            self->context.endEdit(settings.frequency);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setResonance(ParamValue v) {
        resonance = v;
        if (settings.resonance != NoParam) {
            self->context.beginEdit(settings.resonance);
            self->context.performEdit(settings.resonance, v);
            self->context.endEdit(settings.resonance);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setGain(ParamValue v) {
        gain = v;
        if (settings.gain != NoParam) {
            self->context.beginEdit(settings.gain);
            self->context.performEdit(settings.gain, v);
            self->context.endEdit(settings.gain);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setType(ParamValue v) {
        type = v;
        if (settings.type != NoParam) {
            self->context.beginEdit(settings.type);
            self->context.performEdit(settings.type, v);
            self->context.endEdit(settings.type);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setEnable(ParamValue v) {
        enable = v;
        if (settings.enable != NoParam) {
            self->context.beginEdit(settings.enable);
            self->context.performEdit(settings.enable, v);
            self->context.endEdit(settings.enable);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::setPasses(ParamValue v) {
        passes = v;
        if (settings.enable != NoParam) {
            self->context.beginEdit(settings.passes);
            self->context.performEdit(settings.passes, v);
            self->context.endEdit(settings.passes);
        }
    }

    // ------------------------------------------------

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetFrequency() {
        if (settings.frequency == NoParam) {
            frequency = settings.resetValueFrequency;
        } else {
            self->context.beginEdit(settings.frequency);
            self->context.performEdit(settings.frequency, self->context.defaultValue(settings.frequency));
            self->context.endEdit(settings.frequency);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetResonance() {
        if (settings.resonance == NoParam) {
            resonance = settings.resetValueResonance;
        } else {
            self->context.beginEdit(settings.resonance);
            self->context.performEdit(settings.resonance, self->context.defaultValue(settings.resonance));
            self->context.endEdit(settings.resonance);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetGain() {
        if (settings.gain == NoParam) {
            gain = settings.resetValueGain;
        } else {
            self->context.beginEdit(settings.gain);
            self->context.performEdit(settings.gain, self->context.defaultValue(settings.gain));
            self->context.endEdit(settings.gain);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetType() {
        if (settings.type == NoParam) {
            type = settings.resetValueType;
        } else {
            self->context.beginEdit(settings.type);
            self->context.performEdit(settings.type, self->context.defaultValue(settings.type));
            self->context.endEdit(settings.type);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetEnable() {
        if (settings.enable == NoParam) {
            enable = settings.resetValueEnable;
        } else {
            self->context.beginEdit(settings.enable);
            self->context.performEdit(settings.enable, self->context.defaultValue(settings.enable));
            self->context.endEdit(settings.enable);
        }
    }

    template<class FilterType>
    void FilterDisplay::TypedEntry<FilterType>::resetPasses() {
        if (settings.passes == NoParam) {
            passes = settings.resetValuePasses;
        } else {
            self->context.beginEdit(settings.passes);
            self->context.performEdit(settings.passes, self->context.defaultValue(settings.passes));
            self->context.endEdit(settings.passes);
        }
    }

    // ------------------------------------------------

    // ------------------------------------------------

}
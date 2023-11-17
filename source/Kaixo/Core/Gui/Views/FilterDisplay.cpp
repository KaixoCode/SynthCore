#include "Kaixo/Core/Gui/Views/FilterDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    FilterDisplay::FilterDisplay(Context c, Settings s)
        : PointsDisplay(c, std::move(s.pointsDisplay)), settings(std::move(s))
    {
        PointsDisplay::settings.line.start = Height;
        PointsDisplay::settings.line.end = Height;
        PointsDisplay::settings.line.loop = false;
        PointsDisplay::settings.enableAddPoints = false;
        PointsDisplay::settings.keepPointsInOrder = false;
        PointsDisplay::settings.hasCurves = false;
        PointsDisplay::settings.grid.x = 0;
        PointsDisplay::settings.grid.y = 0;
    }

    // ------------------------------------------------

    float FilterDisplay::at(float x) {
        float decibels = 0;
        for (auto& filter : m_Filters)
            decibels += filter->decibelsAt(x);
        if (std::isnan(decibels)) return -1;
        if (std::isinf(decibels)) return -1;
        return decibels / 36 + 0.5;
    }

    std::size_t FilterDisplay::nofPoints() const { return m_Filters.size(); }

    FilterDisplay::Point FilterDisplay::getPoint(std::size_t i) {
        return {
            .x = m_Filters[i]->x(),
            .y = m_Filters[i]->y(),
        };
    }

    void FilterDisplay::setPoint(std::size_t i, Point point) {
        m_Filters[i]->x(point.x);
        m_Filters[i]->y(point.y);
    }

    void FilterDisplay::resetPoint(std::size_t i) {
        m_Filters[i]->reset();
    }

    // ------------------------------------------------

    void FilterDisplay::addFilter(Filter settings) {
        m_Filters.push_back(std::make_unique<Entry>(std::move(settings), this));
    }

    // ------------------------------------------------

    void FilterDisplay::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        if (hoveringPoint() != npos) {
            float mult = 1;
            if (event.mods.isCtrlDown()) mult *= 0.25;
            if (event.mods.isShiftDown()) mult *= 0.25;
            m_Filters[hoveringPoint()]->addZ(d.deltaY * mult * 0.1);
        }
    }

    // ------------------------------------------------

    FilterDisplay::Entry::Entry(Filter s, FilterDisplay* f)
        : settings(std::move(s)), self(f)
    {
        self->context.listener(this);
    }

    FilterDisplay::Entry::~Entry() {}

    // ------------------------------------------------

    void FilterDisplay::Entry::parameterChanged(ParamID id, ParamValue val) {
        bool _changed = false;

        if (id == settings.frequency)      frequency = val, _changed = true;
        else if (id == settings.resonance) resonance = val, _changed = true;
        else if (id == settings.gain)      gain = val,      _changed = true;
        else if (id == settings.type)      type = val,      _changed = true;
        else if (id == settings.enable)    enable = val,    _changed = true;

        if (_changed) {
            self->repaint();
        }
    }

    // ------------------------------------------------

    void FilterDisplay::Entry::reset() {
        resetFrequency();
        resetResonance();
        resetGain();
        resetType();
        resetEnable();
    }

    // ------------------------------------------------

    float FilterDisplay::Entry::x() const { return frequency; }
    float FilterDisplay::Entry::y() const {
        using enum Processing::FilterType;
        switch (Processing::eqParamTypeToFilterType(type)) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
            return resonance;
        default:
            return gain;
        }
    }

    void FilterDisplay::Entry::addZ(ParamValue delta) {
        using enum Processing::FilterType;
        switch (Processing::eqParamTypeToFilterType(type)) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
            break;
        default:
            setResonance(Math::clamp1(resonance + delta));
            break;
        }
    }

    void FilterDisplay::Entry::x(ParamValue v) { setFrequency(Math::clamp1(v)); }
    void FilterDisplay::Entry::y(ParamValue v) {
        using enum Processing::FilterType;
        switch (Processing::eqParamTypeToFilterType(type)) {
        case LowPass:
        case LowPass4:
        case HighPass:
        case HighPass4:
            setResonance(Math::clamp1(v));
            break;
        default:
            setGain(Math::clamp1(v));
            break;
        }
    }

    // ------------------------------------------------

    float FilterDisplay::Entry::decibelsAt(float x) {
        filter.sampleRate(48000);
        filter.frequency(Math::Fast::magnitude_to_log<20.f, 20000.f>(frequency));
        filter.resonance(resonance);
        filter.gain(gain * 30 - 15);
        filter.type(type);
        filter.bypass = enable < 0.5;

        auto& coeff = filter.getCoefficients();
        float freq = Math::Fast::magnitude_to_log<20.f, 20000.f>(x);
        float o2 = Math::powN<2>(Math::sin(std::numbers::pi * freq / 48000));
        float p1 = (coeff.b[0] + coeff.b[1] + coeff.b[2]) / 2;
        float p2 = (coeff.a[0] + coeff.a[1] + coeff.a[2]) / 2;
        float q1 = (4 * coeff.b[0] * coeff.b[2] * (1 - o2) + coeff.b[1] * (coeff.b[0] + coeff.b[2]));
        float q2 = (4 * coeff.a[0] * coeff.a[2] * (1 - o2) + coeff.a[1] * (coeff.a[0] + coeff.a[2]));
        float db = 10 * Math::log10((p1 * p1) - o2 * q1) - 10 * Math::log10((p2 * p2) - o2 * q2);
        return filter.quadruple() ? 4 * db : db;
    }

    // ------------------------------------------------

    void FilterDisplay::Entry::setFrequency(ParamValue v) {
        frequency = v;
        if (settings.frequency != NoParam) {
            self->context.beginEdit(settings.frequency);
            self->context.performEdit(settings.frequency, v);
            self->context.endEdit(settings.frequency);
        }
    }

    void FilterDisplay::Entry::setResonance(ParamValue v) {
        resonance = v;
        if (settings.resonance != NoParam) {
            self->context.beginEdit(settings.resonance);
            self->context.performEdit(settings.resonance, v);
            self->context.endEdit(settings.resonance);
        }
    }

    void FilterDisplay::Entry::setGain(ParamValue v) {
        gain = v;
        if (settings.gain != NoParam) {
            self->context.beginEdit(settings.gain);
            self->context.performEdit(settings.gain, v);
            self->context.endEdit(settings.gain);
        }
    }

    void FilterDisplay::Entry::setType(ParamValue v) {
        type = v;
        if (settings.type != NoParam) {
            self->context.beginEdit(settings.type);
            self->context.performEdit(settings.type, v);
            self->context.endEdit(settings.type);
        }
    }

    void FilterDisplay::Entry::setEnable(ParamValue v) {
        enable = v;
        if (settings.enable != NoParam) {
            self->context.beginEdit(settings.enable);
            self->context.performEdit(settings.enable, v);
            self->context.endEdit(settings.enable);
        }
    }

    // ------------------------------------------------

    void FilterDisplay::Entry::resetFrequency() {
        if (settings.frequency == NoParam) {
            frequency = settings.resetValueFrequency;
        } else {
            self->context.beginEdit(settings.frequency);
            self->context.performEdit(settings.frequency, self->context.defaultValue(settings.frequency));
            self->context.endEdit(settings.frequency);
        }
    }

    void FilterDisplay::Entry::resetResonance() {
        if (settings.resonance == NoParam) {
            resonance = settings.resetValueResonance;
        } else {
            self->context.beginEdit(settings.resonance);
            self->context.performEdit(settings.resonance, self->context.defaultValue(settings.resonance));
            self->context.endEdit(settings.resonance);
        }
    }

    void FilterDisplay::Entry::resetGain() {
        if (settings.gain == NoParam) {
            gain = settings.resetValueGain;
        } else {
            self->context.beginEdit(settings.gain);
            self->context.performEdit(settings.gain, self->context.defaultValue(settings.gain));
            self->context.endEdit(settings.gain);
        }
    }

    void FilterDisplay::Entry::resetType() {
        if (settings.type == NoParam) {
            type = settings.resetValueType;
        } else {
            self->context.beginEdit(settings.type);
            self->context.performEdit(settings.type, self->context.defaultValue(settings.type));
            self->context.endEdit(settings.type);
        }
    }

    void FilterDisplay::Entry::resetEnable() {
        if (settings.enable == NoParam) {
            enable = settings.resetValueEnable;
        } else {
            self->context.beginEdit(settings.enable);
            self->context.performEdit(settings.enable, self->context.defaultValue(settings.enable));
            self->context.endEdit(settings.enable);
        }
    }

    // ------------------------------------------------

}
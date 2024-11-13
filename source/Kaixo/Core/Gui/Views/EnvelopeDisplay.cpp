#include "Kaixo/Core/Gui/Views/EnvelopeDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    EnvelopeDisplay::EnvelopeDisplay(Context c, Settings s)
        : PointsDisplay(c, std::move(s.pointsDisplay)), settings(std::move(s))
    {
        PointsDisplay::settings.enableAddPoints = false;
        PointsDisplay::settings.keepPointsInOrder = false;
        PointsDisplay::settings.line.loop = false;
        PointsDisplay::settings.line.start = Height;
        PointsDisplay::settings.line.end = Height;
        PointsDisplay::settings.grid.x = 0;
        PointsDisplay::settings.grid.y = 0;
        
        if (settings.phase) {
            PointsDisplay::settings.phase.display = true;
        }
    }

    // ------------------------------------------------

    void EnvelopeDisplay::parameterChanged(ParamID id, ParamValue val) {
        bool _changed = false;

        if      (id == settings.delay)        m_Delay = val,        _changed = true;
        else if (id == settings.attack)       m_Attack = val,       _changed = true;
        else if (id == settings.decay)        m_Decay = val,        _changed = true;
        else if (id == settings.attackLevel)  m_AttackLevel = val,  _changed = true;
        else if (id == settings.decayLevel)   m_DecayLevel = val,   _changed = true;
        else if (id == settings.sustain)      m_Sustain = val,      _changed = true;
        else if (id == settings.release)      m_Release = val,      _changed = true;
        else if (id == settings.attackCurve)  m_AttackCurve = val,  _changed = true;
        else if (id == settings.decayCurve)   m_DecayCurve = val,   _changed = true;
        else if (id == settings.releaseCurve) m_ReleaseCurve = val, _changed = true;

        if (_changed) {
            repaint();
        }
    }

    // ------------------------------------------------

    void EnvelopeDisplay::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) {
        float mult = settings.zoomSpeed;
        if (event.mods.isCtrlDown()) mult *= 0.5;
        if (event.mods.isShiftDown()) mult *= 0.5;
        settings.zoom = Math::clamp(settings.zoom + d.deltaY * mult, settings.minZoom, settings.maxZoom);

        repaint();
    }

    // ------------------------------------------------

    float EnvelopeDisplay::at(float x) {
        float totalProgress = x * multiplier();
        float maxCurve = PointsDisplay::settings.maxCurve;

        if (totalProgress <= m_Delay) {
            float progress = totalProgress / m_Delay;
            return m_AttackLevel;
        } else if (totalProgress <= m_Delay + m_Attack) {
            float progress = (totalProgress - m_Delay) / m_Attack;
            return m_AttackLevel + (m_DecayLevel - m_AttackLevel) * Math::curve(progress, m_AttackCurve * 2 * maxCurve - maxCurve);
        } else if (totalProgress <= m_Delay + m_Attack + m_Decay) {
            float progress = (totalProgress - m_Delay - m_Attack) / m_Decay;
            return m_DecayLevel - (m_DecayLevel - m_Sustain) * Math::curve(progress, m_DecayCurve * 2 * maxCurve - maxCurve);
        } else if (totalProgress <= m_Delay + m_Attack + m_Decay + m_Release) {
            float progress = (totalProgress - m_Delay - m_Attack - m_Decay) / m_Release;
            return m_Sustain - m_Sustain * Math::curve(progress, m_ReleaseCurve * 2 * maxCurve - maxCurve);
        } else {
            return 0;
        }
    }

    PointsDisplay::Point EnvelopeDisplay::getPoint(std::size_t i) {
        float maxCurve = PointsDisplay::settings.maxCurve;
        switch (i) {
        case 0: return { .x = (m_Delay) / multiplier(), .y = m_AttackLevel, .c = m_AttackCurve * 2 * maxCurve - maxCurve };
        case 1: return { .x = (m_Delay + m_Attack) / multiplier(), .y = m_DecayLevel, .c = m_DecayCurve * 2 * maxCurve - maxCurve };
        case 2: return { .x = (m_Delay + m_Attack + m_Decay) / multiplier(), .y = m_Sustain, .c = m_ReleaseCurve * 2 * maxCurve - maxCurve };
        case 3: return { .x = (m_Delay + m_Attack + m_Decay + m_Release) / multiplier(), .y = 0, .c = 0 };
        }
    }

    void EnvelopeDisplay::setPoint(std::size_t i, Point point) {
        float maxCurve = PointsDisplay::settings.maxCurve;
        switch (i) {
        case 0:
            setDelay(Math::clamp1(point.x * multiplier()));
            setAttackCurve(point.c / (2 * maxCurve) + 0.5);
            setAttackLevel(point.y);
            break;
        case 1:
            setAttack(Math::clamp1(point.x * multiplier() - m_Delay));
            setDecayCurve(point.c / (2 * maxCurve) + 0.5);
            setDecayLevel(point.y);
            break;
        case 2:
            setDecay(Math::clamp1(point.x * multiplier() - m_Delay - m_Attack));
            setReleaseCurve(point.c / (2 * maxCurve) + 0.5);
            setSustain(point.y);
            break;
        case 3:
            setRelease(Math::clamp1(point.x * multiplier() - m_Delay - m_Attack - m_Decay));
            break;
        }
    }

    void EnvelopeDisplay::resetCurve(std::size_t i) {
        switch (i) {
        case 0:
            resetAttackCurve();
            break;
        case 1:
            resetDecayCurve();
            break;
        case 2:
            resetReleaseCurve();
            break;
        case 3:
            break;
        }
    }

    void EnvelopeDisplay::resetPoint(std::size_t i) {
        switch (i) {
        case 0:
            resetDelay();
            resetAttackLevel();
            break;
        case 1:
            resetAttack();
            resetDecayLevel();
            break;
        case 2:
            resetDecay();
            resetSustain();
            break;
        case 3:
            resetRelease();
            break;
        }
    }

    float EnvelopeDisplay::phase() {
        if (settings.phase) {
            auto phase = settings.phase();
            if (phase < 1) return ((phase - 0) * m_Delay) / multiplier();
            if (phase < 2) return (m_Delay + (phase - 1) * m_Attack) / multiplier();
            if (phase < 3) return (m_Delay + m_Attack + (phase - 2) * m_Decay) / multiplier();
            if (phase < 4) return (m_Delay + m_Attack + m_Decay + (phase - 3) * m_Release) / multiplier();
            return phase;
        }
        return 0;
    }

    // ------------------------------------------------

    void EnvelopeDisplay::setDelay(ParamValue v) {
        m_Delay = v;
        if (settings.delay != NoParam) {
            context.beginEdit(settings.delay);
            context.performEdit(settings.delay, v);
            context.endEdit(settings.delay);
        }
    }

    void EnvelopeDisplay::setAttack(ParamValue v) {
        m_Attack = v;
        if (settings.attack != NoParam) {
            context.beginEdit(settings.attack);
            context.performEdit(settings.attack, v);
            context.endEdit(settings.attack);
        }
    }

    void EnvelopeDisplay::setDecay(ParamValue v) {
        m_Decay = v;
        if (settings.decay != NoParam) {
            context.beginEdit(settings.decay);
            context.performEdit(settings.decay, v);
            context.endEdit(settings.decay);
        }
    }

    void EnvelopeDisplay::setRelease(ParamValue v) {
        m_Release = v;
        if (settings.release != NoParam) {
            context.beginEdit(settings.release);
            context.performEdit(settings.release, v);
            context.endEdit(settings.release);
        }
    }

    void EnvelopeDisplay::setAttackLevel(ParamValue v) {
        m_AttackLevel = v;
        if (settings.attackLevel != NoParam) {
            context.beginEdit(settings.attackLevel);
            context.performEdit(settings.attackLevel, v);
            context.endEdit(settings.attackLevel);
        }
    }

    void EnvelopeDisplay::setDecayLevel(ParamValue v) {
        m_DecayLevel = v;
        if (settings.decayLevel != NoParam) {
            context.beginEdit(settings.decayLevel);
            context.performEdit(settings.decayLevel, v);
            context.endEdit(settings.decayLevel);
        }
    }

    void EnvelopeDisplay::setSustain(ParamValue v) {
        m_Sustain = v;
        if (settings.sustain != NoParam) {
            context.beginEdit(settings.sustain);
            context.performEdit(settings.sustain, v);
            context.endEdit(settings.sustain);
        }
    }

    void EnvelopeDisplay::setAttackCurve(ParamValue v) {
        m_AttackCurve = v;
        if (settings.attackCurve != NoParam) {
            context.beginEdit(settings.attackCurve);
            context.performEdit(settings.attackCurve, v);
            context.endEdit(settings.attackCurve);
        }
    }

    void EnvelopeDisplay::setDecayCurve(ParamValue v) {
        m_DecayCurve = v;
        if (settings.decayCurve != NoParam) {
            context.beginEdit(settings.decayCurve);
            context.performEdit(settings.decayCurve, v);
            context.endEdit(settings.decayCurve);
        }
    }

    void EnvelopeDisplay::setReleaseCurve(ParamValue v) {
        m_ReleaseCurve = v;
        if (settings.releaseCurve != NoParam) {
            context.beginEdit(settings.releaseCurve);
            context.performEdit(settings.releaseCurve, v);
            context.endEdit(settings.releaseCurve);
        }
    }

    // ------------------------------------------------

    void EnvelopeDisplay::resetDelay() {
        if (settings.delay == NoParam) {
            m_Delay = settings.resetValueDelay;
        } else {
            context.beginEdit(settings.delay);
            context.performEdit(settings.delay, context.defaultValue(settings.delay));
            context.endEdit(settings.delay);
        }
    }

    void EnvelopeDisplay::resetAttack() {
        if (settings.attack == NoParam) {
            m_Attack = settings.resetValueAttack;
        } else {
            context.beginEdit(settings.attack);
            context.performEdit(settings.attack, context.defaultValue(settings.attack));
            context.endEdit(settings.attack);
        }
    }

    void EnvelopeDisplay::resetDecay() {
        if (settings.decay == NoParam) {
            m_Decay = settings.resetValueDecay;
        } else {
            context.beginEdit(settings.decay);
            context.performEdit(settings.decay, context.defaultValue(settings.decay));
            context.endEdit(settings.decay);
        }
    }

    void EnvelopeDisplay::resetRelease() {
        if (settings.release == NoParam) {
            m_Release = settings.resetValueRelease;
        } else {
            context.beginEdit(settings.release);
            context.performEdit(settings.release, context.defaultValue(settings.release));
            context.endEdit(settings.release);
        }
    }

    void EnvelopeDisplay::resetAttackLevel() {
        if (settings.attackLevel == NoParam) {
            m_AttackLevel = settings.resetValueAttackLevel;
        } else {
            context.beginEdit(settings.attackLevel);
            context.performEdit(settings.attackLevel, context.defaultValue(settings.attackLevel));
            context.endEdit(settings.attackLevel);
        }
    }

    void EnvelopeDisplay::resetDecayLevel() {
        if (settings.decayLevel == NoParam) {
            m_DecayLevel = settings.resetValueDecayLevel;
        } else {
            context.beginEdit(settings.decayLevel);
            context.performEdit(settings.decayLevel, context.defaultValue(settings.decayLevel));
            context.endEdit(settings.decayLevel);
        }
    }

    void EnvelopeDisplay::resetSustain() {
        if (settings.sustain == NoParam) {
            m_Sustain = settings.resetValueSustain;
        } else {
            context.beginEdit(settings.sustain);
            context.performEdit(settings.sustain, context.defaultValue(settings.sustain));
            context.endEdit(settings.sustain);
        }
    }

    void EnvelopeDisplay::resetAttackCurve() {
        if (settings.attackCurve == NoParam) {
            m_AttackCurve = settings.resetValueAttackCurve;
        } else {
            context.beginEdit(settings.attackCurve);
            context.performEdit(settings.attackCurve, context.defaultValue(settings.attackCurve));
            context.endEdit(settings.attackCurve);
        }
    }

    void EnvelopeDisplay::resetDecayCurve() {
        if (settings.decayCurve == NoParam) {
            m_DecayCurve = settings.resetValueDecayCurve;
        } else {
            context.beginEdit(settings.decayCurve);
            context.performEdit(settings.decayCurve, context.defaultValue(settings.decayCurve));
            context.endEdit(settings.decayCurve);
        }
    }

    void EnvelopeDisplay::resetReleaseCurve() {
        if (settings.releaseCurve == NoParam) {
            m_ReleaseCurve = settings.resetValueReleaseCurve;
        } else {
            context.beginEdit(settings.releaseCurve);
            context.performEdit(settings.releaseCurve, context.defaultValue(settings.releaseCurve));
            context.endEdit(settings.releaseCurve);
        }
    }

    // ------------------------------------------------

}
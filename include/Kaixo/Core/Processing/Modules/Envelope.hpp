#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Envelope : public Module {
    public:

        // ------------------------------------------------

        enum class State { Idle, Delay, Attack, Decay, Sustain, Release, Amount };
        enum class Mode { Normal, Trigger, Loop };

        // ------------------------------------------------

        float output = 0;

        // ------------------------------------------------

        void delay(float millis) {
            if (m_DelayMillis != millis) {
                m_DelayMillis = millis;
                updateDelay();
            }
        }

        void attack(float millis) {
            if (m_AttackMillis != millis) {
                m_AttackMillis = millis;
                updateAttack();
            }
        }

        void decay(float millis) {
            if (m_DecayMillis != millis) {
                m_DecayMillis = millis;
                updateDecay();
            }
        }

        void release(float millis) {
            if (m_ReleaseMillis != millis) {
                m_ReleaseMillis = millis;
                updateRelease();
            }
        }

        void attackLevel(float level) { m_AttackLevel = level; }
        void decayLevel(float level) { m_DecayLevel = level; }
        void sustain(float level) { m_Sustain = level; }

        void attackCurve(float curve) { m_AttackCurve = curve; }
        void decayCurve(float curve) { m_DecayCurve = curve; }
        void releaseCurve(float curve) { m_ReleaseCurve = curve; }

        void mode(Mode mode) { m_Mode = mode; }

        // ------------------------------------------------

        bool idle() const { return m_State == State::Idle; }
        bool active() const override { return !idle(); }

        // ------------------------------------------------

        void gate(bool gate, bool retrigger = false) {
            if (gate) trigger(retrigger);
            else release();
        }

        void release() {
            if (m_State != State::Idle) {
                m_State = State::Release;
                m_Phase = 0;
                m_ReleaseValue = output;
            }
        }

        void trigger(bool retrigger = false) {
            if (retrigger && !idle()) {
                m_State = State::Attack;
                m_Phase = 0;
                m_AttackValue = output;
            } else {
                // Special case when no delay and no attack, immediate
                // set it to the decay value.
                if (m_DelayMillis <= 1 && m_AttackMillis <= 1) {
                    m_State = State::Decay;
                    m_Phase = 0;
                    output = m_DecayLevel;
                } else {
                    m_State = State::Delay;
                    m_Phase = 0;
                    m_AttackValue = m_AttackLevel;
                    output = m_AttackLevel;
                }
            }
        }

        // ------------------------------------------------

        void process() override {
            switch (m_State) {
            case State::Idle: output = 0; break;
            case State::Sustain: output = m_Sustain; break;
            case State::Delay:
                m_Phase += 1 / m_Delay;
                if (m_Phase >= 1.0) {
                    output = m_AttackValue;
                    m_Phase = 0;
                    m_State = State::Attack;
                }
                else {
                    output = m_AttackValue;
                }
                break;
            case State::Attack:
                m_Phase += 1 / m_Attack;
                if (m_Phase >= 1.0) {
                    output = m_DecayLevel;
                    m_Phase = 0;
                    m_State = State::Decay;
                }
                else {
                    output = m_AttackValue + (m_DecayLevel - m_AttackValue) * Math::Fast::curve(m_Phase, m_AttackCurve);
                }
                break;
            case State::Decay:
                m_Phase += 1 / m_Decay;
                if (m_Phase >= 1.0) {
                    output = m_Sustain;
                    m_Phase = 0;
                    switch (m_Mode) {
                    case Mode::Trigger: m_State = State::Release; break;
                    case Mode::Loop:    m_State = State::Attack;  break;
                    default:            m_State = State::Sustain; break;
                    }
                }
                else {
                    output = m_DecayLevel - (m_DecayLevel - m_Sustain) * Math::Fast::curve(m_Phase, m_DecayCurve);
                }
                break;
            case State::Release:
                m_Phase += 1 / m_Release;
                if (m_Phase >= 1.0) {
                    output = 0;
                    m_Phase = 0;
                    m_State = State::Idle;
                }
                else {
                    output = m_ReleaseValue - m_ReleaseValue * Math::Fast::curve(m_Phase, m_ReleaseCurve);
                }
                break;
            }
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            Module::prepare(sampleRate, maxBufferSize);

            updateDelay();
            updateAttack();
            updateDecay();
            updateRelease();
        }

        void reset() override {
            Module::reset();

            m_State = State::Idle;
            m_Phase = 0;
            output = 0;
        }

        // ------------------------------------------------

        float phase() const { 
            switch (m_State) {
            case State::Delay:   return m_Phase;
            case State::Attack:  return m_Phase + 1;
            case State::Decay:   return m_Phase + 2;
            case State::Sustain: 
            case State::Release: return m_Phase + 3;
            default: return 0;
            }
        }

        // ------------------------------------------------

    private:
        Mode m_Mode = Mode::Normal;
        State m_State = State::Idle;

        float m_DelayMillis = 0;
        float m_AttackMillis = 1;
        float m_DecayMillis = 60;
        float m_ReleaseMillis = 50;

        float m_Delay = 0;
        float m_Attack = 0;
        float m_Decay = 0;
        float m_Release = 0;

        float m_AttackLevel = 0;
        float m_DecayLevel = 1;
        float m_Sustain = 0.5;
        
        float m_AttackCurve = 0;
        float m_DecayCurve = 0;
        float m_ReleaseCurve = 0;

        float m_Phase = 0;
        float m_ReleaseValue = 0;
        float m_AttackValue = 0;

        // ------------------------------------------------
        
        void updateDelay()   { m_Delay   = 0.001 * m_DelayMillis   * sampleRate(); }
        void updateAttack()  { m_Attack  = 0.001 * m_AttackMillis  * sampleRate(); }
        void updateDecay()   { m_Decay   = 0.001 * m_DecayMillis   * sampleRate(); }
        void updateRelease() { m_Release = 0.001 * m_ReleaseMillis * sampleRate(); }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
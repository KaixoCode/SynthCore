#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Lfo : public Module {
    public:

        // ------------------------------------------------

        struct Point {
            float x;
            float y;
            float c;
        };

        // ------------------------------------------------

        class Storage : public Vector<Point, 100> {
        public:
            constexpr static Point Zero{ 0, 0, 0 };

            // ------------------------------------------------
            
            float at(float x) const {
                const Point* prev = empty() ? &Zero : &front();
                for (std::size_t i = 0; i < size(); ++i) {
                    auto& point = operator[](i);
                    if (point.x >= x) {
                        if ((point.x - prev->x) == 0) return point.y;
                        float r = Math::Fast::curve((x - prev->x) / (point.x - prev->x), prev->c);
                        return r * point.y + (1 - r) * prev->y;
                    }

                    prev = &point;
                }

                auto& point = empty() ? Zero : front();

                if ((1 - prev->x) == 0) return point.y;

                float r = Math::Fast::curve((x - prev->x) / (1 - prev->x), prev->c);
                return r * point.y + (1 - r) * prev->y;
            }

            // ------------------------------------------------

        };

        // ------------------------------------------------

        enum class Mode { Trigger, Sync, Envelope, Sustain, LoopPoint, LoopHold, Amount };
        enum class Sync { Seconds, Tempo, Amount };
        enum class Tempo { Freeze, T32_1, T16_1, T8_1, T4_1, T2_1, T1_1, T1_2, T1_4, T1_8, T1_16, T1_32, T1_64, Amount };

        constexpr static float bars(Tempo tempo) {
            switch (tempo) {
            case Tempo::Freeze: return 0.f;
            case Tempo::T32_1:  return 32.f;
            case Tempo::T16_1:  return 16.f;
            case Tempo::T8_1:   return 8.f;
            case Tempo::T4_1:   return 4.f;
            case Tempo::T2_1:   return 2.f;
            case Tempo::T1_1:   return 1.f;
            case Tempo::T1_2:   return 0.5f;
            case Tempo::T1_4:   return 0.25f;
            case Tempo::T1_8:   return 0.125f;
            case Tempo::T1_16:  return 0.0625f;
            case Tempo::T1_32:  return 0.03125f;
            case Tempo::T1_64:  return 0.015625f;
            }
        };

        // ------------------------------------------------

        float output = 0;

        // ------------------------------------------------

        void gate(bool gate, bool retrigger = false) {
            if (gate) trigger(retrigger);
            else release();
        }

        void release() { m_Gate = false; }

        void trigger(bool retrigger = false) {
            switch (m_Mode) {
            case Mode::Sync:
                m_Phase = m_PhaseIncremented = Math::Fast::fmod1(m_PhaseOffset + synchronizedPhase());
                break;
            case Mode::Trigger:
            case Mode::Envelope:
                m_Phase = m_PhaseIncremented = m_PhaseOffset;
                break;
            case Mode::Sustain:
            case Mode::LoopPoint:
            case Mode::LoopHold:
                if (!retrigger) {
                    m_Phase = m_PhaseIncremented = 0;
                }
                break;
            }
            m_Gate = true;
            m_FirstPhase = true;
        }

        // ------------------------------------------------

        void process() override {
            // When frozen, always at phaseOffset
            if (m_Tempo == Tempo::Freeze && m_Sync == Sync::Tempo) {
                m_Phase = m_PhaseOffset;
                output = at(m_Phase);
                return;
            }

            output = at(m_Phase);

            float deltaPhase = 1. / samplesPerOscillation();

            switch (m_Mode) {
            case Mode::Sync:
                m_Phase = m_PhaseIncremented = Math::Fast::fmod1(m_PhaseIncremented + deltaPhase);
                break;
            case Mode::Trigger:
                m_Phase = m_PhaseIncremented = Math::Fast::fmod1(m_PhaseIncremented + deltaPhase);
                break;
            case Mode::Envelope:
                m_Phase = m_PhaseIncremented = Math::Fast::min(m_PhaseIncremented + deltaPhase, 1.0);
                break;
            case Mode::Sustain:
                if (m_Gate) {
                    m_Phase = m_PhaseIncremented = Math::Fast::min(m_PhaseIncremented + deltaPhase, m_PhaseOffset);
                } else {
                    m_Phase = m_PhaseIncremented = Math::Fast::min(m_PhaseIncremented + deltaPhase, 1.0);
                }
                break;
            case Mode::LoopPoint:
                if (m_FirstPhase) {
                    float p = m_PhaseIncremented + deltaPhase;
                    if (p >= 1) m_FirstPhase = false;
                    m_Phase = m_PhaseIncremented = Math::Fast::fmod1(p);
                } else if (m_PhaseOffset < 1) {
                    m_PhaseIncremented = Math::Fast::fmod(m_PhaseIncremented + deltaPhase, 1.0 - m_PhaseOffset);
                    m_Phase = m_PhaseIncremented + m_PhaseOffset;
                } else {
                    m_Phase = 1;
                }
                break;
            case Mode::LoopHold:
                if (m_Gate) {
                    if (m_PhaseOffset > 0) {
                        m_Phase = m_PhaseIncremented = Math::Fast::fmod(m_PhaseIncremented + deltaPhase, m_PhaseOffset);
                    } else {
                        m_Phase = 0;
                    }
                } else {
                    m_Phase = m_PhaseIncremented = Math::Fast::min(m_PhaseIncremented + deltaPhase, 1.0);
                }
                break;
            }
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            Module::prepare(sampleRate, maxBufferSize);

        }

        void reset() override {
            Module::reset();
            m_Gate = false;
            m_FirstPhase = false;
            m_Phase = 0;
            m_PhaseIncremented = 0;
            output = 0;
        }

        // ------------------------------------------------
        
        void link(Storage& storage) { m_Storage = &storage; }

        // ------------------------------------------------
        
        void frequency(float freq) { m_Frequency = freq; }
        void time(float seconds) { m_Frequency = 1 / seconds; }
        void mix(float mix) { m_Mix = mix; }
        void phase(float phase) { m_PhaseOffset = phase; }
        void smooth(float millis) { m_SmoothMillis = millis; }

        void mode(Mode mode) { m_Mode = mode; }
        void tempo(Tempo tempo) { m_Tempo = tempo; }
        void sync(Sync sync) { m_Sync = sync; }

        // ------------------------------------------------
        
        float phase() { return m_Phase; }

        // ------------------------------------------------

    private:
        Storage* m_Storage = nullptr;

        Mode m_Mode;
        Sync m_Sync;
        Tempo m_Tempo;

        bool m_Gate = false;
        bool m_FirstPhase = false;

        float m_Mix = 0;
        float m_Phase = 0;
        float m_PhaseIncremented = 0;
        float m_PhaseOffset = 0;
        float m_Frequency = 0.1;
        float m_SmoothMillis = 0;

        // ------------------------------------------------
        
        float at(float x) {
            if (m_Storage) return m_Storage->at(x) * m_Mix + (1 - m_Mix) * 0.5;
            else return 0;
        }

        // ------------------------------------------------
        
        float samplesPerOscillation() {
            float samplesPerOscillation = 1;
            switch (m_Sync) {
            case Sync::Seconds:
                samplesPerOscillation = sampleRate() / m_Frequency;
                break;
            case Sync::Tempo:
                float nmrBarsForTempo = bars(m_Tempo);
                float beatsPerSecond = bpm();
                float beatsPerBar = timeSignature().numerator;
                float barsPerSecond = beatsPerSecond / beatsPerBar;
                float samplesPerBar = sampleRate() / barsPerSecond;
                samplesPerOscillation = nmrBarsForTempo * samplesPerBar;
                break;
            }

            return samplesPerOscillation;
        }

        float synchronizedPhase() {
            return timeInSamples() / samplesPerOscillation();
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
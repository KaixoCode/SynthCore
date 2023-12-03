#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"
#include "Kaixo/Core/Processing/Voice.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    template<std::derived_from<Voice> VoiceClass, std::size_t Count>
    class VoiceBank : public ModuleContainer {
    public:

        // ------------------------------------------------

        VoiceBank() {
            for (auto& voice : m_Voices)
                registerModule(voice);
        }

        virtual ~VoiceBank() = default;

        // ------------------------------------------------
        
        Stereo output;

        // ------------------------------------------------

        void maxVoices(std::size_t active) { 
            active = Math::clamp(active, 1ull, Count);
            if (m_MaxVoices != active) {
                m_MaxVoices = active;
                m_PressedInOrder.clear();
            }
        }

        void alwaysLegato(bool v) { m_AlwaysLegato = v; }
        void threading(bool v) { m_UseThreading = v; }

        // ------------------------------------------------

        void noteOn(Note note, double velocity) {
            std::size_t _activeVoices = 0;
            std::size_t _firstAvailableVoice = 0;
            for (std::size_t i = 0; i < Count; ++i) {
                auto& _voice = m_Voices[i];
                if (_voice.isActive()) _activeVoices++;
                else {
                    _firstAvailableVoice = i;
                    break;
                }
            }

            if (_activeVoices >= m_MaxVoices) {
                overrideVoice(note, velocity);
            } else {
                triggerVoice(_firstAvailableVoice, note, velocity, false);
            }
        }
        
        void noteOff(Note note, double velocity) {
            for (std::size_t i = 0; i < Count; ++i) {
                auto& _voice = m_Voices[i];
                if (_voice.note != note) continue;

                if (!restoreOverriden(i)) {
                    releaseVoice(i, note, velocity);
                }
            }

            removeFromHistory(note); 
        }

        // ------------------------------------------------
        
        void process() override {
            m_LastNote = lastTriggered().currentNote();

            for (auto& voice : m_Voices) {
                voice.output.prepare(outputBuffer().size());
                voice.process();
            }

            for (auto& voice : m_Voices) {
                for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
                    outputBuffer()[i] += voice.output[i];
                }
            }
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override { 
            Module::prepare(sampleRate, maxBufferSize);
            for (auto& voice : m_Voices)
                voice.prepare(sampleRate, maxBufferSize);
        }

        void reset() override {
            for (auto& voice : m_Voices)
                voice.reset();
        }

        // ------------------------------------------------
        
        void param(ParamID id, ParamValue value) override {
            for (auto& voice : m_Voices)
                voice.param(id, value);
        }

        // ------------------------------------------------

        auto begin() { return m_Voices.begin(); }
        auto end() { return m_Voices.end(); }

        VoiceClass& operator[](std::size_t i) { return m_Voices[i]; }

        // ------------------------------------------------
        
        VoiceClass& lastTriggered() {
            if (m_PressedVoices.size() > 0) return m_Voices[m_PressedVoices.back()];
            return m_Voices[m_LastTriggered];
        }

        // ------------------------------------------------
        
    private:
        struct History {
            std::size_t voice;
            Note note;
            double velocity;
        };

        // ------------------------------------------------

        std::array<VoiceClass, Count> m_Voices{};
        Vector<std::size_t, Count> m_PressedVoices{};
        Vector<std::size_t, Count> m_PressedInOrder{};
        Vector<History, 100> m_OverridenNoteHistory{};

        // ------------------------------------------------

        std::size_t m_LastTriggered = 0;
        std::size_t m_MaxVoices = Count;
        bool m_AlwaysLegato = false;
        bool m_UseThreading = false;
        float m_LastNote = -1;

        // ------------------------------------------------

        void removeFromHistory(Note note) {
            for (std::size_t i = 0; i < m_OverridenNoteHistory.size();) {
                auto& _history = m_OverridenNoteHistory[i];
                if (_history.note == note) {
                    m_OverridenNoteHistory.erase_index(i);
                } else ++i;
            }
        }

        bool restoreOverriden(std::size_t voice) {
            for (std::size_t i = m_OverridenNoteHistory.size(); i > 0; --i) {
                auto& _history = m_OverridenNoteHistory[i - 1];
                if (_history.voice == voice) {
                    m_OverridenNoteHistory.erase_index(i - 1);
                    triggerVoice(_history.voice, _history.note, _history.velocity, true);
                    return true;
                }
            }

            return false;
        }
        
        void overrideVoice(Note note, double velocity) {
            auto _toKill = m_PressedInOrder.front(); // kill oldest active note

            // If none removed from pressed, no need to override, just kill
            // the voice that is in release-mode.
            if (m_PressedVoices.remove(_toKill) == 0) {
                triggerVoice(_toKill, note, velocity, true);
                return;
            }

            if (m_OverridenNoteHistory.full()) {
                m_OverridenNoteHistory.pop_front();
            }

            m_OverridenNoteHistory.push_back(History{
                .voice = _toKill,
                .note = m_Voices[_toKill].note,
                .velocity = m_Voices[_toKill].velocity,
            });

            triggerVoice(_toKill, note, velocity, true);
        }
        
        void triggerVoice(std::size_t i, Note note, double velocity, bool legato) {
            m_Voices[i].fromNote = legato ? m_Voices[i].currentNote() : (m_AlwaysLegato ? m_LastNote : note);
            m_Voices[i].note = note;
            m_Voices[i].velocity = velocity;
            m_Voices[i].trigger(m_AlwaysLegato || (legato && m_Voices[i].pressed));
            m_Voices[i].pressed = true;
            m_PressedVoices.remove(i);
            m_PressedVoices.push_back(i);
            m_PressedInOrder.remove(i);
            m_PressedInOrder.push_back(i);
            m_LastTriggered = i;
        }

        void releaseVoice(std::size_t i, Note note, double velocity) {
            m_Voices[i].releaseVelocity = -1;
            m_Voices[i].release();
            m_Voices[i].pressed = false;
            m_PressedVoices.remove(i);
        }
    };
}
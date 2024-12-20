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

        enum class Mode { RoundRobin, Lifo };

        // ------------------------------------------------

        template<class ...Args>
        VoiceBank(Args&& ...args) : m_Voices{ std::forward<Args>(args)... } {
            for (auto& voice : m_Voices) registerModule(voice);
        }

        VoiceBank() { for (auto& voice : m_Voices) registerModule(voice); }

        // ------------------------------------------------

        void maxVoices(std::size_t active) {
            active = Math::clamp(active, 1ull, Count);
            if (m_MaxVoices != active) {
                m_MaxVoices = active;
                killAll();
            }
        }

        void alwaysLegato(bool v) { m_AlwaysLegato = v; }
        void threading(bool v) { m_UseThreading = v; }

        // ------------------------------------------------

        void noteOn(Note note, double velocity, int channel) {
            noteOnMPE(NoNoteID, note, velocity, channel);
        }

        void noteOnMPE(NoteID id, Note note, double velocity, int channel) {
            auto pick = chooseVoice();

            for (std::size_t i = 0; i < m_History.size(); ++i) {
                auto& n = m_History[i];
                if (n.note == note) {
                    // If note in history and already assigned to voice,
                    // use that voice instead
                    if (n.voice != NoVoice) {
                        pick.voice = n.voice;
                        pick.phase = Chosen::Sustain;
                    }
                    // Remove from history so it can later be added back on top
                    m_History.erase_index(i);
                    break;
                }
            }
            // If it is stolen, make sure to remove the voice from the note in the history
            if (pick.stolen()) {
                for (std::size_t i = 0; i < m_History.size(); ++i) {
                    auto& n = m_History[i];
                    if (n.voice == pick.voice) {
                        n.voice = NoVoice;
                        break;
                    }
                }
            }

            trigger(Trigger{
                .id = id,
                .velocity = velocity,
                .voice = pick.voice,
                .note = note,
                .channel = channel,
                .legato = pick.legato(),
                .stolen = pick.stolen(),
            });

            m_History.push_back(PressedNote{
                .id = id,
                .velocity = velocity,
                .note = note, 
                .voice = pick.voice
            });
        }

        void noteOff(Note note, double velocity, int channel) {
            noteOffMPE(NoNoteID, note, velocity, channel);
        }

        void noteOffMPE(NoteID id, Note note, double velocity, int channel) {
            // Remove the note from history
            for (std::size_t i = 0; i < m_History.size(); ++i) {
                auto& n = m_History[i];
                if (n.note == note) {
                    m_History.erase_index(i);
                    break;
                }
            }
            // Look for the voice that had this note
            std::size_t voiceThatHadThisNote = NoVoice;
            for (std::size_t i = 0; i < m_Voices.size(); ++i) {
                auto& voice = m_Voices[i];
                if (voice.pressed && voice.note == note) {
                    voiceThatHadThisNote = i;
                    break;
                }
            }
            // Apparently no voice had this note?? Okay well then don't do anything
            if (voiceThatHadThisNote == NoVoice) return; 

            // Look for notes in history that are not assigned to a voice yet
            std::size_t canSwitchToNoteFromHistory = npos;
            for (std::size_t i = 0; i < m_History.size(); ++i) {
                auto& n = m_History[i];
                if (n.voice == NoVoice) {
                    canSwitchToNoteFromHistory = i;
                    break;
                }
            }

            // Cannot switch to note from history, just release voice
            if (canSwitchToNoteFromHistory == npos) {
                release(Release{
                    .voice = voiceThatHadThisNote,
                    .velocity = velocity,
                    .note = note
                });
            } else {
                PressedNote n = m_History[canSwitchToNoteFromHistory];
                m_History.erase_index(canSwitchToNoteFromHistory);

                trigger({
                    .id = n.id,
                    .velocity = n.velocity,
                    .voice = voiceThatHadThisNote,
                    .note = n.note,
                    .channel = channel,
                    .legato = true,
                    .stolen = true,
                });

                m_History.push_back(PressedNote{
                    .id = n.id,
                    .velocity = n.velocity,
                    .note = n.note,
                    .voice = voiceThatHadThisNote
                });
            }
        }

        // ------------------------------------------------

        void notePitchBendMPE(NoteID id, double value) {
            for (auto& voice : m_Voices) {
                if (voice.id == id) {
                    voice.notePitchBendMPE(value);
                }
            }
        }

        void notePressureMPE(NoteID id, double value) {
            for (auto& voice : m_Voices) {
                if (voice.id == id) {
                    voice.notePressureMPE(value);
                }
            }
        }

        void noteTimbreMPE(NoteID id, double value) {
            for (auto& voice : m_Voices) {
                if (voice.id == id) {
                    voice.noteTimbreMPE(value);
                }
            }
        }

        // ------------------------------------------------

        void process() override {
            updateLastNote();

            const std::size_t nofSamplesToGenerate = outputBuffer().size();

            for (auto& voice : m_Voices) {
                voice.output.prepare(nofSamplesToGenerate);
            }

            if (m_UseThreading) {
                std::future<void> futures[Count]{};
                bool onMain[Count]{};
                int nofVoicesOnMainThread = 0;

                for (int i = 0; i < Count; i++) {
                    const bool isActive = m_Voices[i].active();
                    // If generating less than 52 samples, do work on main thread always
                    if (nofSamplesToGenerate < 52) onMain[i] = true; 
                    // Always do at least 1 voice on main thread
                    else if (isActive && nofVoicesOnMainThread < 1) onMain[i] = true, nofVoicesOnMainThread++; 
                    // If not active, process on main thread because it should not use too much CPU anyway
                    else if (!isActive) onMain[i] = true; 
                    // Otherwise send work to threadpool
                    else futures[i] = m_ThreadPool.push([this, i] { m_Voices[i].process(); }); 
                }

                // Process main thread voices
                for (int i = 0; i < Count; i++) {
                    if (onMain[i]) {
                        m_Voices[i].process();
                    }
                }
                // Wait for worker threads
                for (int i = 0; i < Count; i++) {
                    if (!onMain[i]) {
                        futures[i].wait();
                    }
                }
            } else {
                for (auto& voice : m_Voices) {
                    voice.process();
                }
            }

            for (auto& voice : m_Voices) {
                for (std::size_t i = 0; i < outputBuffer().size(); ++i) {
                    outputBuffer()[i] += voice.output[i];
                }
            }
        }

        // Happens in process call, but can be called separately if for whatever reason
        // the voice bank's process call isn't used.
        void updateLastNote() {
            m_LastNote = lastTriggered().currentNote();
        }

        // ------------------------------------------------

        auto begin() { return m_Voices.begin(); }
        auto end() { return m_Voices.end(); }

        VoiceClass& operator[](std::size_t i) { return m_Voices[i]; }

        // ------------------------------------------------

        VoiceClass& lastTriggered() { return m_Voices[m_LastTriggered]; }

        // ------------------------------------------------
        
        std::size_t activeVoices() const {
            std::size_t i = 0;
            for (auto& voice : m_Voices) {
                if (voice.active()) i++;
            }
            return i;
        }

        // ------------------------------------------------

    private:

        // ------------------------------------------------

        constexpr static std::size_t NoVoice = static_cast<std::size_t>(-1);

        // ------------------------------------------------

        std::array<VoiceClass, Count> m_Voices{};
        
        // ------------------------------------------------
        
        cxxpool::thread_pool m_ThreadPool{ Count };

        // ------------------------------------------------

        bool m_UseThreading = false;
        bool m_AlwaysLegato = false;
        std::size_t m_MaxVoices = Count;
        std::size_t m_LastTriggered = 0;
        Note m_LastNote = 0;

        // ------------------------------------------------

        Mode m_Mode = Mode::RoundRobin;

        // ------------------------------------------------

        struct PressedNote {
            NoteID id;
            double velocity = 0;
            Note note = 0;
            std::size_t voice = NoVoice; // Is note actually assigned to voice?
        };

        Vector<PressedNote, 32> m_History;

        // ------------------------------------------------

        bool active(std::size_t i) const { return m_Voices[i].active(); }
        bool pressed(std::size_t i) const { return m_Voices[i].pressed; }

        // ------------------------------------------------

        struct Chosen {
            std::size_t voice = 0;
            enum Phase { Sustain, Release, Dead } phase = Dead;
            bool stolen() const { return phase != Dead; }
            bool legato() const { return phase == Sustain; }
        };

        // ------------------------------------------------

        Chosen chooseVoice() {
            auto choose = [&](auto index) {
                std::size_t chosen = 0;
                auto find = [&](auto condition) {
                    for (std::size_t i = 0; i < m_MaxVoices; ++i) {
                        chosen = index(i);
                        if (condition(chosen)) return true;
                    }
                    return false;
                };
                // Prefer non-active voices
                if (find([&](auto voice) { return !active(voice); }))
                    return Chosen{ chosen, Chosen::Dead };
                // Otherwise prefer non-pressed voices (in release mode)
                if (find([&](auto voice) { return !pressed(voice); }))
                    return Chosen{ chosen, Chosen::Release };
                // Finally just pick the next voice if no other available
                return Chosen{ index(0), Chosen::Sustain};
            };

            switch (m_Mode) {
            case Mode::RoundRobin: return choose([&](std::size_t i) { return (i + m_LastTriggered + 1) % m_MaxVoices; });
            case Mode::Lifo: return choose([&](std::size_t i) { return i; });
            }
        }

        // ------------------------------------------------
        
        void killAll() {
            m_LastTriggered = 0;
            for (auto& voice : m_Voices) {
                voice.reset();
            }
        }

        // ------------------------------------------------

        struct Trigger {
            NoteID id;
            double velocity;
            std::size_t voice;
            Note note;
            int channel;
            bool legato;
            bool stolen;
        };

        void trigger(Trigger t) {
            auto& voice = m_Voices[t.voice];
            voice.id = t.id;
            voice.fromNote = t.legato ? voice.currentNote() : m_AlwaysLegato ? m_LastNote : t.note;
            voice.note = t.note;
            voice.channel = t.channel;
            voice.velocity = t.velocity;
            voice.legato = m_AlwaysLegato || t.legato;
            voice.stolen = t.stolen;
            voice.trigger();
            voice.pressed = true;
            m_LastTriggered = t.voice;
        }

        // ------------------------------------------------

        struct Release {
            std::size_t voice;
            double velocity;
            Note note;
        };

        void release(Release t) {
            auto& voice = m_Voices[t.voice];
            voice.id = NoNoteID;
            voice.releaseVelocity = t.velocity;
            voice.release();
            voice.pressed = false;
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
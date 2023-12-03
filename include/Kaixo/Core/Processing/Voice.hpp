#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"
#include "Kaixo/Core/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Voice : public ModuleContainer {
    public:

        // ------------------------------------------------

        Voice() = default;
        virtual ~Voice() = default;

        // ------------------------------------------------

        Note fromNote = -1;
        Note note = -1;
        float velocity = 0;
        float releaseVelocity = 0;
        bool pressed = false; // True after trigger is called, false after release is called

        // ------------------------------------------------

        Buffer output;

        // ------------------------------------------------

        // Should return true when playing sound
        virtual bool isActive() const { return false; }

        virtual void trigger(bool legato) = 0;
        virtual void release() = 0;

        virtual float currentNote() const { return note; }

        // ------------------------------------------------

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Voice)
    };

    // ------------------------------------------------

}
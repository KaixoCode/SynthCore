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

        Note fromNote = -1; // Previous note (to allow gliding)
        Note note = -1; // Pressed note
        int channel = 0;
        float velocity = 0;
        float releaseVelocity = 0;
        bool pressed = false; // True after trigger is called, false after release is called
        bool legato = false;
        bool stolen = false;

        // ------------------------------------------------

        Buffer output;

        // ------------------------------------------------

        virtual void trigger() = 0;
        virtual void release() = 0;

        virtual Note currentNote() const { return note; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
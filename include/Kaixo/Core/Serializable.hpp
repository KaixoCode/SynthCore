#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Formatters.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    class Serializable {
    public:

        // ------------------------------------------------

        virtual void init() {} // reset state to default
        virtual json serialize() = 0;
        virtual void deserialize(json&) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
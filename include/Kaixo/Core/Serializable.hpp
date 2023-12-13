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
        virtual basic_json serialize() = 0;
        virtual void deserialize(basic_json&) = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
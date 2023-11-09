#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    class ZoomMultiplier {
    public:

        // ------------------------------------------------

        constexpr ZoomMultiplier(std::floating_point auto val) : m_Z(Math::floor(val)), m_C(val * 10 - Math::floor(val) * 10) {}
        constexpr ZoomMultiplier(std::integral auto val) : m_Z(val), m_C(0) {}

        // ------------------------------------------------

        constexpr ZoomMultiplier& operator=(std::floating_point auto val) {
            m_Z = static_cast<std::uint8_t>(Math::floor(val));
            m_C = static_cast<std::uint8_t>(val * 10 - Math::floor(val) * 10);
            return *this;
        }

        // ------------------------------------------------

        constexpr ZoomMultiplier& operator=(std::integral auto val) {
            m_Z = static_cast<std::uint8_t>(val);
            m_C = 0;
            return *this;
        }

        // ------------------------------------------------

        constexpr bool operator==(const ZoomMultiplier& other) const { return other.m_Z == m_Z && other.m_C == m_C; }
        constexpr bool operator==(std::floating_point auto other) const { return ZoomMultiplier(other) == *this; }
        constexpr bool operator==(std::integral auto other) const { return ZoomMultiplier(other) == *this; }

        // ------------------------------------------------

        constexpr operator double() const { return m_Z + m_C / 10.; }

        // ------------------------------------------------

    private:
        std::uint8_t m_Z;
        std::uint8_t m_C;
    };

    // ------------------------------------------------

}
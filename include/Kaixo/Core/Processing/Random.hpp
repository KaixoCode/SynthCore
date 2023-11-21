#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class Random {
    public:

        // ------------------------------------------------

        float next() { 
			return static_cast<float>(xoroshiro128p()) / std::numeric_limits<std::uint64_t>::max();
		}

        // ------------------------------------------------

    private:
		std::uint64_t m_State[2]{ 0x84242f96eca9c41d, 0xa3c65b8776f96855 };

		// ------------------------------------------------

        constexpr uint64_t rotl(const std::uint64_t x, int k) {
            return (x << k) | (x >> (64 - k));
        }

        constexpr std::uint64_t xoroshiro128p() {
            const uint64_t s0 = m_State[0];
            uint64_t s1 = m_State[1];
            const uint64_t result = s0 + s1;

            s1 ^= s0;
            m_State[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
            m_State[1] = rotl(s1, 37);

            return result;
		}

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
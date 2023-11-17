#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/Core/Processing/Buffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct Resampler {

        // ------------------------------------------------

        struct {
            double in;
            double out;
        } samplerate;

        // ------------------------------------------------

        Stereo generate(auto generator) {
            Stereo input = { 0, 0 };

            // ------------------------------------------------

            const double delta = (samplerate.in / samplerate.out);

            // ------------------------------------------------

            if (delta < 1) { // Upsample
                filter.sampleRateIn = samplerate.in;
                filter.sampleRateOut = samplerate.out;

                const std::int64_t count = static_cast<std::int64_t>(leftovers + delta);

                if (count) {
                    input += filter.process(generator());
                } else {
                    input += filter.process({ 0, 0 });
                }

                input /= delta;

                leftovers = (leftovers + delta) - count;

            } else { // Downsample
                filter.sampleRateIn = samplerate.in;
                filter.sampleRateOut = samplerate.out;

                const std::int64_t count = static_cast<std::int64_t>(leftovers + delta);
                for (std::int64_t i = 0; i < count; ++i) {
                    input += filter.process(generator());
                }

                input /= count;

                leftovers = (leftovers + delta) - count;
            }

            // ------------------------------------------------

            return input;
        }

        // ------------------------------------------------

    private:
        double leftovers = 0;
        AAFilter filter;
    };
}
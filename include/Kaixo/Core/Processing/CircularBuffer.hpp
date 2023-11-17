#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class CircularBuffer {
    public:
        std::size_t writeIndex = 0;
        std::size_t size = 0;
        Stereo* data = nullptr;

        // ------------------------------------------------

        CircularBuffer(std::size_t size = 8192) : size(size), data(new Stereo[size]) {}
        ~CircularBuffer() { delete[] data; }

        // ------------------------------------------------

        void resize(std::size_t s) {
            auto backup = data;
            data = new Stereo[s];
            std::memcpy(data, backup, sizeof(Stereo) * std::min(size, s));
            size = s;
            delete[] backup;
        }

        void clear() { std::memset(data, 0, sizeof(Stereo) * size); }

        void write(Stereo in) {
            data[writeIndex] = in;
            writeIndex = (writeIndex + 1) % size;
        }

        void read(std::vector<Stereo>& to) {
            std::size_t writeI = writeIndex;
            for (std::size_t i = 0; i < to.size(); ++i) {
                to[i] += data[(i + writeI + size / 2) % size];
            }
        }
    };
}
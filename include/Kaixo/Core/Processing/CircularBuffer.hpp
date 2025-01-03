#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class CircularBuffer {
    public:

        // ------------------------------------------------

        std::size_t writeIndex = 0;
        std::size_t size = 0;
        Stereo* data = nullptr;

        // ------------------------------------------------

        CircularBuffer(std::size_t size = 8192) { reserve(size); }
        ~CircularBuffer() { delete[] data; }

        // ------------------------------------------------

        void reserve(std::size_t s) {
            if (s > size) resize(s);
        }

        void resize(std::size_t s) {
            auto backup = data;
            data = new Stereo[s];
            size = s;
            delete[] backup;
        }

        // ------------------------------------------------

        void clear() { std::memset(data, 0, sizeof(Stereo) * size); }

        // ------------------------------------------------

        void write(Stereo in) {
            data[writeIndex] = in;
            writeIndex = (writeIndex + 1) % size;
        }

        void read(std::vector<float>& to) const {
            std::size_t writeI = writeIndex;
            for (std::size_t i = 0; i < to.size(); ++i) {
                to[i] += data[(i + writeI + size / 2) % size].average();
            }
        }
        
        void read(std::vector<Stereo>& to) const {
            std::size_t writeI = writeIndex;
            for (std::size_t i = 0; i < to.size(); ++i) {
                to[i] += data[(i + writeI + size / 2) % size];
            }
        }
        
        void read(std::vector<std::complex<float>>& mono) const {
            std::size_t writeI = writeIndex;
            for (std::size_t i = 0; i < mono.size(); ++i) {
                mono[i] += data[(i + writeI + size / 2) % size].average();
            }
        }
        
        void read(std::vector<std::complex<Stereo>>& stereo) const {
            std::size_t writeI = writeIndex;
            for (std::size_t i = 0; i < stereo.size(); ++i) {
                stereo[i] += data[(i + writeI + size / 2) % size];
            }
        }
        
        void read(std::vector<std::complex<float>>& l, std::vector<std::complex<float>>& r) const {
            std::size_t writeI = writeIndex;
            std::size_t outSize = Math::min(l.size(), r.size());
            for (std::size_t i = 0; i < outSize; ++i) {
                auto& read = data[(i + writeI + size / 2) % size];
                l[i] += read.l;
                r[i] += read.r;
            }
        }

        // ------------------------------------------------

    };
}
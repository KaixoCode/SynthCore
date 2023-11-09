#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct Buffer {

        // ------------------------------------------------

        ~Buffer() { delete[] m_Data; }
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& buffer) noexcept
            : m_Data(buffer.m_Data), m_CurrentSize(buffer.m_CurrentSize), m_Size(buffer.m_Size) {
            buffer.m_Size = 0;
            buffer.m_CurrentSize = 0;
            buffer.m_Data = nullptr;
        }

        // ------------------------------------------------

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&& buffer) noexcept {
            delete[] m_Data;
            m_Data = buffer.m_Data;
            m_CurrentSize = buffer.m_CurrentSize;
            m_Size = buffer.m_Size;
            buffer.m_Size = 0;
            buffer.m_CurrentSize = 0;
            buffer.m_Data = nullptr;
            return *this;
        }

        // ------------------------------------------------

        Stereo* data() { return m_Data; }
        std::size_t size() const noexcept { return m_CurrentSize; }
        Stereo& operator[](std::size_t i) { return m_Data[i]; }

        // ------------------------------------------------

        void prepare(std::size_t s) {
            reserve(s);
            std::memset(m_Data, 0, sizeof(Stereo) * m_CurrentSize);
        }

        void reserve(std::size_t s) {
            if (s > m_Size) resize(s);
            m_CurrentSize = s;
        }

        void resize(std::size_t s) {
            m_CurrentSize = s;
            auto _backup = m_Data;
            m_Data = new Stereo[s];
            m_Size = s;
            delete[] _backup;
        }

        // ------------------------------------------------

    private:
        Stereo* m_Data = nullptr;
        std::size_t m_Size = 0;        // Allocated size
        std::size_t m_CurrentSize = 0; // Current size
    };
}
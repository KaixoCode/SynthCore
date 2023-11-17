#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct DelayBuffer {

        // ------------------------------------------------

        DelayBuffer(std::size_t bufferSize = 1024) { reserve(bufferSize); }
        ~DelayBuffer() { delete[] m_Buffer; }

        // ------------------------------------------------

        void setSampleRate(double sr) {
            if (m_SampleRate != sr) {
                m_SampleRate = sr;
                updateSmooth();
                updateDelay();
            }
        }

        // ------------------------------------------------

        void setDelay(double millis) {
            if (m_DelayMillis != millis) {
                m_DelayMillis = millis;
                updateDelay();
            }
        }

        // ------------------------------------------------

        Stereo write(Stereo in) {
            m_Buffer[m_Write] = in;
            auto res = read();

            m_Write = (m_Write + 1) % m_Size;
            m_Delay = m_Delay * m_Smooth + m_TargetDelay * (1 - m_Smooth);

            return res;
        }

        Stereo read() const {
            double read = Math::Fast::fmod(m_Write + m_Size - m_Delay, m_Size);
            std::size_t delay1 = static_cast<std::size_t>(read);
            std::size_t delay2 = static_cast<std::size_t>(read + 1) % m_Size;
            double ratio = read - delay1;

            return m_Buffer[delay2] * ratio + m_Buffer[delay1] * (1 - ratio);
        }

        // ------------------------------------------------

        void reset() {
            m_Delay = m_TargetDelay;
            std::memset(m_Buffer, 0, m_Size * sizeof(Stereo));
        }

        void reserve(std::size_t size) { if (size > m_Size) resize(size); }

        void resize(std::size_t size) {
            auto backup = m_Buffer;
            m_Buffer = new Stereo[size];
            std::memcpy(m_Buffer, backup, sizeof(Stereo) * Math::min(m_Size, size));
            m_Size = size;
            delete[] backup;
        }

        // ------------------------------------------------
    
    private:
        double m_SampleRate = 48000;
        std::size_t m_Size = 0;
        Stereo* m_Buffer = nullptr;
        std::size_t m_Write = 0;
        double m_Delay = 0;
        double m_TargetDelay = 0;
        double m_DelayMillis = 0;
        double m_Smooth = Math::smoothCoef(0.99, 48000. / m_SampleRate);

        void updateSmooth() { m_Smooth = Math::smoothCoef(0.99, 48000. / m_SampleRate); }
        void updateDelay() { m_TargetDelay = (m_DelayMillis / 1000.) * m_SampleRate; }
    };
}
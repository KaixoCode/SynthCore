
// ------------------------------------------------

#include "Kaixo/Core/Processing/MeasureCPU.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void MeasureCPU::start() {
        m_Timer.restart();
    }

    void MeasureCPU::end() {
        double nanos = m_Timer.time<std::chrono::nanoseconds>();
        double nanosUsedPerSample = nanos / outputBuffer().size();
        double availableNanosPerSample = 1e9 / sampleRate();
        double percentUsed = 100 * nanosUsedPerSample / availableNanosPerSample;

        m_TimerPercentMax = m_TimerPercentMax * 0.99 + 0.01 * percentUsed;
        m_TimerNanosPerSampleMax = m_TimerNanosPerSampleMax * 0.99 + 0.01 * nanosUsedPerSample;

        auto now = std::chrono::steady_clock::now();
        if (now - m_LastMeasure >= std::chrono::milliseconds(250)) {
            m_LastMeasure = now;
            m_TimerPercent = m_TimerPercentMax;
            m_TimerNanosPerSample = m_TimerNanosPerSampleMax;
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Utils/Timer.hpp"

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class MeasureCPU : public Module {
    public:

        // ------------------------------------------------
        
        float percentCPUUsage() const { return m_TimerPercent; }
        float nanosPerSample() const { return m_TimerNanosPerSample; }

        // ------------------------------------------------
        
        void start();
        void end();

        // ------------------------------------------------
        
    private:
        float m_TimerPercent = 0;
        float m_TimerNanosPerSample = 0;
        float m_TimerPercentMax = 0;
        float m_TimerNanosPerSampleMax = 0;
        std::chrono::time_point<std::chrono::steady_clock> m_LastMeasure;
        Timer<> m_Timer{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------

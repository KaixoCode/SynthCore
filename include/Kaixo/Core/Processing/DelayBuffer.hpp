#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"
#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class DelayBuffer : public Module {
    public:

        // ------------------------------------------------

        DelayBuffer(float maxDelay)
            : m_MaxDelay(maxDelay) 
        {}

        // ------------------------------------------------

        Stereo input;
        Stereo output;

        // ------------------------------------------------

        void delay(float millis) { m_TargetDelay = millis; }

        // ------------------------------------------------

        void process() override {
            m_Samples[m_Write] = input;
            output = read(m_Delay);
            m_Write = (m_Write + 1) % size();
            m_Delay = m_Delay * m_Smooth + m_TargetDelay * (1 - m_Smooth);
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            resize(sampleRate * m_MaxDelay / 1000.);
            m_Smooth = Math::smoothCoef(0.99, 48000. / sampleRate);
        }

        void reset() override {
            m_Delay = m_TargetDelay;
            std::ranges::fill(m_Samples, 0);
        }

        // ------------------------------------------------

        Stereo read(float delayMs) const {
            float delaySamples = sampleRate() * delayMs / 1000.;
            float read = Math::Fast::fmod(m_Write + size() - delaySamples, size());
            std::size_t delay1 = static_cast<std::size_t>(read);
            std::size_t delay2 = static_cast<std::size_t>(read + 1) % size();
            float ratio = read - delay1;
            return m_Samples[delay2] * ratio + m_Samples[delay1] * (1 - ratio);
        }

        // ------------------------------------------------

        void resize(std::size_t size) { m_Samples.resize(size); }

        // ------------------------------------------------

        std::size_t size() const { return m_Samples.size(); }

        // ------------------------------------------------
    
    private:
        std::vector<Stereo> m_Samples{};
        std::size_t m_Write = 0;
        float m_MaxDelay = 0;
        float m_Delay = 0;
        float m_TargetDelay = 0;
        float m_Smooth = 0.99;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
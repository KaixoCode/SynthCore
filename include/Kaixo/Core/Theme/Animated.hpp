#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Element.hpp"

// ------------------------------------------------

namespace Kaixo::Theme {

    // ------------------------------------------------

    template<class Ty>
    struct Animated : Animation {

        // ------------------------------------------------

        struct Assign {
            const Ty& value;
            double transition;
            std::function<double(double)> curve;
        };

        // ------------------------------------------------

        Animated& operator=(Assign a) {
            if (m_Goal == a.value && m_TransitionTime == a.transition) return *this; // Already assigned
            m_Current = get();
            m_Goal = a.value;
            m_TransitionTime = a.transition;
            m_TransitionCurve = std::move(a.curve);
            if (m_IsInitialized) {
                m_PointOfChange = std::chrono::steady_clock::now();
            }
            m_IsInitialized = true;
            return *this;
        }

        // ------------------------------------------------

        Ty get() const {
            auto _percent = percent();
            if (_percent == 1) return m_Goal;
            auto _transformed = m_TransitionCurve ? m_TransitionCurve(_percent) : _percent;
            return m_Current + (m_Goal - m_Current) * _transformed;
        }

        // ------------------------------------------------

        double percent() const {
            if (m_TransitionTime == 0) return 1;
            double millisPassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_PointOfChange).count();
            return Math::clamp1(millisPassed / m_TransitionTime);
        }

        // ------------------------------------------------

        bool changing() const override { return percent() != 1; }

        // ------------------------------------------------
        
        void uninitialize() { m_IsInitialized = false; }

        // ------------------------------------------------

    private:
        Ty m_Current{};
        Ty m_Goal{};
        double m_TransitionTime{}; // Millis
        std::chrono::steady_clock::time_point m_PointOfChange{}; // Time point when change happened
        bool m_IsInitialized = false;
        std::function<double(double)> m_TransitionCurve{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}
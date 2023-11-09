#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/Views/PointsDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class EnvelopeDisplay : public PointsDisplay, public ParameterListener {
    public:

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            PointsDisplay::Settings pointsDisplay{};
            
            // ------------------------------------------------
            
            Processing::InterfaceStorage<float()> phase;

            // ------------------------------------------------

            const ParamID delay = NoParam;
            const ParamID attack = NoParam;
            const ParamID decay = NoParam;
            const ParamID release = NoParam;
            const ParamID attackLevel = NoParam;
            const ParamID decayLevel = NoParam;
            const ParamID sustain = NoParam;
            const ParamID attackCurve = NoParam;
            const ParamID decayCurve = NoParam;
            const ParamID releaseCurve = NoParam;

            // ------------------------------------------------
            
            ParamValue resetValueDelay = 0;
            ParamValue resetValueAttack = 0.2;
            ParamValue resetValueDecay = 0.2;
            ParamValue resetValueRelease = 0.2;
            ParamValue resetValueAttackLevel = 0;
            ParamValue resetValueDecayLevel = 1;
            ParamValue resetValueSustain = 0.5;
            ParamValue resetValueAttackCurve = 0.5;
            ParamValue resetValueDecayCurve = 0.8;
            ParamValue resetValueReleaseCurve = 0.8;

            // ------------------------------------------------
            
            float zoom = 3;

            // ------------------------------------------------

            bool tooltip = false;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------
        
        EnvelopeDisplay(Context c, Settings s = {});

        // ------------------------------------------------
        
        void parameterChanged(ParamID id, ParamValue val) override;

        // ------------------------------------------------
        
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& d) override;

        // ------------------------------------------------

        float at(float x) override;
        std::size_t nofPoints() const override { return 4; }
        Point getPoint(std::size_t i) override;
        void setPoint(std::size_t i, Point point) override;
        void resetCurve(std::size_t i) override;
        void resetPoint(std::size_t i) override;
        float phase() override;

        // ------------------------------------------------

    private:
        ParamValue m_Delay = settings.resetValueDelay;
        ParamValue m_Attack = settings.resetValueAttack;
        ParamValue m_Decay = settings.resetValueDecay;
        ParamValue m_Release = settings.resetValueRelease;
        ParamValue m_AttackLevel = settings.resetValueAttackLevel;
        ParamValue m_DecayLevel = settings.resetValueDecayLevel;
        ParamValue m_Sustain = settings.resetValueSustain;
        ParamValue m_AttackCurve = settings.resetValueAttackCurve;
        ParamValue m_DecayCurve = settings.resetValueDecayCurve;
        ParamValue m_ReleaseCurve = settings.resetValueReleaseCurve;

        // ------------------------------------------------

        float multiplier() const { return nofPoints() / settings.zoom; }

        // ------------------------------------------------

        void setDelay(ParamValue v);
        void setAttack(ParamValue v);
        void setDecay(ParamValue v);
        void setRelease(ParamValue v);
        void setAttackLevel(ParamValue v);
        void setDecayLevel(ParamValue v);
        void setSustain(ParamValue v);
        void setAttackCurve(ParamValue v);
        void setDecayCurve(ParamValue v);
        void setReleaseCurve(ParamValue v);

        // ------------------------------------------------
        
        void resetDelay();
        void resetAttack();
        void resetDecay();
        void resetRelease();
        void resetAttackLevel();
        void resetDecayLevel();
        void resetSustain();
        void resetAttackCurve();
        void resetDecayCurve();
        void resetReleaseCurve();

        // ------------------------------------------------

    };
}
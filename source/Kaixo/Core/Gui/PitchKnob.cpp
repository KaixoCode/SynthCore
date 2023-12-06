#include "Kaixo/Core/Gui/PitchKnob.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    PitchKnob::PitchKnob(Context c, Settings settings)
        : Knob(c, std::move(settings.knob)), settings(std::move(settings)) {}

    // ------------------------------------------------

    void PitchKnob::mouseDown(const juce::MouseEvent& event) {
        Knob::mouseDown(event);
        m_IsTranspose = !settings.detuneLocation(localDimensions()).contains(event.x, event.y);
    }

    void PitchKnob::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);

        if (event.mods.isLeftButtonDown()) {

            ParamValue difference = Knob::settings.speed;

            if (event.mods.isShiftDown()) difference *= 0.25;
            if (event.mods.isCtrlDown())  difference *= 0.25;

            switch (Knob::settings.type) {
            case Type::Vertical:   difference *= (m_PreviousMousePosition.y() - event.y) * +.005; break;
            case Type::Horizontal: difference *= (m_PreviousMousePosition.x() - event.x) * -.005; break;
            }

            if (m_IsTranspose) {
                if (std::abs(difference) < 0.020) return;

                float pitch = value() * (settings.max - settings.min) + settings.min;
                float transpose = Math::trunc(pitch);
                float detune = pitch - transpose;
                transpose += Math::sign(difference);

                performEdit((transpose + detune - settings.min) / (1.0 * settings.max - settings.min));
            } else {
                performEdit(value() + difference * 0.02);
            }

            if (Storage::flag(Setting::TouchMode)) {
                m_PreviousMousePosition = { event.x, event.y };
            } else {
                context.cursorPos(localPointToGlobal(m_PreviousMousePosition));
                setMouseCursor(juce::MouseCursor::NoCursor);
            }

            if (Knob::settings.tooltipValue) {
                context.tooltip().update(valueString());
            }
        }
    }

    // ------------------------------------------------

    void PitchKnob::paint(juce::Graphics& g) {
        float pitch = std::abs(value() * (settings.max - settings.min) + settings.min);
        std::size_t transpose = std::trunc(pitch);
        std::size_t detune = 100 * (pitch - transpose);
        std::size_t sign = float(value() * (settings.max - settings.min) + settings.min < -0.001);

        settings.sign.draw({
            .graphics = g, 
            .bounds = settings.signLocation(localDimensions()),
            .index = sign, 
            .state = state()
        });

        settings.transpose.draw({
            .graphics = g, 
            .bounds = settings.transposeLocation(localDimensions()),
            .index = transpose, 
            .state = state()
        });

        settings.detune.draw({
            .graphics = g, 
            .bounds = settings.detuneLocation(localDimensions()),
            .index = detune, 
            .state = state()
        });
    }

    // ------------------------------------------------

}
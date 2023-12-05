#include "Kaixo/Core/Gui/Knob.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Knob::Knob(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        if (settings.param != NoParam) {
            m_Value = context.param(settings.param);
            description(parameter(settings.param).description);
        } else {
            m_Value = settings.resetValue;
        }
    }

    Knob::~Knob() {}

    // ------------------------------------------------

    void Knob::parameterChanged(ParamID id, ParamValue value) {
        if (settings.param == id) {
            m_Value = value;
            repaint();
        }
    }

    // ------------------------------------------------

    void Knob::mouseEnter(const juce::MouseEvent& event) {
        View::mouseEnter(event);
        if (settings.tooltipName) {
            context.tooltip().open({
                .string = name(),
                .position = openTooltipAt(),
            });
        }
    }

    void Knob::mouseExit(const juce::MouseEvent& event) {
        View::mouseExit(event);
        if (settings.tooltipValue || settings.tooltipName) {
            context.tooltip().close();
        }
    }

    void Knob::mouseDown(const juce::MouseEvent& event) {
        View::mouseDown(event);
        if (settings.tooltipValue) {
            if (settings.tooltipName) {
                context.tooltip().update(valueString());
            } else {
                context.tooltip().open({
                    .string   = valueString(),
                    .position = openTooltipAt()
                });
            }
        }

        if (!Storage::flag(Setting::TouchMode)) {
            setMouseCursor(juce::MouseCursor::NoCursor);
        }

        m_PreviousMousePosition = event.mouseDownPosition;

        beginEdit();
    }

    void Knob::mouseDrag(const juce::MouseEvent& event) {
        View::mouseDrag(event);
        if (event.mods.isLeftButtonDown()) {
            ParamValue difference = settings.speed;

            if (event.mods.isShiftDown()) difference *= 0.25;
            if (event.mods.isCtrlDown())  difference *= 0.25;

            switch (settings.type) {
            case Type::Vertical:   difference *= (m_PreviousMousePosition.y() - event.y) * +.005; break;
            case Type::Horizontal: difference *= (m_PreviousMousePosition.x() - event.x) * -.005; break;
            }

            performEdit(value() + difference);
            
            if (Storage::flag(Setting::TouchMode)) {
                m_PreviousMousePosition = { event.x, event.y };
            } else {
                context.cursorPos(localPointToGlobal(m_PreviousMousePosition));
                setMouseCursor(juce::MouseCursor::NoCursor);
            }

            if (settings.tooltipValue) {
                context.tooltip().update(valueString());
            }
        }
    }

    void Knob::mouseUp(const juce::MouseEvent& event) {
        View::mouseUp(event);

        if (!Storage::flag(Setting::TouchMode)) {
            if (settings.moveCursorWithValue) {
                switch (settings.type) {
                case Type::Vertical:
                    context.cursorPos(localPointToGlobal(Point{
                        m_PreviousMousePosition.x(),
                        y() + value() * height()
                    })); break;
                case Type::Horizontal:
                    context.cursorPos(localPointToGlobal(Point{
                        x() + value() * width(),
                        m_PreviousMousePosition.y(),
                    })); break;
                }
            }
        }

        setMouseCursor(juce::MouseCursor::NormalCursor);
        endEdit();
    }

    void Knob::mouseDoubleClick(const juce::MouseEvent& event) {
        View::mouseDoubleClick(event);
        reset();

        if (settings.tooltipValue) {
            if (settings.tooltipName) {
                context.tooltip().update(valueString());
            } else {
                context.tooltip().open({
                    .string = valueString(),
                    .position = openTooltipAt()
                });
            }
        }
    }

    // ------------------------------------------------

    ParamValue Knob::value() const { return m_Value; }

    void Knob::value(ParamValue value) {
        beginEdit();
        performEdit(value);
        endEdit();
    }

    void Knob::reset() {
        if (isLinkedToParam()) {
            value(context.defaultValue(settings.param));
        } else {
            value(settings.resetValue);
        }
    }

    // ------------------------------------------------

    void Knob::paint(juce::Graphics& g) {
        ParamValue v = value();
        if (steps() != 0) { // Round value if step
            v = normalToIndex(v, steps()) / (steps() - 1.);
        }

        settings.graphics.draw({
            .graphics = g,
            .position = localDimensions(),
            .parameter = settings.param,
            .value = v,
            .index = steps() == 0 ? npos : normalToIndex(v, steps()),
            .state = state(),
        });
    }

    // ------------------------------------------------

    bool Knob::isLinkedToParam() const { return settings.param != NoParam; }

    // ------------------------------------------------

    void Knob::beginEdit() {
        if (isLinkedToParam()) {
            context.beginEdit(settings.param);
        }
    }

    void Knob::endEdit() {
        if (isLinkedToParam()) {
            context.endEdit(settings.param);
        }
    }

    void Knob::performEdit(ParamValue newValue) {
        newValue = Math::clamp1(newValue);
        if (newValue == value()) return;

        if (settings.callback) 
            settings.callback(newValue);

        m_Value = newValue;
        if (isLinkedToParam()) {
            context.performEdit(settings.param, newValue);
        } else {
            repaint();
        }
    }

    // ------------------------------------------------

    Point<> Knob::openTooltipAt() const {
        return localPointToGlobal(settings.tooltipLocation(localDimensions()));
    }

    // ------------------------------------------------

    int Knob::steps() const {
        if (isLinkedToParam()) return context.steps(settings.param);
        else return settings.steps;
    }

    std::string Knob::name() const {
        if (isLinkedToParam()) return std::string(context.name(settings.param));
        else return settings.name;
    }

    std::string Knob::valueString() const {
        if (isLinkedToParam()) return context.toString(settings.param);
        else return settings.format.format(settings.transform.transform(value()));
    }

    // ------------------------------------------------

}
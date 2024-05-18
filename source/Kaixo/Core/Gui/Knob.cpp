#include "Kaixo/Core/Gui/Knob.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    Knob::Knob(Context c, Settings s)
        : View(c), settings(std::move(s))
    {
        if (settings.param != NoParam) {
            auto& param = parameter(settings.param);
            m_Value = context.param(settings.param);
            description(param.description);
        } else {
            m_Value = settings.resetValue;
        }

        animation(settings.graphics);
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
        if (event.mods.isLeftButtonDown()) {
            if (settings.tooltipValue) {
                if (settings.tooltipName) {
                    context.tooltip().update(valueString());
                }
                else {
                    context.tooltip().open({
                        .string = valueString(),
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
            case Type::Both:       difference *= (m_PreviousMousePosition.y() - event.y) * +.005 + 
                                                 (m_PreviousMousePosition.x() - event.x) * -.005; break;
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
                Rect evalRect = { value(), 0, width(), height() };

                bool hasX = settings.valuePosition.x;
                auto xPos = hasX
                    ? settings.valuePosition.x(evalRect) + x()
                    : m_PreviousMousePosition.x();
                
                bool hasY = settings.valuePosition.y;
                auto yPos = hasY
                    ? settings.valuePosition.y(evalRect) + y()
                    : m_PreviousMousePosition.y();

                switch (settings.type) {
                case Type::Vertical:
                    if (hasY) {
                        context.cursorPos(localPointToGlobal(Point{ xPos, yPos })); 
                    } else {
                        context.cursorPos(localPointToGlobal(Point{ xPos, y() + value() * height() })); 
                    }
                    break;
                case Type::Horizontal:
                    if (hasX) {
                        context.cursorPos(localPointToGlobal(Point{ xPos, yPos })); 
                    } else {
                        context.cursorPos(localPointToGlobal(Point{ x() + value() * width(), yPos })); 
                    }
                    break;
                case Type::Both:
                    context.cursorPos(localPointToGlobal(Point{ 
                        hasX ? xPos : (x() + value() * width()), 
                        hasY ? yPos : (y() + value() * height())
                    }));
                    break;
                }
            }
        }

        if (event.mods.isLeftButtonDown()) {
            setMouseCursor(juce::MouseCursor::NormalCursor);
            if (settings.onchange)
                settings.onchange(value());
            endEdit();
        }
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

    ParamValue Knob::transformedValue() const { 
        if (isLinkedToParam()) return context.controller<Controller>().parameter(settings.param).transformedValue();
        return settings.transform.transform(value());
    }

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

        if (settings.onchange)
            settings.onchange(value());
    }

    // ------------------------------------------------

    void Knob::paint(juce::Graphics& g) {
        ParamValue v = value();
        if (steps() != 0) { // Round value if step
            v = normalToIndex(v, steps()) / (steps() - 1.);
        }

        if (isLinkedToParam()) {
            settings.graphics.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .parameter = settings.param,
                .value = v,
                .state = state(),
            });
        } else {
            settings.graphics.draw({
                .graphics = g,
                .bounds = localDimensions(),
                .parameter = settings.param,
                .value = v,
                .text = { 
                    { "$name", settings.name },
                    { "$value", valueString() }
                },
                .state = state(),
            });
        }
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
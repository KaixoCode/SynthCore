#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Storage.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Gui/Tooltip.hpp"
#include "Kaixo/Core/Theme/Drawable.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class Knob : public View, public ParameterListener {
    public:

        // ------------------------------------------------

        enum class Type {
            Vertical,  // Responds to vertical mouse movement
            Horizontal,// Responds to horizontal mouse movement
            Both,      // Responds to both vertical and horizontal
        };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            std::function<void(ParamValue)> callback{};
            std::function<void(ParamValue)> onchange{};
            Theme::Drawable graphics{};
            Type type = Type::Vertical;
            float speed = 1;
            bool moveCursorWithValue = false;
            UnevaluatedPoint valuePosition{ {}, {} };

            // ------------------------------------------------

            bool tooltipName = true;
            bool tooltipValue = true;
            UnevaluatedPoint tooltipLocation{ X + Width / 2, Y - 10 };

            // ------------------------------------------------

            std::string name = "";
            std::size_t steps = 0;
            Formatter format = Formatters::Default;
            Transform transform = Transformers::Range<0.f, 1.f>;
            ParamValue resetValue = 0;

            // ------------------------------------------------

            // When selected, it will use parameter format/transform etc.
            const ParamID param = NoParam;

            // ------------------------------------------------

        } settings{};

        // ------------------------------------------------

        Knob(Context c, Settings settings = {});
        ~Knob();

        // ------------------------------------------------

        void parameterChanged(ParamID id, ParamValue value) override;

        // ------------------------------------------------

        void mouseEnter(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;

        // ------------------------------------------------

        virtual ParamValue value() const;
        void value(ParamValue value);

        ParamValue transformedValue() const;

        void reset();

        // ------------------------------------------------

        void beginEdit();
        virtual void performEdit(ParamValue newValue);
        void endEdit();

        // ------------------------------------------------

        void paint(juce::Graphics& g) override;

        // ------------------------------------------------
    protected:
        ParamValue m_Value = 0;
        Point<> m_PreviousMousePosition{ 0, 0 };

        // ------------------------------------------------

        bool isLinkedToParam() const;

        // ------------------------------------------------

        Point<> openTooltipAt() const;

        // ------------------------------------------------

        int steps() const;

        std::string name() const;
        std::string valueString() const;

        // ------------------------------------------------

    };
}
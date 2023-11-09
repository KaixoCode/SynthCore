#pragma once
#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Color.hpp"
#include "Kaixo/Core/Theme/Stateful.hpp"
#include "Kaixo/Core/Theme/Basic.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class PointsDisplay : public View {
    public:

        // ------------------------------------------------

        struct Listener {

            // ------------------------------------------------

            virtual void select(std::size_t) = 0;
            virtual void hover(std::size_t) = 0;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        struct Point {
            float x = 0;
            float y = 0;
            float c = 0;
        };

        struct UIPoint {
            bool hovering = false;
            bool dragging = false;
            bool selected = false;

            float x = 0;
            float y = 0;
        };

        // ------------------------------------------------

        struct Settings {

            // ------------------------------------------------

            struct Line {
                Theme::Color fill{};
                Theme::Color stroke{};

                float strokeWeight = 2;
                bool loop = true;

                UnevaluatedCoord start = Height / 2;
                UnevaluatedCoord end   = Height / 2;
            } line{};

            // ------------------------------------------------

            struct Grid {
                Theme::Color color{};
                float snapDistance = 10; // Distance in pixels from grid line to snap to grid

                int x = 8; // grid in x-axis
                int y = 1; // grid in y-axis
            } grid{};

            // ------------------------------------------------

            struct Phase {
                Theme::Color color{};
                bool display = false;
            } phase{};

            // ------------------------------------------------
            
            Theme::Basic background{};

            // ------------------------------------------------

            const std::size_t maxPoints = 0;

            Theme::Stateful mainPoint{};
            Theme::Stateful curvePoint{};

            float size = 20;     // Size of the points
            float maxCurve = 10; // Maximum curve value of line between points

            bool enableAddPoints = true;
            bool keepPointsInOrder = true; // Stops points from moving past their neighbouring points
            bool hasCurves = true; // Are the points connected by adjustable curves?

            Kaixo::Point<> padding{ 0, 0 };

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        PointsDisplay(Context c, Settings s = {});

        // ------------------------------------------------

        void mouseDoubleClick(const juce::MouseEvent& event) override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override;

        // ------------------------------------------------

        void onIdle() override;

        // ------------------------------------------------
        
        std::size_t hoveringPoint() const { return static_cast<std::size_t>(m_Closest); }

        // ------------------------------------------------
        
        void addListener(Listener* l) { m_Listeners.push_back(l); }
        void removeListener(Listener* l) { std::erase(m_Listeners, l); }

        void addCallback(std::function<void(std::size_t)> c) { m_Callbacks.push_back(std::move(c)); }
        
        void select(std::size_t i);

        // ------------------------------------------------

    protected:
        std::vector<std::function<void(std::size_t)>> m_Callbacks;
        std::vector<Listener*> m_Listeners{};
        std::vector<UIPoint> m_UIPoints{};
        std::size_t m_Closest = npos;
        Kaixo::Point<> m_PreviousMousePosition{ 0, 0 };
        float m_CurrentPhase = 0;
        bool m_IsCurve = false;
        bool m_DidDrag = false;

        // ------------------------------------------------

        virtual float at(float x) = 0;
        virtual std::size_t nofPoints() const = 0;
        virtual Point getPoint(std::size_t i) = 0;
        virtual void setPoint(std::size_t i, Point point) = 0;
        virtual void resetCurve(std::size_t i);
        virtual void resetPoint(std::size_t i) {};
        virtual void addPoint(Point point) {};
        virtual void removePoint(std::size_t i) {};
        virtual float phase() { return 0; }

        // ------------------------------------------------
        
        void hover(std::size_t i);

        // ------------------------------------------------
        
        void ensureUIPointsSize();

        // ------------------------------------------------

        Rect<> paddedDimensions();

        // ------------------------------------------------

        float snapToGrid(float x, int n, float distance);
        float snapToGridX(float x);
        float snapToGridY(float y);

        // ------------------------------------------------

        void addAmountToPoint(std::size_t i, Kaixo::Point<float> amount, bool snap);
        void addNewPointAtPosition(Kaixo::Point<float> position);

        // ------------------------------------------------

        Kaixo::Point<float> positionOfCurvePoint(std::size_t i);
        Kaixo::Point<float> positionOfPoint(std::size_t i);
        Rect<float> handleRectAt(Kaixo::Point<float> point);

        // ------------------------------------------------

        void synchronizeValues();

        // ------------------------------------------------

        bool shouldIgnoreCurve(std::size_t i);
        void findClosest(Kaixo::Point<float> point);

        // ------------------------------------------------


    };

    // ------------------------------------------------

}

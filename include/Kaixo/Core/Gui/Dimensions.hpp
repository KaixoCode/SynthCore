#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class UnevaluatedCoord {
    public:

        // ------------------------------------------------

        class Value {
        public:

            // ------------------------------------------------

            virtual Coord get(const Rect<>&) const = 0;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        template<std::invocable<const Rect<>&> Ty>
        class FunctionValue : public Value {
        public:

            // ------------------------------------------------

            FunctionValue(const Ty& value) : m_Value(value) {}

            // ------------------------------------------------

            Coord get(const Rect<>& rect) const override { return m_Value(rect); }

            // ------------------------------------------------

        private:
            Ty m_Value;

            // ------------------------------------------------

        };
        
        // ------------------------------------------------

        class PlainValue : public Value {
        public:
            
            // ------------------------------------------------
            
            PlainValue(Coord value) : m_Value(value) {}

            // ------------------------------------------------

            Coord get(const Rect<>&) const override { return m_Value; }

            // ------------------------------------------------

        private:
            Coord m_Value;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        UnevaluatedCoord() {}
        
        UnevaluatedCoord(std::convertible_to<Coord> auto pos)
            : m_Value(std::make_shared<PlainValue>(static_cast<Coord>(pos)))
        {}

        UnevaluatedCoord(const std::invocable<const Rect<>&> auto & fun)
            : m_Value(std::make_shared<FunctionValue<std::decay_t<decltype(fun)>>>(fun))
        {}

        // ------------------------------------------------

        operator bool() const { return (bool)m_Value; }

        Coord operator()(const Rect<>& r) const { return m_Value ? m_Value->get(r) : 0; }

        static Coord from(const UnevaluatedCoord& pos, const Rect<>& r) { return pos(r); }
        static Coord from(const std::invocable<const Rect<>&> auto & pos, const Rect<>& r) { return pos(r); }
        static Coord from(std::convertible_to<Coord> auto c, const Rect<>&) { return static_cast<Coord>(c); }

        // ------------------------------------------------

    private:
        std::shared_ptr<Value> m_Value{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class A, class B>
    concept posOpArgs = std::same_as<A, UnevaluatedCoord> || std::same_as<B, UnevaluatedCoord>;

    template<class A, class B> requires posOpArgs<A, B>
    inline UnevaluatedCoord operator-(const A& a, const B& b) { return { [=](const Rect<>& r) { return UnevaluatedCoord::from(a, r) - UnevaluatedCoord::from(b, r); } }; }
    template<class A, class B> requires posOpArgs<A, B>
    inline UnevaluatedCoord operator+(const A& a, const B& b) { return { [=](const Rect<>& r) { return UnevaluatedCoord::from(a, r) + UnevaluatedCoord::from(b, r); } }; }
    template<class A, class B> requires posOpArgs<A, B>
    inline UnevaluatedCoord operator*(const A& a, const B& b) { return { [=](const Rect<>& r) { return UnevaluatedCoord::from(a, r) * UnevaluatedCoord::from(b, r); } }; }
    template<class A, class B> requires posOpArgs<A, B>
    inline UnevaluatedCoord operator/(const A& a, const B& b) { return { [=](const Rect<>& r) { return UnevaluatedCoord::from(a, r) / UnevaluatedCoord::from(b, r); } }; }

    // ------------------------------------------------

    inline const UnevaluatedCoord X     { [](const Rect<>& r) { return r.x(); } };
    inline const UnevaluatedCoord Y     { [](const Rect<>& r) { return r.y(); } };
    inline const UnevaluatedCoord Width { [](const Rect<>& r) { return r.width(); } };
    inline const UnevaluatedCoord Height{ [](const Rect<>& r) { return r.height(); } };
    inline const UnevaluatedCoord Left  { [](const Rect<>& r) { return r.left(); } };
    inline const UnevaluatedCoord Right { [](const Rect<>& r) { return r.right(); } };
    inline const UnevaluatedCoord Top   { [](const Rect<>& r) { return r.top(); } };
    inline const UnevaluatedCoord Bottom{ [](const Rect<>& r) { return r.bottom(); } };

    // ------------------------------------------------

    struct UnevaluatedRect {

        // ------------------------------------------------

        UnevaluatedCoord x{};
        UnevaluatedCoord y{};
        UnevaluatedCoord width{};
        UnevaluatedCoord height{};

        // ------------------------------------------------

        UnevaluatedCoord left()   const { return x + width; }
        UnevaluatedCoord top()    const { return y + height; }
        UnevaluatedCoord right()  const { return x + width; }
        UnevaluatedCoord bottom() const { return y + height; }

        // ------------------------------------------------

        Rect<> operator()(const Rect<>& r) const {
            return { x(r), y(r), width(r), height(r) };
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    struct UnevaluatedPoint {

        // ------------------------------------------------

        UnevaluatedCoord x;
        UnevaluatedCoord y;

        // ------------------------------------------------

        Point<> operator()(const Rect<>& r) const {
            return { x(r), y(r) };
        }

        // ------------------------------------------------

    };
}
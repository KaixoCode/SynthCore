#pragma once

// ------------------------------------------------

#include <JuceHeader.h>

// ------------------------------------------------

#include <any>
#include <bitset>
#include <numbers>
#include <algorithm>
#include <cmath>
#include <format>
#include <vector>
#include <charconv>
#include <string>
#include <string_view>
#include <iostream>
#include <array>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <condition_variable>
#include <future>
#include <complex>
#include <cassert>
#include <valarray>
#include <random>
#include <set>
#include <map>
#include <unordered_map>
#include <ranges>
#include <numeric>
#include <unordered_set>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <memory>

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    constexpr static std::size_t npos = static_cast<std::size_t>(-1);

    // ------------------------------------------------

    using ParamValue = float;
    using ParamID = std::uint32_t;
    using ModulationSourceID = std::uint32_t;
    using ModulationID = std::uint32_t;

    constexpr ParamID NoParam = static_cast<ParamID>(-1);
    constexpr ParamID NoSource = static_cast<ModulationSourceID>(-1);

    // ------------------------------------------------

    using ImageID = std::size_t;
    constexpr ImageID NoImage = static_cast<ImageID>(-1);
    
    // ------------------------------------------------

    using FontID = std::size_t;
    constexpr FontID NoFont = static_cast<FontID>(-1);

    // ------------------------------------------------

    using Note = float; // Should be int32_t, but this allows gliding

    // ------------------------------------------------

    using Coord = int;

    // ------------------------------------------------
    
    template<class Ty = Coord> struct Point;

    // ------------------------------------------------
    
    template<class Ty = Coord>
    struct Rect : juce::Rectangle<Ty> {

        // ------------------------------------------------

        Rect()
            : juce::Rectangle<Ty>(0, 0, 0, 0) {}

        Rect(juce::Rectangle<Ty> o)
            : juce::Rectangle<Ty>(o) {}

        Rect(Ty x, Ty y, Ty w, Ty h)
            : juce::Rectangle<Ty>{ x, y, w, h } {}
        
        Rect(std::convertible_to<Ty> auto x, 
             std::convertible_to<Ty> auto y, 
             std::convertible_to<Ty> auto w, 
             std::convertible_to<Ty> auto h
        ) : juce::Rectangle<Ty>{ 
                static_cast<Ty>(x), 
                static_cast<Ty>(y), 
                static_cast<Ty>(w), 
                static_cast<Ty>(h) 
            } 
        {}

        template<class Arg>
        Rect(juce::Rectangle<Arg> o)
            : juce::Rectangle<Ty>(
                static_cast<Ty>(o.getX()), 
                static_cast<Ty>(o.getY()), 
                static_cast<Ty>(o.getWidth()),
                static_cast<Ty>(o.getHeight())) {}

        // ------------------------------------------------

        Ty x() const { return this->getX(); }
        Ty y() const { return this->getY(); }
        Ty width() const { return this->getWidth(); }
        Ty height() const { return this->getHeight(); }
        Ty left() const { return x(); }
        Ty right() const { return x() + width(); }
        Ty top() const { return y(); }
        Ty bottom() const { return y() + height(); }
        Ty centerX() const { return this->getCentreX(); }
        Ty centerY() const { return this->getCentreY(); }
        Point<Ty> size() const { return { width(), height() }; }
        Point<Ty> position() const { return { x(), y() }; }

        // ------------------------------------------------
        
        Point<Ty> topLeft() const { return { left(), top() }; }
        Point<Ty> topRight() const { return { right(), top() }; }
        Point<Ty> topCenter() const { return { centerX(), top() }; }
        Point<Ty> bottomLeft() const { return { left(), bottom() }; }
        Point<Ty> bottomRight() const { return { right(), bottom() }; }
        Point<Ty> bottomCenter() const { return { centerX(), bottom() }; }
        Point<Ty> centerLeft() const { return { left(), centerY() }; }
        Point<Ty> centerRight() const { return { right(), centerY() }; }
        Point<Ty> center() const { return { centerX(), centerY() }; }

        // ------------------------------------------------

        Rect& x(Ty value) { this->setX(value); return *this; }
        Rect& y(Ty value) { this->setY(value); return *this; }
        Rect& width(Ty value) { this->setWidth(value); return *this; }
        Rect& height(Ty value) { this->setHeight(value); return *this; }
        Rect& left(Ty value) { auto r = right(); x(value); right(r); return *this; }
        Rect& right(Ty value) { width(value - x()); return *this; }
        Rect& top(Ty value) { auto b = bottom(); y(value); bottom(b); return *this; }
        Rect& bottom(Ty value) { height(value - y()); return *this; }
        Rect& size(Point<Ty> value) { width(value.x()), height(value.y()); return *this; }
        Rect& size(Ty w, Ty h) { width(w), height(h); return *this; }
        Rect& position(Point<Ty> value) { x(value.x()), y(value.y()); return *this; }
        Rect& position(Ty a, Ty b) { x(a), y(b); return *this; }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<class Ty>
    struct Point : juce::Point<Ty> {

        // ------------------------------------------------

        Point()
            : juce::Point<Ty>(0, 0) {}

        Point(juce::Point<Ty> o)
            : juce::Point<Ty>(o) {}

        Point(Ty x, Ty y)
            : juce::Point<Ty>{ x, y } {}

        Point(std::convertible_to<Ty> auto x,
            std::convertible_to<Ty> auto y
        ) : juce::Point<Ty>{
                static_cast<Ty>(x),
                static_cast<Ty>(y)
            } 
        {}

        template<class Arg>
        Point(juce::Point<Arg> o)
            : juce::Point<Ty>(
                static_cast<Ty>(o.getX()),
                static_cast<Ty>(o.getY())) {}

        // ------------------------------------------------

        Ty x() const { return this->getX(); }
        Ty y() const { return this->getY(); }

        // ------------------------------------------------

        Point& x(Ty value) { this->setX(value); return *this; }
        Point& y(Ty value) { this->setY(value); return *this; }

        // ------------------------------------------------

    };
}

// ------------------------------------------------

#include "Kaixo/Utils/json.hpp"
#include "Kaixo/Utils/utils.hpp"
#include "Kaixo/Utils/string_literal.hpp"
#include "Kaixo/Utils/thread_pool.hpp"
#include "Kaixo/Utils/Containers.hpp"
#include "Kaixo/Utils/Random.hpp"
#include "Kaixo/Utils/Timer.hpp"
#include "Kaixo/Utils/utils.hpp"
#include "Kaixo/Utils/simd.hpp"
#include "Kaixo/Utils/Math.hpp"
#include "Kaixo/Utils/Color.hpp"
#include "Kaixo/Utils/StringUtils.hpp"

#include "Kaixo/Core/Processing/Stereo.hpp"

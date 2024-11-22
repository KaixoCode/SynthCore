#pragma once
#include <utility>
#include <cstddef>
#include <type_traits>
#include <numbers>
#include "basic_simd.hpp"
#include "Kaixo/Core/Processing/Stereo.hpp"

// ------------------------------------------------

#define KAIXO_MONO constexpr static __forceinline auto
#define KAIXO_POLY static __forceinline auto
#define KAIXO_STEREO(name) \
constexpr static __forceinline Processing::Stereo name(is_stereo auto ...args) noexcept { \
    return { name(args.l...), name(args.r...) }; \
}
#define KAIXO_STEREO_T(name, call) \
constexpr static __forceinline Processing::Stereo name(is_stereo auto ...args) noexcept { \
    return { call(args.l...), call(args.r...) }; \
}

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    using namespace kaixo;

    // ------------------------------------------------
    
    template<class Ty> concept is_mono = std::is_arithmetic_v<Ty>;
    template<class Ty> concept is_poly = is_simd<Ty>;

    // ------------------------------------------------

    struct Math {

        // ------------------------------------------------
        
        KAIXO_MONO saw(is_mono auto x, is_mono auto nf /*norm freq [0-1]*/) noexcept {
            constexpr auto f = [](auto x) {
                constexpr auto g = [](auto x) {
                    return ((x * x - 1.f) * x) / 6.f;
                };

                return g(fmod1(x + 3) * 2 - 1);
            };

            return (f(x - 2 * nf) + f(x) - 2 * f(x - nf)) * 0.25f * (1.f / nf) * (1.f / nf);
        }
        
        KAIXO_POLY saw(is_poly auto x, is_poly auto nf /*norm freq [0-1]*/) noexcept {
            constexpr auto f = [](auto x) {
                constexpr auto g = [](auto x) {
                    return ((x * x - 1.f) * x) / 6.f;
                };

                return g(fmod1(x + 3) * 2 - 1);
            };

            return (f(x - 2 * nf) + f(x) - 2 * f(x - nf)) * 0.25f * (1.f / nf) * (1.f / nf);
        }
        KAIXO_STEREO(saw)

        // ------------------------------------------------

        KAIXO_MONO trunc(is_mono auto x) noexcept { return std::trunc(x); }
        KAIXO_POLY trunc(is_poly auto x) noexcept { return x.trunc(); }
        KAIXO_STEREO(trunc)

        KAIXO_MONO floor(is_mono auto x) noexcept { return std::floor(x); }
        KAIXO_POLY floor(is_poly auto x) noexcept { return x.floor(); }
        KAIXO_STEREO(floor)
        
        KAIXO_MONO round(is_mono auto x) noexcept { return std::round(x); }
        KAIXO_POLY round(is_poly auto x) noexcept { return x.round(); }
        KAIXO_STEREO(round)

        KAIXO_MONO ceil(is_mono auto x) noexcept { return std::ceil(x); }
        KAIXO_POLY ceil(is_poly auto x) noexcept { return x.ceil(); }
        KAIXO_STEREO(ceil)

        KAIXO_MONO abs(is_mono auto x) noexcept { return std::abs(x); }
        KAIXO_MONO abs(is_poly auto x) noexcept { return x.abs(); }
        KAIXO_STEREO(abs)

        // ------------------------------------------------

        KAIXO_MONO max(is_mono auto a, is_mono auto b) noexcept { return a > b ? a : b; }
        KAIXO_POLY max(is_poly auto a, is_poly auto b) noexcept { auto m = a > b; return m & a | ~m & b; }
        KAIXO_STEREO(max)

        KAIXO_MONO min(is_mono auto a, is_mono auto b) noexcept { return a < b ? a : b; }
        KAIXO_POLY min(is_poly auto a, is_poly auto b) noexcept { auto m = a < b; return m & a | ~m & b; }
        KAIXO_STEREO(min)

        KAIXO_MONO clamp(is_mono auto a, is_mono auto b, is_mono auto c) noexcept { return a < b ? b : a > c ? c : a; }
        KAIXO_POLY clamp(is_poly auto a, is_mono auto b, is_mono auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
        KAIXO_POLY clamp(is_poly auto a, is_poly auto b, is_mono auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
        KAIXO_POLY clamp(is_poly auto a, is_mono auto b, is_poly auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
        KAIXO_POLY clamp(is_poly auto a, is_poly auto b, is_poly auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
        KAIXO_STEREO(clamp)

        KAIXO_MONO clamp1(is_mono auto a) noexcept { return a < 0 ? 0 : a > 1 ? 1 : a; }
        KAIXO_POLY clamp1(is_poly auto a) noexcept { auto m = a < 0.f; auto n = a > 1.f; return n & 1.f | ~m & ~n & a; }
        KAIXO_STEREO(clamp1)

        KAIXO_MONO clamp11(is_mono auto a) noexcept { return a < -1 ? -1 : a > 1 ? 1 : a; }
        KAIXO_POLY clamp11(is_poly auto a) noexcept { auto m = a < -1.f; auto n = a > 1.f; return m & -1.f | n & 1.f | ~m & ~n & a; }
        KAIXO_STEREO(clamp11)

        // ------------------------------------------------

        KAIXO_MONO tanh(is_mono auto x) noexcept { return std::tanh(x); }
        KAIXO_POLY tanh(is_poly auto x) noexcept { return x.tanh(); }
        KAIXO_STEREO(tanh)

        KAIXO_MONO tanh_like(is_mono auto x) noexcept { return x / (1 + abs(x)); }
        KAIXO_POLY tanh_like(is_poly auto x) noexcept { return x / (1 + abs(x)); }
        KAIXO_STEREO(tanh_like)

        // ------------------------------------------------

        KAIXO_MONO exp2(is_mono auto x) noexcept { return std::exp2(x); }
        KAIXO_POLY exp2(is_poly auto x) noexcept { return x.exp2(); }
        KAIXO_STEREO(exp2)

        KAIXO_MONO log2(is_mono auto x) { return std::log2(x); }
        KAIXO_POLY log2(is_poly auto x) { return x.log2(); }
        KAIXO_STEREO(log2)

        KAIXO_MONO exp(is_mono auto x) noexcept { return std::exp(x); }
        KAIXO_POLY exp(is_poly auto x) noexcept { return x.exp(); }
        KAIXO_STEREO(exp)

        KAIXO_MONO log(is_mono auto x) noexcept { return std::log(x); }
        KAIXO_POLY log(is_poly auto x) noexcept { return x.log(); }
        KAIXO_STEREO(log)

        KAIXO_MONO log10(is_mono auto x) noexcept { return std::log10(x); }
        KAIXO_POLY log10(is_poly auto x) noexcept { return x.log10(); }
        KAIXO_STEREO(log10)
            
        KAIXO_MONO pow(is_mono auto a, is_mono auto b) noexcept { return std::pow(a, b); }
        KAIXO_POLY pow(is_poly auto a, is_poly auto b) noexcept { return a.pow(b); }
        KAIXO_STEREO(pow)
        
        template<std::size_t N> KAIXO_MONO powN(is_mono auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return ((Is, a) * ...); }(std::make_index_sequence<N>{}); }
        template<std::size_t N> KAIXO_POLY powN(is_poly auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return ((Is, a) * ...); }(std::make_index_sequence<N>{}); }
        template<std::size_t N> KAIXO_MONO invPowN(is_mono auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return 1 - ((Is, (1 - a)) * ...); }(std::make_index_sequence<N>{}); }
        template<std::size_t N> KAIXO_POLY invPowN(is_poly auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return 1 - ((Is, (1 - a)) * ...); }(std::make_index_sequence<N>{}); }

        KAIXO_MONO sqrt(is_mono auto a) noexcept { return std::sqrt(a); }
        KAIXO_POLY sqrt(is_poly auto a) noexcept { return a.sqrt(); }
        KAIXO_STEREO(sqrt)

        // ------------------------------------------------

        KAIXO_MONO bend(is_mono auto a, is_mono auto b) noexcept {
            return b == 0 ? a
                : b < 0 ? 1 - tanh_like(b - b * a) / tanh_like(b)
                : tanh_like(b * a) / tanh_like(b);
        }

        KAIXO_POLY bend(is_poly auto a, is_poly auto b) noexcept {
            auto c1 = b == 0.f;
            auto c2 = b < 0.f;
            auto d = tanh_like(b);
            return (c1 & ~c2) & a
                | (~c1 & c2) & (1 - tanh_like(b - b * a) / d)
                | (~c1 & ~c2) & (tanh_like(b * a) / d);
        }
        KAIXO_STEREO(bend)

        // ------------------------------------------------

        KAIXO_MONO sin(is_mono auto x) noexcept { return std::sin(x); }
        KAIXO_POLY sin(is_poly auto x) noexcept { return x.sin(); }
        KAIXO_STEREO(sin)
        KAIXO_MONO nsin(is_mono auto x) noexcept { return sin(x * std::numbers::pi_v<float> * 2); }
        KAIXO_POLY nsin(is_poly auto x) noexcept { return (x * (std::numbers::pi_v<float> * 2)).sin(); }
        KAIXO_STEREO(nsin)
        KAIXO_MONO sinh(is_mono auto x) noexcept { return std::sinh(x); }
        KAIXO_POLY sinh(is_poly auto x) noexcept { return x.sinh(); }
        KAIXO_STEREO(sinh)
        KAIXO_MONO sinHalf(is_mono auto x) noexcept { return 0.5f * sin(x); }
        KAIXO_POLY sinHalf(is_poly auto x) noexcept { return 0.5f * x.sin(); }
        KAIXO_STEREO(sinHalf)
        KAIXO_MONO nsinHalf(is_mono auto x) noexcept { return 0.5f * nsin(x); }
        KAIXO_POLY nsinHalf(is_poly auto x) noexcept { return 0.5f * nsin(x); }
        KAIXO_STEREO(nsinHalf)
        KAIXO_MONO cos(is_mono auto x) noexcept { return std::cos(x); }
        KAIXO_POLY cos(is_poly auto x) noexcept { return x.cos(); }
        KAIXO_STEREO(cos)
        KAIXO_MONO ncos(is_mono auto x) noexcept { return cos(x * std::numbers::pi_v<float> * 2); }
        KAIXO_POLY ncos(is_poly auto x) noexcept { return cos(x * std::numbers::pi_v<float> * 2); }
        KAIXO_STEREO(ncos)
        KAIXO_MONO cosh(is_mono auto x) noexcept { return std::cosh(x); }
        KAIXO_POLY cosh(is_poly auto x) noexcept { return x.cosh(); }
        KAIXO_STEREO(cosh)

        // ------------------------------------------------
            
        KAIXO_MONO mod(is_mono auto a, is_mono auto b) noexcept { return a % b; }
        KAIXO_POLY mod(is_poly auto a, is_mono auto b) noexcept { return a - (a / b) * b; }
        KAIXO_POLY mod(is_poly auto a, is_poly auto b) noexcept { return a - (a / b) * b; }

        KAIXO_MONO fmod(is_mono auto a, is_mono auto b) noexcept { return std::fmod(a, b); }
        KAIXO_POLY fmod(is_poly auto a, is_mono auto b) noexcept { return a - trunc(a / b) * b; }
        KAIXO_POLY fmod(is_poly auto a, is_poly auto b) noexcept { return a - trunc(a / b) * b; }
        KAIXO_STEREO(fmod)

        KAIXO_MONO fmod1(is_mono auto a) noexcept { return fmod(a, (decltype(a))1); }
        KAIXO_POLY fmod1(is_poly auto a) noexcept { return a - trunc(a); }
        KAIXO_STEREO(fmod1)

        // ------------------------------------------------

        KAIXO_MONO db_to_magnitude(is_mono auto x) { return pow(10.f, 0.05f * x); }
        KAIXO_POLY db_to_magnitude(is_poly auto x) { return pow(10.f, 0.05f * x); }
        KAIXO_STEREO(db_to_magnitude)

        KAIXO_MONO magnitude_to_db(is_mono auto x) { return (20.f * log10(x)); }
        KAIXO_POLY magnitude_to_db(is_poly auto x) { return (20.f * log10(x)); }
        KAIXO_STEREO(magnitude_to_db)

        KAIXO_MONO magnitude_to_log(is_mono auto x, is_mono auto y0, is_mono auto y1) { return y0 / exp(log(y0 / y1) * x); }
        KAIXO_POLY magnitude_to_log(is_poly auto x, is_poly auto y0, is_poly auto y1) { return y0 / exp(log(y0 / y1) * x); }
        KAIXO_STEREO(magnitude_to_log)

        template<auto y0, auto y1> KAIXO_MONO magnitude_to_log(is_mono auto x) { return y0 / exp(log(y0 / y1) * x); }
        template<auto y0, auto y1> KAIXO_POLY magnitude_to_log(is_poly auto x) { return y0 / exp(log(y0 / y1) * x); }

        KAIXO_MONO log_to_magnitude(is_mono auto x, is_mono auto y0, is_mono auto y1) { return -log(x / y0) / log(y0 / y1); }
        KAIXO_POLY log_to_magnitude(is_poly auto x, is_poly auto y0, is_poly auto y1) { return -log(x / y0) / log(y0 / y1); }
        KAIXO_STEREO(log_to_magnitude)

        template<auto y0, auto y1> KAIXO_MONO log_to_magnitude(is_mono auto x) { return -log(x / y0) / log(y0 / y1); }
        template<auto y0, auto y1> KAIXO_POLY log_to_magnitude(is_poly auto x) { return -log(x / y0) / log(y0 / y1); }

        // ------------------------------------------------

        KAIXO_MONO sign(is_mono auto x) noexcept { return x > 0 ? 1 : -1; }
        KAIXO_STEREO(sign)

        // ------------------------------------------------

        KAIXO_MONO smoothCoef(is_mono auto x, is_mono auto r) noexcept { return pow(x, r); }
        KAIXO_POLY smoothCoef(is_poly auto x, is_poly auto r) noexcept { return pow(x, r); }
        KAIXO_STEREO(smoothCoef)

        // ------------------------------------------------
            
        KAIXO_MONO curve(is_mono auto x, is_mono auto c) noexcept {
            if (c == 0) return x;
            const auto tanhc = tanh(c);
            const auto cx = c * x;
            return c < 0 ? 1 - tanh(c - cx) / tanhc : tanh(cx) / tanhc;
        }
        
        KAIXO_POLY curve(is_poly auto x, is_poly auto c) noexcept {
            const auto tanhc = tanh(c);
            const auto cx = c * x;
            const auto is0 = c == 0;
            const auto less0 = c < 0;

            return (is0 & ~less0 & x) 
                | (~is0 & ~less0 & (tanh(cx) / tanhc))
                | (~is0 &  less0 & (1 - tanh(c - cx) / tanhc));
        }

        KAIXO_STEREO(curve)

        // ------------------------------------------------

        struct Fast {
            
            // ------------------------------------------------
        
            KAIXO_MONO saw(is_mono auto x, is_mono auto nf /*norm freq [0-1]*/) noexcept {
                constexpr auto f = [](auto x) {
                    constexpr auto g = [](auto x) {
                        return ((x * x - 1.f) * x) / 6.f;
                    };

                    return g(fmod1(x + 3) * 2 - 1);
                };

                return (f(x - 2 * nf) + f(x) - 2 * f(x - nf)) * 0.25f * (1.f / nf) * (1.f / nf);
            }
        
            KAIXO_POLY saw(is_poly auto x, is_poly auto nf /*norm freq [0-1]*/) noexcept {
                constexpr auto f = [](auto x) {
                    constexpr auto g = [](auto x) {
                        return ((x * x - 1.f) * x) / 6.f;
                    };

                    return g(fmod1(x + 3) * 2 - 1);
                };

                return (f(x - 2 * nf) + f(x) - 2 * f(x - nf)) * 0.25f * (1.f / nf) * (1.f / nf);
            }
            KAIXO_STEREO(saw)

            // ------------------------------------------------

            KAIXO_MONO trunc(is_mono auto x) noexcept { return (decltype(x))(std::int64_t)x; }
            KAIXO_POLY trunc(is_poly auto x) noexcept { return x.trunc(); }
            KAIXO_STEREO(trunc)

            KAIXO_MONO floor(is_mono auto x) noexcept { return (decltype(x))((std::int64_t)x) - (((std::int64_t)x) > x); }
            KAIXO_POLY floor(is_poly auto x) noexcept { auto t = trunc(x); return t + (-0.f & t > x); }
            KAIXO_STEREO(floor)
            
            KAIXO_MONO round(is_mono auto x) noexcept { return std::round(x); }
            KAIXO_POLY round(is_poly auto x) noexcept { return x.round(); }
            KAIXO_STEREO(round)

            KAIXO_MONO ceil(is_mono auto x) noexcept { return (decltype(x))((std::int64_t)x) + (((std::int64_t)x) > x); }
            KAIXO_POLY ceil(is_poly auto x) noexcept { auto t = trunc(x); return t + (0.f & t > x); }
            KAIXO_STEREO(ceil)

            KAIXO_MONO abs(is_mono auto x) noexcept { return x > 0 ? x : -x; }
            KAIXO_MONO abs(is_poly auto x) noexcept { return x.abs(); }
            KAIXO_STEREO(abs)

            // ------------------------------------------------

            KAIXO_MONO max(is_mono auto a, is_mono auto b) noexcept { return a > b ? a : b; }
            KAIXO_POLY max(is_poly auto a, is_poly auto b) noexcept { auto m = a > b; return m & a | ~m & b; }
            KAIXO_STEREO(max)

            KAIXO_MONO min(is_mono auto a, is_mono auto b) noexcept { return a < b ? a : b; }
            KAIXO_POLY min(is_poly auto a, is_poly auto b) noexcept { auto m = a < b; return m & a | ~m & b; }
            KAIXO_STEREO(min)

            KAIXO_MONO clamp(is_mono auto a, is_mono auto b, is_mono auto c) noexcept { return a < b ? b : a > c ? c : a; }
            KAIXO_POLY clamp(is_poly auto a, is_mono auto b, is_mono auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
            KAIXO_POLY clamp(is_poly auto a, is_poly auto b, is_mono auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
            KAIXO_POLY clamp(is_poly auto a, is_mono auto b, is_poly auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
            KAIXO_POLY clamp(is_poly auto a, is_poly auto b, is_poly auto c) noexcept { auto m = a < b; auto n = a > c; return m & b | n & c | ~m & ~n & a; }
            KAIXO_STEREO(clamp)

            KAIXO_MONO clamp1(is_mono auto a) noexcept { return a < 0 ? 0 : a > 1 ? 1 : a; }
            KAIXO_POLY clamp1(is_poly auto a) noexcept { auto m = a < 0.f; auto n = a > 1.f; return n & 1.f | ~m & ~n & a; }
            KAIXO_STEREO(clamp1)

            KAIXO_MONO clamp11(is_mono auto a) noexcept { return a < -1 ? -1 : a > 1 ? 1 : a; }
            KAIXO_POLY clamp11(is_poly auto a) noexcept { auto m = a < -1.f; auto n = a > 1.f; return m & -1.f | n & 1.f | ~m & ~n & a; }
            KAIXO_STEREO(clamp11)

            // ------------------------------------------------

            KAIXO_MONO tanh(is_mono auto x) noexcept {
                if (x < -4.644f) return -1.f;
                if (x > +4.644f) return +1.f;
                float xx = x * x;
                float xxxx = xx * xx;
                float xxxxxx = xxxx * xx;
                float result = (x * (10.f + xx) * (60.f + xx)) / (600.f + 270.f * xx + 11.f * xxxx + (xxxxxx / 24.f));
                return result;
            }

            KAIXO_POLY tanh(is_poly auto x) noexcept {
                auto xx = x * x;
                auto xxxx = xx * xx;
                auto xxxxxx = xxxx * xx;
                auto result = (x * (10.f + xx) * (60.f + xx)) / (600.f + 270.f * xx + 11.f * xxxx + (xxxxxx / 24.f));
                auto mask1 = x > +4.644f;
                auto mask2 = x < -4.644f;
                return (mask1 & ~mask2) & 1.f
                    | (mask2 & ~mask1) & -1.f
                    | (~mask1 & ~mask2) & result;
            }
            KAIXO_STEREO(tanh)

            KAIXO_MONO tanh_like(is_mono auto x) noexcept { return x / (1.f + abs(x)); }
            KAIXO_POLY tanh_like(is_poly auto x) noexcept { return x / (1.f + abs(x)); }
            KAIXO_STEREO(tanh_like)

            // ------------------------------------------------

            KAIXO_MONO exp2(is_mono auto x) noexcept {
                constexpr float kCoefficient0 = 1.0f;
                constexpr float kCoefficient1 = 16970.0 / 24483.0;
                constexpr float kCoefficient2 = 1960.0 / 8161.0;
                constexpr float kCoefficient3 = 1360.0 / 24483.0;
                constexpr float kCoefficient4 = 80.0 / 8161.0;
                constexpr float kCoefficient5 = 32.0 / 24483.0;

                int integer = (int)x;
                float t = x - (float)integer;
                float int_pow = std::bit_cast<float>((integer + 127) << 23);

                float cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
                float interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
                return int_pow * interpolate;
            }

            KAIXO_POLY exp2(is_poly auto x) noexcept {
                constexpr float kCoefficient0 = 1.0f;
                constexpr float kCoefficient1 = 16970.0 / 24483.0;
                constexpr float kCoefficient2 = 1960.0 / 8161.0;
                constexpr float kCoefficient3 = 1360.0 / 24483.0;
                constexpr float kCoefficient4 = 80.0 / 8161.0;
                constexpr float kCoefficient5 = 32.0 / 24483.0;

                auto integer = x.template cast<int>();
                auto t = x - integer.template cast<float>();
                auto int_pow = ((integer + 127) << 23).template reinterpret<float>();

                auto cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
                auto interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
                return int_pow * interpolate;
            }
            KAIXO_STEREO(exp2)

            KAIXO_MONO log2(is_mono auto x) {
                constexpr float kCoefficient0 = -1819.0 / 651.0;
                constexpr float kCoefficient1 = 5.0;
                constexpr float kCoefficient2 = -10.0 / 3.0;
                constexpr float kCoefficient3 = 10.0 / 7.0;
                constexpr float kCoefficient4 = -1.0 / 3.0;
                constexpr float kCoefficient5 = 1.0 / 31.0;

                int as_int = std::bit_cast<int>((float)x);
                int floored_log2 = (as_int >> 23) - 0x7f;
                float t = std::bit_cast<float>((as_int & 0x7fffff) | (0x7f << 23));

                float cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
                float interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
                return static_cast<float>(floored_log2) + interpolate;
            }

            KAIXO_POLY log2(is_poly auto x) {
                constexpr float kCoefficient0 = -1819.0 / 651.0;
                constexpr float kCoefficient1 = 5.0;
                constexpr float kCoefficient2 = -10.0 / 3.0;
                constexpr float kCoefficient3 = 10.0 / 7.0;
                constexpr float kCoefficient4 = -1.0 / 3.0;
                constexpr float kCoefficient5 = 1.0 / 31.0;

                auto floored_log2 = (x.template reinterpret<int>() >> 23u) - 127;
                auto t = (x & std::bit_cast<float>(0x7fffff)) | std::bit_cast<float>(127 << 23);

                auto cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
                auto interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
                return floored_log2.template cast<float>() + interpolate;
            }
            KAIXO_STEREO(log2)

            KAIXO_MONO exp(is_mono auto x) noexcept { return exp2(x * std::numbers::log2e_v<float>); }
            KAIXO_POLY exp(is_poly auto x) noexcept { return exp2(x * std::numbers::log2e_v<float>); }
            KAIXO_STEREO(exp)
            KAIXO_MONO log(is_mono auto x) noexcept { return log2(x) * std::numbers::ln2_v<float>; }
            KAIXO_POLY log(is_poly auto x) noexcept { return log2(x) * std::numbers::ln2_v<float>; }
            KAIXO_STEREO(log)
            KAIXO_MONO log10(is_mono auto x) noexcept { return log2(x) * (std::numbers::log10e_v<float> *std::numbers::ln2_v<float>); }
            KAIXO_POLY log10(is_poly auto x) noexcept { return log2(x) * (std::numbers::log10e_v<float> *std::numbers::ln2_v<float>); }
            KAIXO_STEREO(log10)

            KAIXO_MONO pow(is_mono auto a, is_mono auto b) noexcept {
                struct int2 { int x[2]; };
                int2 h = std::bit_cast<int2>(static_cast<double>(a));
                h.x[1] = (int)(b * (h.x[1] - 1072632447) + 1072632447), h.x[0] = 0;
                return (float)std::bit_cast<double>(h);
            }

            KAIXO_POLY pow(is_poly auto a, is_poly auto b) noexcept { return exp2(log2(a) * b); }

            template<std::size_t N> KAIXO_MONO powN(is_mono auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return ((Is, a) * ...); }(std::make_index_sequence<N>{}); }
            template<std::size_t N> KAIXO_POLY powN(is_poly auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return ((Is, a) * ...); }(std::make_index_sequence<N>{}); }
            template<std::size_t N> KAIXO_STEREO_T(powN, powN<N>)
            template<std::size_t N> KAIXO_MONO invPowN(is_mono auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return 1 - ((Is, (1 - a)) * ...); }(std::make_index_sequence<N>{}); }
            template<std::size_t N> KAIXO_POLY invPowN(is_poly auto a) noexcept { return[&]<std::size_t ...Is>(std::index_sequence<Is...>) { return 1 - ((Is, (1 - a)) * ...); }(std::make_index_sequence<N>{}); }
            template<std::size_t N> KAIXO_STEREO_T(invPowN, invPowN<N>)

            KAIXO_MONO sqrt(is_mono auto a) noexcept { return std::sqrt(a); }
            KAIXO_POLY sqrt(is_poly auto a) noexcept { return a.sqrt(); }
            KAIXO_STEREO(sqrt)

            // ------------------------------------------------

            KAIXO_MONO bend(is_mono auto a, is_mono auto b) noexcept {
                return b == 0 ? a
                    : b < 0 ? 1 - tanh_like(b - b * a) / tanh_like(b)
                    : tanh_like(b * a) / tanh_like(b);
            }

            KAIXO_POLY bend(is_poly auto a, is_poly auto b) noexcept {
                auto c1 = b == 0.f;
                auto c2 = b < 0.f;
                auto d = tanh_like(b);
                return (c1 & ~c2) & a
                    | (~c1 & c2) & (1 - tanh_like(b - b * a) / d)
                    | (~c1 & ~c2) & (tanh_like(b * a) / d);
            }
            KAIXO_STEREO(bend)

            // ------------------------------------------------

            // Requires input between -0.5 and 0.5!
            KAIXO_MONO nsin(is_mono auto x) noexcept {
                auto approx = x * (8.0f - 16.0f * abs(x));
                return approx * (0.776f + 0.224f * abs(approx));
            }

            // Requires input between -0.5 and 0.5!
            KAIXO_POLY nsin(is_poly auto x) noexcept { return x.fast_normalized_sin(); }
            KAIXO_STEREO(nsin)

            // Requires input between -0.5 and 0.5!
            KAIXO_MONO nsinHalf(is_mono auto x) noexcept {
                auto approx = x * (4.0f - 8.0f * abs(x));
                return approx * (0.338f + 0.112f * abs(approx));
            }

            // Requires input between -0.5 and 0.5!
            KAIXO_POLY nsinHalf(is_poly auto x) noexcept {
                auto approx = x * (4.0f - 8.0f * abs(x));
                return approx * (0.338f + 0.112f * abs(approx));
            }
            KAIXO_STEREO(nsinHalf)

            // Phase 0 -> 1
            KAIXO_MONO ncos(is_mono auto x) noexcept {
                x = x - (.25f + floor(x + .25f));
                x = x * (16.f * (abs(x) - .5f));
                x = x + (.225f * x * (abs(x) - 1.f));
                return x;
            }

            // Phase 0 -> 1
            KAIXO_POLY ncos(is_poly auto x) noexcept {
                x = x - (.25f + floor(x + .25f));
                x = x * (16.f * (abs(x) - .5f));
                x = x + (.225f * x * (abs(x) - 1.f));
                return x;
            }
            KAIXO_STEREO(ncos)

            KAIXO_MONO sin(is_mono auto x) noexcept { return std::sin(x); }
            KAIXO_POLY sin(is_poly auto x) noexcept { return x.sin(); }
            KAIXO_STEREO(sin)
            KAIXO_MONO sinh(is_mono auto x) noexcept { return std::sinh(x); }
            KAIXO_POLY sinh(is_poly auto x) noexcept { return x.sinh(); }
            KAIXO_STEREO(sinh)
            KAIXO_MONO sinHalf(is_mono auto x) noexcept { return 0.5f * std::sin(x); }
            KAIXO_POLY sinHalf(is_poly auto x) noexcept { return 0.5f * x.sin(); }
            KAIXO_STEREO(sinHalf)
            KAIXO_MONO cos(is_mono auto x) noexcept { return std::cos(x); }
            KAIXO_POLY cos(is_poly auto x) noexcept { return x.cos(); }
            KAIXO_STEREO(cos)
            KAIXO_MONO cosh(is_mono auto x) noexcept { return std::cosh(x); }
            KAIXO_POLY cosh(is_poly auto x) noexcept { return x.cosh(); }
            KAIXO_STEREO(cosh)

            // ------------------------------------------------

            KAIXO_MONO mod(is_mono auto a, is_mono auto b) noexcept { return a % b; }
            KAIXO_POLY mod(is_poly auto a, is_mono auto b) noexcept { return a - (a / b) * b; }
            KAIXO_POLY mod(is_poly auto a, is_poly auto b) noexcept { return a - (a / b) * b; }

            KAIXO_MONO fmod(is_mono auto a, is_mono auto b) noexcept { return a - trunc(a / b) * b; }
            KAIXO_POLY fmod(is_poly auto a, is_mono auto b) noexcept { return a - trunc(a / b) * b; }
            KAIXO_POLY fmod(is_poly auto a, is_poly auto b) noexcept { return a - trunc(a / b) * b; }
            KAIXO_STEREO(fmod)

            KAIXO_MONO fmod1(is_mono auto a) noexcept { return a - trunc(a); }
            KAIXO_POLY fmod1(is_poly auto a) noexcept { return a.fmod1(); }
            KAIXO_STEREO(fmod1)

            // ------------------------------------------------

            KAIXO_MONO db_to_magnitude(is_mono auto x) { return pow(10.f, 0.05f * x); }
            KAIXO_POLY db_to_magnitude(is_poly auto x) { return pow(10.f, 0.05f * x); }
            KAIXO_STEREO(db_to_magnitude)

            KAIXO_MONO magnitude_to_db(is_mono auto x) { return (20.f * log10(x)); }
            KAIXO_POLY magnitude_to_db(is_poly auto x) { return (20.f * log10(x)); }
            KAIXO_STEREO(magnitude_to_db)

            KAIXO_MONO magnitude_to_log(is_mono auto x, is_mono auto y0, is_mono auto y1) { return y0 / exp(log(y0 / y1) * x); }
            KAIXO_POLY magnitude_to_log(is_poly auto x, is_poly auto y0, is_poly auto y1) { return y0 / exp(log(y0 / y1) * x); }
            KAIXO_STEREO(magnitude_to_log)

            template<auto y0, auto y1> KAIXO_MONO magnitude_to_log(is_mono auto x) { return y0 / exp(log(y0 / y1) * x); }
            template<auto y0, auto y1> KAIXO_POLY magnitude_to_log(is_poly auto x) { return y0 / exp(log(y0 / y1) * x); }
            
            KAIXO_MONO log_to_magnitude(is_mono auto x, is_mono auto y0, is_mono auto y1) { return -log(x / y0) / log(y0 / y1); }
            KAIXO_POLY log_to_magnitude(is_poly auto x, is_poly auto y0, is_poly auto y1) { return -log(x / y0) / log(y0 / y1); }
            KAIXO_STEREO(log_to_magnitude)

            template<auto y0, auto y1> KAIXO_MONO log_to_magnitude(is_mono auto x) { return -log(x / y0) / log(y0 / y1); }
            template<auto y0, auto y1> KAIXO_POLY log_to_magnitude(is_poly auto x) { return -log(x / y0) / log(y0 / y1); }

            // ------------------------------------------------

            KAIXO_MONO sign(is_mono auto x) noexcept { return x > 0 ? 1 : -1; }
            KAIXO_MONO sign(is_poly auto x) noexcept { 
                const auto cnd = 0.f < x;
                return cnd & 1.f | ~cnd & -1.f;
            }
            KAIXO_STEREO(sign)

            // ------------------------------------------------

            KAIXO_MONO smoothCoef(is_mono auto x, is_mono auto r) noexcept { return pow(x, r); }
            KAIXO_POLY smoothCoef(is_poly auto x, is_poly auto r) noexcept { return pow(x, r); }
            KAIXO_STEREO(smoothCoef)

            // ------------------------------------------------

            KAIXO_MONO curve(is_mono auto x, is_mono auto c) noexcept {
                if (c == 0) return x;
                const auto tanhc = tanh(c);
                const auto cx = c * x;
                return c < 0 ? 1 - tanh(c - cx) / tanhc : tanh(cx) / tanhc;
            }

            KAIXO_POLY curve(is_poly auto x, is_poly auto c) noexcept {
                const auto tanhc = tanh(c);
                const auto cx = c * x;
                const auto is0 = c == 0;
                const auto less0 = c < 0;

                return (is0 & ~less0 & x)
                    | (~is0 & ~less0 & (tanh(cx) / tanhc))
                    | (~is0 & less0 & (1 - tanh(c - cx) / tanhc));
            }

            KAIXO_STEREO(curve)

            // ------------------------------------------------

        };

        // ------------------------------------------------

    };

    // ------------------------------------------------

    constexpr float noteToFreq(float note) { return 440.f * Math::Fast::exp2(((note - 69) / 12.f)); }

    // ------------------------------------------------

}
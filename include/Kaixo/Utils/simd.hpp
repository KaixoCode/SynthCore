#pragma once
#include <algorithm>
#include <bit>
#include <concepts>
#include <type_traits>
#include <numeric>

#include <immintrin.h>

// ------------------------------------------------

#define KAIXO_VECTORCALL __vectorcall
#define KAIXO_INLINE __forceinline

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    template<class Ty, std::size_t Bits> struct underlying_simd;
    template<> struct underlying_simd<int, 128> : std::type_identity<__m128i> {};
    template<> struct underlying_simd<int, 256> : std::type_identity<__m256i> {};
    template<> struct underlying_simd<int, 512> : std::type_identity<__m512i> {};
    template<> struct underlying_simd<float, 128> : std::type_identity<__m128> {};
    template<> struct underlying_simd<float, 256> : std::type_identity<__m256> {};
    template<> struct underlying_simd<float, 512> : std::type_identity<__m512> {};
    template<class Ty, std::size_t Bits> using underlying_simd_t = typename underlying_simd<Ty, Bits>::type;

    // ------------------------------------------------

    template<class Ty>
    concept simd_type = std::floating_point<Ty> || std::integral<Ty>;

    template<class Ty>
    concept is_simd = requires () { typename Ty::base; typename Ty::type; { Ty::bits }; };

    enum simd_capability {
        SSE      = 1 <<  0, // Always required
        SSE2     = 1 <<  1,
        SSE3     = 1 <<  2,
        SSE4p1   = 1 <<  3,
        AVX      = 1 <<  4,
        FMA      = 1 <<  5,
        AVX2     = 1 <<  6,
        AVX512F  = 1 <<  7,
        AVX512BW = 1 <<  8,
        AVX512VL = 1 <<  9,
        AVX512CD = 1 << 10,
        AVX512DQ = 1 << 11,
    };

    template<class Type, std::size_t Bits, simd_capability Capabilities>
    struct simd_add {
        constexpr static simd_capability capabilities = Capabilities;
        using type = Type;
        using simd_type = underlying_simd_t<Type, Bits>;
        KAIXO_INLINE simd_type KAIXO_VECTORCALL operator()(const simd_type& a, const simd_type& b) const noexcept {
            if constexpr (std::same_as<type, float>) {
                if constexpr (Bits == 128) {
                    if constexpr (Capabilities & SSE) return _mm_add_ps(a, b);
                } else if constexpr (Bits == 256) {
                    if constexpr (Capabilities & AVX) return _mm256_add_ps(a, b);
                } else if constexpr (Bits == 512) {
                    if constexpr (Capabilities & AVX512F) return _mm512_add_ps(a, b);
                }
            }
        }
    };

    constexpr std::size_t determine_highest_supported_bit_count(simd_capability capabilities) {
        if (capabilities & AVX512F) return 512;
        else if (capabilities & AVX) return 256;
        else if (capabilities & SSE) return 128;
    }

    template<class = float, simd_capability>
    class specialized_simd;

    template<simd_capability Capabilities>
    class specialized_simd<float, Capabilities> {
    public:

        constexpr static std::size_t bits = determine_highest_supported_bit_count(Capabilities);
        using type = float;
        using simd_type = underlying_simd_t<type, bits>;

    };


    struct simd_path {

        enum _path {
            s0, s128, s256, s512
        };

        static inline _path path = s256;

    };

    // ------------------------------------------------

    template<class = float, std::size_t = 256>
    struct simd;

    // ------------------------------------------------

#define KAIXO_FROM_MASK512(NAME, TYPE, MASK, FUN, UNDERLYING)\
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) {                                    \
        constexpr UNDERLYING all_mask = std::bit_cast<UNDERLYING>(0xffffffff);              \
        return FUN(mask & (1ull <<  0) ? all_mask : 0, mask & (1ull <<  1) ? all_mask : 0,  \
                   mask & (1ull <<  2) ? all_mask : 0, mask & (1ull <<  3) ? all_mask : 0,  \
                   mask & (1ull <<  4) ? all_mask : 0, mask & (1ull <<  5) ? all_mask : 0,  \
                   mask & (1ull <<  6) ? all_mask : 0, mask & (1ull <<  7) ? all_mask : 0,  \
                   mask & (1ull <<  8) ? all_mask : 0, mask & (1ull <<  9) ? all_mask : 0,  \
                   mask & (1ull << 10) ? all_mask : 0, mask & (1ull << 11) ? all_mask : 0,  \
                   mask & (1ull << 12) ? all_mask : 0, mask & (1ull << 13) ? all_mask : 0,  \
                   mask & (1ull << 14) ? all_mask : 0, mask & (1ull << 15) ? all_mask : 0); \
    }
#define KAIXO_FROM_MASK256(NAME, TYPE, MASK, FUN, UNDERLYING)\
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) {                                    \
        constexpr UNDERLYING all_mask = std::bit_cast<UNDERLYING>(0xffffffff);              \
        return FUN(mask & (1ull <<  0) ? all_mask : 0, mask & (1ull <<  1) ? all_mask : 0,  \
                   mask & (1ull <<  2) ? all_mask : 0, mask & (1ull <<  3) ? all_mask : 0,  \
                   mask & (1ull <<  4) ? all_mask : 0, mask & (1ull <<  5) ? all_mask : 0,  \
                   mask & (1ull <<  6) ? all_mask : 0, mask & (1ull <<  7) ? all_mask : 0); \
    }
#define KAIXO_FROM_MASK128(NAME, TYPE, MASK, FUN, UNDERLYING)\
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) {                                    \
        constexpr UNDERLYING all_mask = std::bit_cast<UNDERLYING>(0xffffffff);              \
        return FUN(mask & (1ull <<  0) ? all_mask : 0, mask & (1ull <<  1) ? all_mask : 0,  \
                   mask & (1ull <<  2) ? all_mask : 0, mask & (1ull <<  3) ? all_mask : 0); \
    }

    KAIXO_FROM_MASK128(_mm_from_mask_ps, __m128, __mmask8, _mm_setr_ps, float);
    KAIXO_FROM_MASK128(_mm_from_mask_epi32, __m128i, __mmask8, _mm_setr_epi32, int);
    KAIXO_FROM_MASK256(_mm256_from_mask_ps, __m256, __mmask8, _mm256_setr_ps, float);
    KAIXO_FROM_MASK256(_mm256_from_mask_epi32, __m256i, __mmask8, _mm256_setr_epi32, int);
    KAIXO_FROM_MASK512(_mm512_from_mask_ps, __m512, __mmask16, _mm512_setr_ps, float);
    KAIXO_FROM_MASK512(_mm512_from_mask_epi32, __m512i, __mmask16, _mm512_setr_epi32, int);

    // ------------------------------------------------

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator-(__m128i a, __m128i b) noexcept { return _mm_sub_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator-(__m256i a, __m256i b) noexcept { return _mm256_sub_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator-(__m512i a, __m512i b) noexcept { return _mm512_sub_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator-(__m128 a, __m128 b) noexcept { return _mm_sub_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator-(__m256 a, __m256 b) noexcept { return _mm256_sub_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator-(__m512 a, __m512 b) noexcept { return _mm512_sub_ps(a, b); }

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator*(__m128i a, __m128i b) noexcept { return _mm_mullo_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator*(__m256i a, __m256i b) noexcept { return _mm256_mullo_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator*(__m512i a, __m512i b) noexcept { return _mm512_mullo_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator*(__m128 a, __m128 b) noexcept { return _mm_mul_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator*(__m256 a, __m256 b) noexcept { return _mm256_mul_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator*(__m512 a, __m512 b) noexcept { return _mm512_mul_ps(a, b); }

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator+(__m128i a, __m128i b) noexcept { return _mm_add_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator+(__m256i a, __m256i b) noexcept { return _mm256_add_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator+(__m512i a, __m512i b) noexcept { return _mm512_add_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator+(__m128 a, __m128 b) noexcept { return _mm_add_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator+(__m256 a, __m256 b) noexcept { return _mm256_add_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator+(__m512 a, __m512 b) noexcept { return _mm512_add_ps(a, b); }

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator/(__m128i a, __m128i b) noexcept { return _mm_div_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator/(__m256i a, __m256i b) noexcept { return _mm256_div_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator/(__m512i a, __m512i b) noexcept { return _mm512_div_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator/(__m128 a, __m128 b) noexcept { return _mm_div_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator/(__m256 a, __m256 b) noexcept { return _mm256_div_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator/(__m512 a, __m512 b) noexcept { return _mm512_div_ps(a, b); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator&(__m128i a, __m128i b) noexcept { return _mm_and_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator&(__m256i a, __m256i b) noexcept { return _mm256_and_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator&(__m512i a, __m512i b) noexcept { return _mm512_and_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator&(__m128 a, __m128 b) noexcept { return _mm_and_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator&(__m256 a, __m256 b) noexcept { return _mm256_and_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator&(__m512 a, __m512 b) noexcept { return _mm512_and_ps(a, b); }

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator|(__m128i a, __m128i b) noexcept { return _mm_or_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator|(__m256i a, __m256i b) noexcept { return _mm256_or_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator|(__m512i a, __m512i b) noexcept { return _mm512_or_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator|(__m128 a, __m128 b) noexcept { return _mm_or_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator|(__m256 a, __m256 b) noexcept { return _mm256_or_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator|(__m512 a, __m512 b) noexcept { return _mm512_or_ps(a, b); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator^(__m128i a, __m128i b) noexcept { return _mm_xor_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator^(__m256i a, __m256i b) noexcept { return _mm256_xor_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator^(__m512i a, __m512i b) noexcept { return _mm512_xor_epi32(a, b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator^(__m128 a, __m128 b) noexcept { return _mm_xor_ps(a, b); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator^(__m256 a, __m256 b) noexcept { return _mm256_xor_ps(a, b); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator^(__m512 a, __m512 b) noexcept { return _mm512_xor_ps(a, b); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator==(__m128i a, __m128i b) noexcept { return _mm_cmpeq_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator==(__m256i a, __m256i b) noexcept { return _mm256_cmpeq_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator==(__m512i a, __m512i b) noexcept { return _mm512_from_mask_epi32(_mm512_cmpeq_epi32_mask(a, b)); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator==(__m128 a, __m128 b) noexcept { return _mm_cmp_ps(a, b, _CMP_EQ_OS); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator==(__m256 a, __m256 b) noexcept { return _mm256_cmp_ps(a, b, _CMP_EQ_OS); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator==(__m512 a, __m512 b) noexcept { return _mm512_from_mask_ps(_mm512_cmpeq_ps_mask(a, b)); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator>(__m128i a, __m128i b) noexcept { return _mm_cmpgt_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator>(__m256i a, __m256i b) noexcept { return _mm256_cmpgt_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator>(__m512i a, __m512i b) noexcept { return _mm512_from_mask_epi32(_mm512_cmpgt_epi32_mask(a, b)); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator>(__m128 a, __m128 b) noexcept { return _mm_cmp_ps(a, b, _CMP_GT_OS); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator>(__m256 a, __m256 b) noexcept { return _mm256_cmp_ps(a, b, _CMP_GT_OS); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator>(__m512 a, __m512 b) noexcept { return _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_GT_OS)); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator<(__m128i a, __m128i b) noexcept { return (b > a) | (b == a); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator<(__m256i a, __m256i b) noexcept { return (b > a) | (b == a); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator<(__m512i a, __m512i b) noexcept { return (b > a) | (b == a); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator<(__m128 a, __m128 b) noexcept { return _mm_cmp_ps(a, b, _CMP_LT_OS); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator<(__m256 a, __m256 b) noexcept { return _mm256_cmp_ps(a, b, _CMP_LT_OS); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator<(__m512 a, __m512 b) noexcept { return _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_LT_OS)); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator>=(__m128i a, __m128i b) noexcept { return (a > b) | (a == b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator>=(__m256i a, __m256i b) noexcept { return (a > b) | (a == b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator>=(__m512i a, __m512i b) noexcept { return (a > b) | (a == b); }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator>=(__m128 a, __m128 b) noexcept { return _mm_cmp_ps(a, b, _CMP_GE_OS); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator>=(__m256 a, __m256 b) noexcept { return _mm256_cmp_ps(a, b, _CMP_GE_OS); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator>=(__m512 a, __m512 b) noexcept { return _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_GE_OS)); }
    
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator<=(__m128i a, __m128i b) noexcept { return b > a; }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator<=(__m256i a, __m256i b) noexcept { return b > a; }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator<=(__m512i a, __m512i b) noexcept { return b > a; }
    KAIXO_INLINE __m128 KAIXO_VECTORCALL operator<=(__m128 a, __m128 b) noexcept { return _mm_cmp_ps(a, b, _CMP_LE_OS); }
    KAIXO_INLINE __m256 KAIXO_VECTORCALL operator<=(__m256 a, __m256 b) noexcept { return _mm256_cmp_ps(a, b, _CMP_LE_OS); }
    KAIXO_INLINE __m512 KAIXO_VECTORCALL operator<=(__m512 a, __m512 b) noexcept { return _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_LE_OS)); }

    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator<<(__m128i a, __m128i b) noexcept { return _mm_sllv_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator<<(__m256i a, __m256i b) noexcept { return _mm256_sllv_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator<<(__m512i a, __m512i b) noexcept { return _mm512_sllv_epi32(a, b); }
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator<<(__m128i a, int b) noexcept { return _mm_slli_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator<<(__m256i a, int b) noexcept { return _mm256_slli_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator<<(__m512i a, unsigned int b) noexcept { return _mm512_slli_epi32(a, b); }
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator>>(__m128i a, __m128i b) noexcept { return _mm_srlv_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator>>(__m256i a, __m256i b) noexcept { return _mm256_srlv_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator>>(__m512i a, __m512i b) noexcept { return _mm512_srlv_epi32(a, b); }
    KAIXO_INLINE __m128i KAIXO_VECTORCALL operator>>(__m128i a, int b) noexcept { return _mm_srli_epi32(a, b); }
    KAIXO_INLINE __m256i KAIXO_VECTORCALL operator>>(__m256i a, int b) noexcept { return _mm256_srli_epi32(a, b); }
    KAIXO_INLINE __m512i KAIXO_VECTORCALL operator>>(__m512i a, unsigned int b) noexcept { return _mm512_srli_epi32(a, b); }

    // ------------------------------------------------

#define _mm_setzero_epi32() _mm_setzero_si128()
#define _mm256_setzero_epi32() _mm256_setzero_si256()

#define KAIXO_BASIC_SUM(v)                                     \
    [this](auto&) -> auto {                                    \
        base vals[elements];                                   \
        get(vals);                                             \
        return std::accumulate(vals, vals + elements, base{}); \
    }(v)

#define _mm_sum_epi32(v) KAIXO_BASIC_SUM(v)
#define _mm256_sum_epi32(v) KAIXO_BASIC_SUM(v)
#define _mm512_sum_epi32(v) KAIXO_BASIC_SUM(v)

#define _mm_sum_ps(v)                         \
    [this](auto& val) -> auto {               \
        auto shuf = _mm_movehdup_ps(val);     \
        auto sums = _mm_add_ps(val, shuf);    \
        shuf = _mm_movehl_ps(shuf, sums);     \
        sums = _mm_add_ss(sums, shuf);        \
        return _mm_cvtss_f32(sums);           \
    }(v)
    
#define _mm256_sum_ps(v)                                  \
    [this](auto& val) -> auto {                           \
        auto hiQuad = _mm256_extractf128_ps(val, 1);      \
        auto loQuad = _mm256_castps256_ps128(val);        \
        auto sumQuad = _mm_add_ps(loQuad, hiQuad);        \
        auto loDual = sumQuad;                            \
        auto hiDual = _mm_movehl_ps(sumQuad, sumQuad);    \
        auto sumDual = _mm_add_ps(loDual, hiDual);        \
        auto lo = sumDual;                                \
        auto hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);  \
        auto sum = _mm_add_ss(lo, hi);                    \
        return _mm_cvtss_f32(sum);                        \
    }(v)
    
#define _mm512_sum_ps(v)                                \
    [this](auto& val) -> auto {                         \
        const auto v0 = _mm512_castps512_ps256(val);    \
        const auto v1 = _mm512_extractf32x8_ps(val, 1); \
        const auto x0 = _mm256_add_ps(v0, v1);          \
        return simd<float, 256>(x0).sum();              \
    }(v)

    // ------------------------------------------------

#define _mm_gather_ps(ptr, index, size) _mm_i32gather_ps(ptr, index, size)
#define _mm_gather_epi32(ptr, index, size) _mm_i32gather_epi32(ptr, index, size)
#define _mm256_gather_ps(ptr, index, size) _mm256_i32gather_ps(ptr, index, size)
#define _mm256_gather_epi32(ptr, index, size) _mm256_i32gather_epi32(ptr, index, size)
#define _mm512_gather_ps(ptr, index, size) _mm512_i32gather_ps(index, ptr, size)
#define _mm512_gather_epi32(ptr, index, size) _mm512_i32gather_epi32(index, ptr, size)

    // ------------------------------------------------

#define _mm_load_epi32(addr) _mm_load_si128(reinterpret_cast<const __m128i*>(addr))
#define _mm256_load_epi32(addr) _mm256_load_si256(reinterpret_cast<const __m256i*>(addr))

    // ------------------------------------------------

#define _mm_stream_epi32(addr, a) _mm_stream_si128(reinterpret_cast<__m128i*>(addr), a)
#define _mm256_stream_epi32(addr, a) _mm256_stream_si256(reinterpret_cast<__m256i*>(addr), a)
#define _mm512_stream_epi32(addr, a) _mm512_stream_si512(reinterpret_cast<__m512i*>(addr), a)
    
    // ------------------------------------------------

#define _mm_store_epi32(addr, a) _mm_store_si128(reinterpret_cast<__m128i*>(addr), a)
#define _mm256_store_epi32(addr, a) _mm256_store_si256(reinterpret_cast<__m256i*>(addr), a)

    // ------------------------------------------------

#define SIMD_INSTANCE_FLOATING_PART(BASE, TYPE, BITS, PRE, POST)\
        KAIXO_INLINE simd KAIXO_VECTORCALL trunc() const noexcept { return PRE##_trunc_##POST(value); }                                                                   \
        KAIXO_INLINE simd KAIXO_VECTORCALL floor() const noexcept { return PRE##_floor_##POST(value); }                                                                   \
        KAIXO_INLINE simd KAIXO_VECTORCALL ceil() const noexcept { return PRE##_ceil_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL round() const noexcept { return PRE##_roundscale_##POST(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }               \
        KAIXO_INLINE simd KAIXO_VECTORCALL log() const noexcept { return PRE##_log_##POST(value); }                                                                       \
        KAIXO_INLINE simd KAIXO_VECTORCALL log2() const noexcept { return PRE##_log2_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL log10() const noexcept { return PRE##_log10_##POST(value); }                                                                   \
        KAIXO_INLINE simd KAIXO_VECTORCALL sqrt() const noexcept { return PRE##_sqrt_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL cbrt() const noexcept { return PRE##_cbrt_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL exp() const noexcept { return PRE##_exp_##POST(value); }                                                                       \
        KAIXO_INLINE simd KAIXO_VECTORCALL exp2() const noexcept { return PRE##_exp2_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL exp10() const noexcept { return PRE##_exp10_##POST(value); }                                                                   \
        KAIXO_INLINE simd KAIXO_VECTORCALL tanh() const noexcept { return PRE##_tanh_##POST(value); }                                                                     \
        KAIXO_INLINE simd KAIXO_VECTORCALL abs() const noexcept { return PRE##_andnot_##POST(PRE##_set1_##POST(-0.0), value); }                                           \
        KAIXO_INLINE simd KAIXO_VECTORCALL cos() const noexcept { return PRE##_cos_##POST(value); }                                                                       \
        KAIXO_INLINE simd KAIXO_VECTORCALL cosh() const noexcept { return PRE##_cosh_##POST(value); }                                                                       \
        KAIXO_INLINE simd KAIXO_VECTORCALL sin() const noexcept { return PRE##_sin_##POST(value); }                                                                       \
        KAIXO_INLINE simd KAIXO_VECTORCALL sinh() const noexcept { return PRE##_sinh_##POST(value); }                                                                       \
        KAIXO_INLINE std::pair<simd, simd> KAIXO_VECTORCALL sincos() const noexcept { simd cos; simd sin = PRE##_sincos_##POST(&cos.value, value); return { sin, cos }; } \
        KAIXO_INLINE simd KAIXO_VECTORCALL copysign(const simd& b) noexcept { return (-0.f & *this) | (~simd(-0.f) & b); }                                                \
        KAIXO_INLINE simd KAIXO_VECTORCALL pow(const simd& b) const noexcept { return PRE##_pow_##POST(value, b.value); }                                                 \

#define SIMD_INSTANCE_FLOAT_PART(BASE, TYPE, BITS, PRE, POST)\
        template<class Ty> KAIXO_INLINE simd<Ty, BITS> to() const noexcept { return PRE##_cvtt##POST##_epi32(value); }             \
        template<class Ty> KAIXO_INLINE simd<Ty, BITS> reinterpret() const noexcept { return PRE##_cast##POST##_si##BITS(value); } \
        template<std::size_t N = 1> KAIXO_INLINE simd<float,  sizeof(float)  * elements> lookup(const float*  data) const noexcept { return PRE##_gather_ps(data, to<int>().value, N * sizeof(float)); }  \
        template<std::size_t N = 1> KAIXO_INLINE simd<int,    sizeof(int)    * elements> lookup(const int*    data) const noexcept { return PRE##_gather_epi32(data, to<int>().value, N * sizeof(int)); } \
        SIMD_INSTANCE_FLOATING_PART(BASE, TYPE, BITS, PRE, POST)

#define SIMD_INSTANCE_INT_PART(BASE, TYPE, BITS, PRE, POST)\
        template<class Ty> KAIXO_INLINE simd<Ty, BITS> to() const noexcept { return PRE##_cvt##POST##_ps(value); }                 \
        template<class Ty> KAIXO_INLINE simd<Ty, BITS> reinterpret() const noexcept { return PRE##_castsi##BITS##_ps(value); }     \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<<(const simd& a, const simd& b) noexcept { return a.value << b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>>(const simd& a, const simd& b) noexcept { return a.value >> b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<<(const simd& a, int b) noexcept { return a.value << b; }               \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>>(const simd& a, int b) noexcept { return a.value >> b; }               \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL abs(const simd& v) noexcept { return PRE##_abs_##POST(v.value); }                \
        template<std::size_t N = 1> KAIXO_INLINE simd<float,  sizeof(float)  * elements> lookup(const float*  data) const noexcept { return PRE##_gather_ps(data, value, N * sizeof(float)); }  \
        template<std::size_t N = 1> KAIXO_INLINE simd<int,    sizeof(int)    * elements> lookup(const int*    data) const noexcept { return PRE##_gather_epi32(data, value, N * sizeof(int)); } \

    // ------------------------------------------------

#define SIMD_INSTANCE(BASE, TYPE, BITS, PRE, POST, PART)\
    template<>                                                                                                                     \
    struct simd<BASE, BITS> {                                                                                                      \
        constexpr static std::size_t bits = BITS;                                                                                  \
        constexpr static size_t elements = BITS / sizeof(BASE);                                                                    \
        constexpr static BASE all_mask = std::bit_cast<BASE>(0xffffffff);                                                          \
        using base = BASE;                                                                                                         \
        using type = TYPE;                                                                                                         \
                                                                                                                                   \
        static simd zero() noexcept { return PRE##_setzero_##POST(); }                                                             \
        KAIXO_INLINE static simd aligned(const base* v) noexcept { return PRE##_load_##POST(v); }                                  \
                                                                                                                                   \
        type value;                                                                                                                \
                                                                                                                                   \
        KAIXO_INLINE explicit simd(bool v) : value(v ? PRE##_set1_##POST(all_mask) : PRE##_set1_##POST(0)) {}                      \
        KAIXO_INLINE simd() : value{} {}                                                                                           \
        KAIXO_INLINE simd(const base* v) : value(PRE##_loadu_##POST(v)) {}                                                         \
        KAIXO_INLINE simd(base v) : value(PRE##_set1_##POST(v)) {}                                                                 \
        KAIXO_INLINE simd(const type& v) : value(v) {}                                                                             \
                                                                                                                                   \
        template<std::same_as<base> ...Args>                                                                                       \
        KAIXO_INLINE simd(Args...args) : value(PRE##_setr_##POST(args...)) {}                                                      \
                                                                                                                                   \
        KAIXO_INLINE void get(base* data) const noexcept { return PRE##_storeu_##POST(data, value); }                              \
        KAIXO_INLINE void stream(base* data) const noexcept { return PRE##_stream_##POST(data, value); }                           \
        KAIXO_INLINE void store(base* data) const noexcept { return PRE##_store_##POST(data, value); }                             \
                                                                                                                                   \
        KAIXO_INLINE base sum() const noexcept { return PRE##_sum_##POST(value); }                                                 \
                                                                                                                                   \
        KAIXO_INLINE simd operator~() const noexcept { return PRE##_xor_##POST(value, PRE##_set1_##POST(all_mask)); }              \
                                                                                                                                   \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator +(const simd& a, const simd& b) noexcept { return a.value  + b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator -(const simd& a, const simd& b) noexcept { return a.value  - b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator *(const simd& a, const simd& b) noexcept { return a.value  * b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator /(const simd& a, const simd& b) noexcept { return a.value  / b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator &(const simd& a, const simd& b) noexcept { return a.value  & b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator |(const simd& a, const simd& b) noexcept { return a.value  | b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator ^(const simd& a, const simd& b) noexcept { return a.value  ^ b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator <(const simd& a, const simd& b) noexcept { return a.value  < b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator >(const simd& a, const simd& b) noexcept { return a.value  > b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<=(const simd& a, const simd& b) noexcept { return a.value <= b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>=(const simd& a, const simd& b) noexcept { return a.value >= b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator==(const simd& a, const simd& b) noexcept { return a.value == b.value; } \
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator!=(const simd& a, const simd& b) noexcept { return ~simd{ a.value == b.value }; } \
        PART(BASE, TYPE, BITS, PRE, POST)                                                                                          \
    };

    // ------------------------------------------------

    SIMD_INSTANCE(    int, __m128i, 128, _mm   , epi32, SIMD_INSTANCE_INT_PART);
    SIMD_INSTANCE(    int, __m256i, 256, _mm256, epi32, SIMD_INSTANCE_INT_PART);
    SIMD_INSTANCE(    int, __m512i, 512, _mm512, epi32, SIMD_INSTANCE_INT_PART);
    SIMD_INSTANCE(  float, __m128 , 128, _mm   , ps   , SIMD_INSTANCE_FLOAT_PART);
    SIMD_INSTANCE(  float, __m256 , 256, _mm256, ps   , SIMD_INSTANCE_FLOAT_PART);
    SIMD_INSTANCE(  float, __m512 , 512, _mm512, ps   , SIMD_INSTANCE_FLOAT_PART);

    // ------------------------------------------------

    template<class Ty> concept is_mono = std::is_arithmetic_v<Ty>;
    template<class Ty> concept is_poly = is_simd<Ty>;

    // ------------------------------------------------

    template<class Ty> struct base;
    template<is_mono Ty> struct base<Ty> : std::type_identity<Ty> {};
    template<is_poly Ty> struct base<Ty> : std::type_identity<typename Ty::base> {};
    template<class Ty> using base_t = typename base<Ty>::type;

    template<class Type>
    KAIXO_INLINE decltype(auto) at(const base_t<Type>* ptr, std::size_t index) noexcept {
        if constexpr (is_mono<Type>) return ptr[index];
        else return Type(ptr + index);
    }

    template<class Type>
    KAIXO_INLINE decltype(auto) aligned_at(const base_t<Type>* ptr, std::size_t index) noexcept {
        if constexpr (is_mono<Type>) return ptr[index];
        else return Type::aligned(ptr + index);
    }

    template<class Type>
    KAIXO_INLINE decltype(auto) assign(base_t<Type>* ptr, Type value) noexcept {
        if constexpr (is_mono<Type>) return *ptr = value;
        else return value.get(ptr);
    }

    template<class Type>
    KAIXO_INLINE decltype(auto) store(base_t<Type>* ptr, Type value) noexcept {
        if constexpr (is_mono<Type>) return *ptr = value;
        else return value.store(ptr);
    }

    template<class Type, std::convertible_to<Type> B>
    KAIXO_INLINE decltype(auto) conditional(Type condition, B value) noexcept {
        if constexpr (is_mono<Type>) return condition * value;
        else return condition & value;
    }

    template<class Type, std::invocable A, std::invocable B>
    KAIXO_INLINE decltype(auto) iff(Type condition, A then, B otherwise) noexcept {
        if constexpr (is_mono<Type>) return condition ? then() : otherwise();
        else return condition & then() | ~condition & otherwise();
    }

    // Multiply with 1 or -1
    template<class Type, std::convertible_to<Type> B>
    KAIXO_INLINE decltype(auto) mul1(Type condition, B value) noexcept {
        if constexpr (is_mono<Type>) return condition * value;
        else return condition ^ ((-0.f) & value); // Toggle sign bit if value has sign bit
    };

    template<class To, class Type>
    KAIXO_INLINE decltype(auto) to(Type v) noexcept {
        if constexpr (is_mono<Type>) return (To)v;
        else return v.template to<To>();
    }

    template<class To, class Type>
    KAIXO_INLINE decltype(auto) reinterpret(Type v) noexcept {
        if constexpr (is_mono<Type>) return std::bit_cast<To>(v);
        else return v.template reinterpret<To>();
    }

    template<class Type, class Ptr>
    KAIXO_INLINE decltype(auto) lookup(Ptr* data, Type index) noexcept {
        if constexpr (is_mono<Type>) return data[(std::int64_t)index];
        else return index.lookup(data);
    }

    template<class Type>
    KAIXO_INLINE decltype(auto) sum(Type value) noexcept {
        if constexpr (is_mono<Type>) return value;
        else return value.sum();
    }
}
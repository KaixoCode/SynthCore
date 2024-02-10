#pragma once
#include <algorithm>
#include <bit>
#include <concepts>
#include <type_traits>
#include <numeric>
#include <concepts>

#include <immintrin.h>
#include <stdint.h>
#include <cpuid/cpuinfo.hpp>

#include "Kaixo/Utils/utils.hpp"

// ------------------------------------------------

#define KAIXO_VECTORCALL __vectorcall
#define KAIXO_INLINE __forceinline

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------
    
    enum simd_capability {
        SSE      = 0b00000000'00000000'00000000'00000001ull,
        SSE2     = 0b00000000'00000000'00000000'00000010ull,
        SSE3     = 0b00000000'00000000'00000000'00000100ull,
        SSSE3    = 0b00000000'00000000'00000000'00001000ull,
        SSE4_1   = 0b00000000'00000000'00000000'00010000ull,
        SSE4_2   = 0b00000000'00000000'00000000'00100000ull,
        AVX      = 0b00000000'00000000'00000001'00000000ull,
        AVX2     = 0b00000000'00000000'00000010'00000000ull,
        AVX512F  = 0b00000000'00000001'00000000'00000000ull,
        AVX512DQ = 0b00000000'00000010'00000000'00000000ull,
        AVX512VL = 0b00000000'00000100'00000000'00000000ull,
    };

    // ------------------------------------------------
    
    KAIXO_INLINE simd_capability simdCapabilities() {
        simd_capability _result{};
        cpuid::cpuinfo _info{};
        if (_info.has_sse())       _result |= SSE;
        if (_info.has_sse2())      _result |= SSE2;
        if (_info.has_sse3())      _result |= SSE3;
        if (_info.has_ssse3())     _result |= SSSE3;
        if (_info.has_sse4_1())    _result |= SSE4_1;
        if (_info.has_sse4_2())    _result |= SSE4_2;
        if (_info.has_avx())       _result |= AVX;
        if (_info.has_avx2())      _result |= AVX2;
        if (_info.has_avx512_f())  _result |= AVX512F;
        if (_info.has_avx512_dq()) _result |= AVX512DQ;
        if (_info.has_avx512_vl()) _result |= AVX512VL;
        return _result;
    }

    inline const simd_capability SIMD_CAPABILITIES = simdCapabilities();

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
    
#define KAIXO_FROM_MASK512(NAME, TYPE, MASK, FUN, UNDERLYING)\
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) noexcept{                            \
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
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) noexcept {                           \
        constexpr UNDERLYING all_mask = std::bit_cast<UNDERLYING>(0xffffffff);              \
        return FUN(mask & (1ull <<  0) ? all_mask : 0, mask & (1ull <<  1) ? all_mask : 0,  \
                   mask & (1ull <<  2) ? all_mask : 0, mask & (1ull <<  3) ? all_mask : 0,  \
                   mask & (1ull <<  4) ? all_mask : 0, mask & (1ull <<  5) ? all_mask : 0,  \
                   mask & (1ull <<  6) ? all_mask : 0, mask & (1ull <<  7) ? all_mask : 0); \
    }
#define KAIXO_FROM_MASK128(NAME, TYPE, MASK, FUN, UNDERLYING)\
    KAIXO_INLINE TYPE KAIXO_VECTORCALL NAME(MASK mask) noexcept {                           \
        constexpr UNDERLYING all_mask = std::bit_cast<UNDERLYING>(0xffffffff);              \
        return FUN(mask & (1ull <<  0) ? all_mask : 0, mask & (1ull <<  1) ? all_mask : 0,  \
                   mask & (1ull <<  2) ? all_mask : 0, mask & (1ull <<  3) ? all_mask : 0); \
    }

    KAIXO_FROM_MASK128(_mm_from_mask_ps, __m128, __mmask8, _mm_setr_ps, float);
    KAIXO_FROM_MASK128(_mm_from_mask_epi32, __m128i, __mmask8, _mm_setr_epi32, int);
    KAIXO_FROM_MASK256(_mm256_from_mask_ps, __m256, __mmask8, _mm256_setr_ps, float);
    KAIXO_FROM_MASK256(_mm256_from_mask_epi32, __m256i, __mmask8, _mm256_setr_epi32, int);
    KAIXO_FROM_MASK512(_mm512_from_mask_ps, __m512, __mmask16, _mm512_setr_ps, float);
    KAIXO_FROM_MASK512(_mm512_from_mask_epi32, __m512i, __mmask16, _mm512_setr_epi32, int)

    // ------------------------------------------------
    
#define SIMD_CALL(CAP, BITS, TYPE) if constexpr ((Capabilities & (CAP)) && std::same_as<TYPE, Ty> && BITS == Bits) return

    template<class Ty, std::size_t Bits, simd_capability Capabilities>
    struct simd {
        constexpr static std::size_t bits = Bits;
        constexpr static std::size_t bytes = Bits / 8;
        constexpr static std::size_t elements = bytes / sizeof(Ty);
        constexpr static simd_capability capabilities = Capabilities;

        using base = Ty;
        using simd_type = underlying_simd_t<Ty, Bits>;

        simd_type value{};

        KAIXO_INLINE static simd KAIXO_VECTORCALL setzero() noexcept {
            SIMD_CALL(SSE, 128, float) _mm_setzero_ps(); 
            SIMD_CALL(AVX, 256, float) _mm256_setzero_ps(); 
            SIMD_CALL(AVX512F, 512, float) _mm512_setzero_ps(); 
            SIMD_CALL(SSE2, 128, int) _mm_setzero_si128();
            SIMD_CALL(AVX, 256, int) _mm256_setzero_si256();
            SIMD_CALL(AVX512F, 512, int) _mm512_setzero_si512();
        }
            
        KAIXO_INLINE static simd KAIXO_VECTORCALL setone() noexcept {
            SIMD_CALL(SSE, 128, float) _mm_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(AVX, 256, float) _mm256_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(AVX512F, 512, float) _mm512_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(SSE2, 128, int) _mm_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
            SIMD_CALL(AVX, 256, int) _mm256_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
            SIMD_CALL(AVX512F, 512, int) _mm512_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
        }
            
        KAIXO_INLINE static simd KAIXO_VECTORCALL setr(base a1, base a2, base a3, base a4) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_setr_ps(a1, a2, a3, a4); 
            SIMD_CALL(SSE2, 128, int) _mm_setr_epi32(a1, a2, a3, a4);
        }

        KAIXO_INLINE static simd KAIXO_VECTORCALL setr(base a1, base a2, base a3, base a4, base a5, base a6, base a7, base a8) noexcept {
            SIMD_CALL(AVX, 256, float) _mm256_setr_ps(a1, a2, a3, a4, a5, a6, a7, a8); 
            SIMD_CALL(AVX, 256, int) _mm256_setr_epi32(a1, a2, a3, a4, a5, a6, a7, a8);
        }
            
        KAIXO_INLINE static simd KAIXO_VECTORCALL setr(
                base a1, base  a2, base  a3, base  a4, base  a5, base  a6, base  a7, base  a8,
                base a9, base a10, base a11, base a12, base a13, base a14, base a15, base a16) noexcept {
            SIMD_CALL(AVX512F, 512, float) _mm512_setr_ps(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16); 
            SIMD_CALL(AVX512F, 512, int) _mm512_setr_epi32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16); 
        }
            
        KAIXO_INLINE static simd KAIXO_VECTORCALL set1(base val) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_set1_ps(val);
            SIMD_CALL(AVX, 256, float) _mm256_set1_ps(val);
            SIMD_CALL(AVX512F, 512, float) _mm512_set1_ps(val);
            SIMD_CALL(SSE2, 128, int) _mm_set1_epi32(val);
            SIMD_CALL(AVX, 256, int) _mm256_set1_epi32(val);
            SIMD_CALL(AVX512F, 512, int) _mm512_set1_epi32(val);
        }

        // Must be aligned!
        KAIXO_INLINE static simd KAIXO_VECTORCALL load(base const* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_load_ps(addr);
            SIMD_CALL(AVX, 256, float) _mm256_load_ps(addr);
            SIMD_CALL(AVX512F, 512, float) _mm512_load_ps(addr);
            SIMD_CALL(SSE2, 128, int) _mm_load_si128(addr);
            SIMD_CALL(AVX, 256, int) _mm256_load_si256(addr);
            SIMD_CALL(AVX512F, 512, int) _mm512_load_si512(addr);
        }

        KAIXO_INLINE static simd KAIXO_VECTORCALL loadu(base const* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_loadu_ps(addr);
            SIMD_CALL(AVX, 256, float) _mm256_loadu_ps(addr);
            SIMD_CALL(AVX512F, 512, float) _mm512_loadu_ps(addr);
            SIMD_CALL(SSE2, 128, int) _mm_loadu_si128(addr);
            SIMD_CALL(AVX, 256, int) _mm256_loadu_si256(addr);
            SIMD_CALL(AVX512F, 512, int) _mm512_loadu_si512(addr);
        }

        KAIXO_INLINE void KAIXO_VECTORCALL store(base* addr) const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_store_ps(addr, value);
            SIMD_CALL(AVX, 256, float) _mm256_store_ps(addr, value);
            SIMD_CALL(AVX512F, 512, float) _mm512_store_ps(addr, value);
            SIMD_CALL(SSE2, 128, int) _mm_store_si128(addr, value);
            SIMD_CALL(AVX, 256, int) _mm256_store_si256(addr, value);
            SIMD_CALL(AVX512F, 512, int) _mm512_store_si512(addr, value);
        }

        KAIXO_INLINE void KAIXO_VECTORCALL storeu(base* addr) const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_storeu_ps(addr, value);
            SIMD_CALL(AVX, 256, float) _mm256_storeu_ps(addr, value);
            SIMD_CALL(AVX512F, 512, float) _mm512_storeu_ps(addr, value);
            SIMD_CALL(SSE2, 128, int) _mm_storeu_si128(addr, value);
            SIMD_CALL(AVX, 256, int) _mm256_storeu_si256(addr, value);
            SIMD_CALL(AVX512F, 512, int) _mm512_storeu_si512(addr, value);
        }

        KAIXO_INLINE simd() : value() {}
        KAIXO_INLINE simd(base const* addr) : value(loadu(addr).value) {}
        KAIXO_INLINE simd(base val) : value(set1(val).value) {}
        KAIXO_INLINE simd(simd_type val) : value(val) {}
        KAIXO_INLINE explicit simd(bool val) : value(val ? setone().value : setzero().value) {}

        template<class ...Args> requires (sizeof...(Args) == elements && (std::same_as<base, Args> && ...))
        KAIXO_INLINE simd(Args ... args) : value(setr(args...)) {}

        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator+(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_add_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_add_ps(a.value, b.value);
            SIMD_CALL(AVX512F, 512, float) _mm512_add_ps(a.value, b.value);
            SIMD_CALL(SSE2, 128, int) _mm_add_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_add_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_add_epi32(a.value, b.value);
        }
        
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator-(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sub_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_sub_ps(a.value, b.value);
            SIMD_CALL(AVX512F, 512, float) _mm512_sub_ps(a.value, b.value);
            SIMD_CALL(SSE2, 128, int) _mm_sub_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_sub_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_sub_epi32(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator*(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_mul_ps(a. value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_mul_ps(a. value, b.value);
            SIMD_CALL(AVX512F, 512, float) _mm512_mul_ps(a. value, b.value);
            SIMD_CALL(SSE4_1, 128, int) _mm_mullo_epi32(a. value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_mullo_epi32(a. value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_mullo_epi32(a. value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator/(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_div_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_div_ps(a.value, b.value);
            SIMD_CALL(AVX512F, 512, float) _mm512_div_ps(a.value, b.value);
            SIMD_CALL(SSE, 128, int) _mm_div_epi32(a.value, b.value);
            SIMD_CALL(AVX, 256, int) _mm256_div_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_div_epi32(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator&(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_and_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_and_ps(a.value, b.value);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_and_ps(a.value, b.value);
            SIMD_CALL(SSE2, 128, int) _mm_and_si128(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_and_si256(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_and_si512(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator|(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_or_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_or_ps(a.value, b.value);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_or_ps(a.value, b.value);
            SIMD_CALL(SSE2, 128, int) _mm_or_si128(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_or_si256(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_or_si512(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator^(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_xor_ps(a.value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_xor_ps(a.value, b.value);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_xor_ps(a.value, b.value);
            SIMD_CALL(SSE2, 128, int) _mm_xor_si128(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_xor_si256(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_xor_si512(a.value, b.value);
        }
            
        KAIXO_INLINE simd KAIXO_VECTORCALL operator~() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_xor_ps(value, _mm_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(AVX, 256, float) _mm256_xor_ps(value, _mm256_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(AVX512DQ, 512, float) _mm512_xor_ps(value, _mm512_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(SSE2, 128, int) _mm_xor_si128(value, _mm_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
            SIMD_CALL(AVX2, 256, int) _mm256_xor_si256(value, _mm256_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
            SIMD_CALL(AVX512F, 512, int) _mm512_xor_si512(value, _mm512_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
        }

        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator==(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpeq_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_EQ_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_EQ_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmpeq_ps_mask(a.value, b.value));
            SIMD_CALL(SSE2, 128, int) _mm_cmpeq_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_cmpeq_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_cmpeq_epi32(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator!=(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpneq_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_NEQ_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_NEQ_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmpneq_ps_mask(a.value, b.value));
            SIMD_CALL(Capabilities, 128, int) ~(a == b);
            SIMD_CALL(Capabilities, 256, int) ~(a == b);
            SIMD_CALL(Capabilities, 512, int) ~(a == b);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpgt_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_GT_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_GT_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a.value, b.value, _CMP_GT_OS));
            SIMD_CALL(SSE2, 128, int) _mm_cmpgt_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_cmpgt_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_cmpgt_epi32(a.value, b.value);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmplt_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_LT_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_LT_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a.value, b.value, _CMP_LT_OS));
            SIMD_CALL(SSE2, 128, int) _mm_cmplt_epi32(a.value, b.value);
            SIMD_CALL(Capabilities, 256, int) (b > a) | (b > a);
            SIMD_CALL(Capabilities, 512, int) (b > a) | (b > a);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>=(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpge_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_GE_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_GE_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a.value, b.value, _CMP_GE_OS));
            SIMD_CALL(Capabilities, 128, int) (a > b) | (a == b);
            SIMD_CALL(Capabilities, 256, int) (a > b) | (a == b);
            SIMD_CALL(Capabilities, 512, int) (a > b) | (a == b);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<=(const simd& a, const simd& b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmple_ps(a.value, b.value);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a.value, b.value, _CMP_LE_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a.value, b.value, _CMP_LE_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a.value, b.value, _CMP_LE_OS));
            SIMD_CALL(Capabilities, 128, int) (a < b) | (a == b);
            SIMD_CALL(Capabilities, 256, int) (a < b) | (a == b);
            SIMD_CALL(Capabilities, 512, int) (a < b) | (a == b);
        }
            
        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<<(const simd& a, const simd& b) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_sllv_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_sllv_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_sllv_epi32(a.value, b.value);
        }

        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator<<(const simd& a, std::integral auto b) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_slli_epi32(a.value, static_cast<int>(b));
            SIMD_CALL(AVX2, 256, int) _mm256_slli_epi32(a.value, static_cast<int>(b));
            SIMD_CALL(AVX512F, 512, int) _mm512_slli_epi32(a.value, static_cast<unsigned int>(b));
        }

        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>>(const simd& a, const simd& b) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_srlv_epi32(a.value, b.value);
            SIMD_CALL(AVX2, 256, int) _mm256_srlv_epi32(a.value, b.value);
            SIMD_CALL(AVX512F, 512, int) _mm512_srlv_epi32(a.value, b.value);
        }

        friend KAIXO_INLINE simd KAIXO_VECTORCALL operator>>(const simd& a, std::integral auto b) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_srli_epi32(a.value, static_cast<int>(b));
            SIMD_CALL(AVX2, 256, int) _mm256_srli_epi32(a.value, static_cast<int>(b));
            SIMD_CALL(AVX512F, 512, int) _mm512_srli_epi32(a.value, static_cast<unsigned int>(b));
        }

        // SSE3 | SSE
        KAIXO_INLINE static base KAIXO_VECTORCALL _sum_general(simd_type a) noexcept {
            base vals[elements];
            simd{ a }.store(vals);
            return std::accumulate(vals, vals + elements, base{});
        }
            
        // SSE3 | SSE
        KAIXO_INLINE static base KAIXO_VECTORCALL _sum_128_f(__m128 a) noexcept {
            __m128 shuf = _mm_movehdup_ps(a);
            __m128 sums = _mm_add_ps(a, shuf);
            shuf = _mm_movehl_ps(shuf, sums);
            sums = _mm_add_ss(sums, shuf);
            return _mm_cvtss_f32(sums);
        }

        // AVX | SSE
        KAIXO_INLINE static base KAIXO_VECTORCALL _sum_256_f(__m256 a) noexcept {
            __m128 hiQuad = _mm256_extractf128_ps(a, 1);
            __m128 loQuad = _mm256_castps256_ps128(a);
            __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
            __m128 loDual = sumQuad;
            __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
            __m128 sumDual = _mm_add_ps(loDual, hiDual);
            __m128 lo = sumDual;
            __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
            __m128 sum = _mm_add_ss(lo, hi);
            return _mm_cvtss_f32(sum);
        }

        // AVX512F | AVX512DQ | AVX | SSE
        KAIXO_INLINE static base KAIXO_VECTORCALL _sum_512_f(__m512 a) noexcept {
            __m256 v0 = _mm512_castps512_ps256(a);
            __m256 v1 = _mm512_extractf32x8_ps(a, 1);
            __m256 x0 = _mm256_add_ps(v0, v1);
            return _sum_256_f(x0);
        }

        KAIXO_INLINE base KAIXO_VECTORCALL sum() const noexcept {
            SIMD_CALL(SSE3 | SSE, 128, float) _sum_128_f(value);
            SIMD_CALL(AVX | SSE, 256, float) _sum_256_f(value);
            SIMD_CALL(AVX512F | AVX512DQ | AVX | SSE, 512, float) _sum_512_f(value);
            SIMD_CALL(Capabilities, 128, float) _sum_general(value);
            SIMD_CALL(Capabilities, 256, float) _sum_general(value);
            SIMD_CALL(Capabilities, 512, float) _sum_general(value);
            SIMD_CALL(Capabilities, 128, int) _sum_general(value);
            SIMD_CALL(Capabilities, 256, int) _sum_general(value);
            SIMD_CALL(Capabilities, 512, int) _sum_general(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL trunc() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_trunc_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_trunc_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_trunc_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL floor() const noexcept {
            SIMD_CALL(SSE4_1, 128, float) _mm_floor_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_floor_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_floor_ps(value);
        }
        
        KAIXO_INLINE simd KAIXO_VECTORCALL fmod1() const noexcept {
            SIMD_CALL(SSE | SSE4_1, 128, float) _mm_sub_ps(value, _mm_floor_ps(value));
            SIMD_CALL(SSE, 128, float) _mm_sub_ps(value, _mm_trunc_ps(value));
            SIMD_CALL(AVX, 256, float) _mm256_sub_ps(value, _mm256_floor_ps(value));
            SIMD_CALL(AVX512F, 512, float) _mm512_sub_ps(value, _mm512_floor_ps(value));
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL ceil() const noexcept {
            SIMD_CALL(SSE4_1, 128, float) _mm_ceil_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_ceil_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_ceil_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL round() const noexcept {
            SIMD_CALL(AVX512F | AVX512VL, 128, float) _mm_roundscale_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
            SIMD_CALL(AVX512F | AVX512VL, 256, float) _mm256_roundscale_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
            SIMD_CALL(AVX512F | AVX512VL, 512, float) _mm512_roundscale_ps(value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL log() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_log_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_log_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL log2() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log2_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_log2_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_log2_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL log10() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log10_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_log10_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_log10_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL sqrt() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sqrt_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_sqrt_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_sqrt_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL cbrt() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cbrt_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_cbrt_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_cbrt_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL exp() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_exp_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL exp2() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp2_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_exp2_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp2_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL exp10() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp10_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_exp10_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp10_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL tanh() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_tanh_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_tanh_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_tanh_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL abs() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_andnot_ps(_mm_set1_ps(-0.0), value);
            SIMD_CALL(AVX, 256, float) _mm256_andnot_ps(_mm256_set1_ps(-0.0), value);
            SIMD_CALL(AVX512F | AVX512DQ, 512, float) _mm512_andnot_ps(_mm512_set1_ps(-0.0), value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL cos() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cos_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_cos_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_cos_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL cosh() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cosh_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_cosh_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_cosh_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL sin() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sin_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_sin_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_sin_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL sinh() const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sinh_ps(value);
            SIMD_CALL(AVX, 256, float) _mm256_sinh_ps(value);
            SIMD_CALL(AVX512F, 512, float) _mm512_sinh_ps(value);
        }

        KAIXO_INLINE simd KAIXO_VECTORCALL pow(const simd& b) const noexcept {
            SIMD_CALL(SSE, 128, float) _mm_pow_ps(value, b.value);
            SIMD_CALL(AVX, 256, float) _mm256_pow_ps(value, b.value);
            SIMD_CALL(AVX512F, 512, float) _mm512_pow_ps(value, b.value);
        }

        template<class To>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL cast() const noexcept {
            if constexpr (std::same_as<Ty, To>) return value;
            else if constexpr (std::same_as<To, int>) {
                SIMD_CALL(SSE2, 128, float) _mm_cvtps_epi32(value);
                SIMD_CALL(AVX, 256, float) _mm256_cvtps_epi32(value);
                SIMD_CALL(AVX512F, 512, float) _mm512_cvtps_epi32(value);
            } else if constexpr (std::same_as<To, float>) {
                SIMD_CALL(SSE2, 128, int) _mm_cvtepi32_ps(value);
                SIMD_CALL(AVX, 256, int) _mm256_cvtepi32_ps(value);
                SIMD_CALL(AVX512F, 512, int) _mm512_cvtepi32_ps(value);
            }
        }

        template<class To>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL reinterpret() const noexcept {
            if constexpr (std::same_as<Ty, To>) return value;
            else if constexpr (std::same_as<To, int>) {
                SIMD_CALL(SSE2, 128, float) _mm_castps_si128(value);
                SIMD_CALL(AVX, 256, float) _mm256_castps_si256(value);
                SIMD_CALL(AVX512F, 512, float) _mm512_castps_si512(value);
            } else if constexpr (std::same_as<To, float>) {
                SIMD_CALL(SSE2, 128, int) _mm_castsi128_ps(value);
                SIMD_CALL(AVX, 256, int) _mm256_castsi256_ps(value);
                SIMD_CALL(AVX512F, 512, int) _mm512_castsi512_ps(value);
            }
        }

        KAIXO_INLINE simd<float, Bits, Capabilities> KAIXO_VECTORCALL gather(float const* data) const noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_i32gather_ps(data, value, sizeof(float));
            SIMD_CALL(AVX2, 256, int) _mm256_i32gather_ps(data, value, sizeof(float));
            SIMD_CALL(AVX512F, 512, int) _mm512_i32gather_ps(data, value, sizeof(float));
        }

        KAIXO_INLINE simd<int, Bits, Capabilities> KAIXO_VECTORCALL gather(int const* data) const noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_i32gather_epi32(data, value, sizeof(int));
            SIMD_CALL(AVX2, 256, int) _mm256_i32gather_epi32(data, value, sizeof(int));
            SIMD_CALL(AVX512F, 512, int) _mm512_i32gather_epi32(data, value, sizeof(int));
        }
    };

    template<class Simd>
    concept is_simd = requires() {
        typename Simd::base;
        typename Simd::simd_type;
        std::same_as<decltype(Simd::elements), std::size_t>;
        std::same_as<decltype(Simd::bits), std::size_t>;
        std::same_as<decltype(Simd::bytes), std::size_t>;
        std::same_as<decltype(Simd::capabilities), simd_capability>;
    };

    // ------------------------------------------------

    template<class Ty> concept is_mono = std::is_arithmetic_v<Ty>;
    template<class Ty> concept is_poly = is_simd<Ty>;

    // ------------------------------------------------

    template<class Ty> struct base;
    template<is_mono Ty> struct base<Ty> : std::type_identity<Ty> {};
    template<is_poly Ty> struct base<Ty> : std::type_identity<typename Ty::base> {};
    template<class Ty> using base_t = typename base<Ty>::type;

    template<class Type>
    KAIXO_INLINE Type KAIXO_VECTORCALL at(base_t<Type> const* ptr, std::size_t index) noexcept {
        if constexpr (is_mono<Type>) return ptr[index];
        else return Type(ptr + index);
    }

    template<class Type>
    KAIXO_INLINE Type KAIXO_VECTORCALL aligned_at(base_t<Type> const* ptr, std::size_t index) noexcept {
        if constexpr (is_mono<Type>) return ptr[index];
        else return Type::load(ptr + index);
    }

    template<class Type>
    KAIXO_INLINE void KAIXO_VECTORCALL assign(base_t<Type>* ptr, const Type& value) noexcept {
        if constexpr (is_mono<Type>) *ptr = value;
        else value.storeu(ptr);
    }

    template<class Type>
    KAIXO_INLINE void KAIXO_VECTORCALL store(base_t<Type>* ptr, const Type& value) noexcept {
        if constexpr (is_mono<Type>) *ptr = value;
        else value.store(ptr);
    }

    template<class Type, std::convertible_to<Type> B>
    KAIXO_INLINE Type KAIXO_VECTORCALL conditional(const Type& condition, B value) noexcept {
        if constexpr (is_mono<Type>) return condition * value;
        else return condition & value;
    }

    template<class Type, std::invocable A, std::invocable B>
    KAIXO_INLINE Type KAIXO_VECTORCALL iff(const Type& condition, A then, B otherwise) noexcept {
        if constexpr (is_mono<Type>) return condition ? then() : otherwise();
        else return condition & then() | ~condition & otherwise();
    }

    // Multiply with 1 or -1
    template<class Type, std::convertible_to<Type> B>
    KAIXO_INLINE Type KAIXO_VECTORCALL mul1(const Type& condition, B value) noexcept {
        if constexpr (is_mono<Type>) return condition * value;
        else return condition ^ ((-0.f) & value); // Toggle sign bit if value has sign bit
    };

    template<class To, class Type>
    KAIXO_INLINE To KAIXO_VECTORCALL to(const Type& v) noexcept {
        if constexpr (is_mono<Type>) return (To)v;
        else return v.template cast<To>();
    }

    template<class To, class Type>
    KAIXO_INLINE To KAIXO_VECTORCALL reinterpret(const Type& v) noexcept {
        if constexpr (is_mono<Type>) return std::bit_cast<To>(v);
        else return v.template reinterpret<To>();
    }

    template<class Type, class Ptr>
    KAIXO_INLINE decltype(auto) KAIXO_VECTORCALL lookup(Ptr* data, const Type& index) noexcept {
        if constexpr (is_mono<Type>) return data[(std::int64_t)index];
        else return index.gather(data);
    }

    template<class Type>
    KAIXO_INLINE base_t<Type> KAIXO_VECTORCALL sum(const Type& value) noexcept {
        if constexpr (is_mono<Type>) return value;
        else return value.sum();
    }
}
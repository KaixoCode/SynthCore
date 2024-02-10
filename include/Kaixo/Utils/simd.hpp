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
    
#define SIMD_OPERATION KAIXO_INLINE static auto KAIXO_VECTORCALL
#define SIMD_CALL(CAP, BITS, TYPE) if constexpr ((Capabilities & (CAP)) && std::same_as<TYPE, Ty> && BITS == Bits) return

    template<class Ty, std::size_t Bits, simd_capability Capabilities>
    struct simd_operation {
        constexpr static std::size_t bits = Bits;
        constexpr static std::size_t bytes = Bits / 8;
        constexpr static std::size_t elements = bytes / sizeof(Ty);
        constexpr static simd_capability capabilities = Capabilities;

        using base = Ty;
        using simd_type = underlying_simd_t<Ty, Bits>;
        using operation = simd_operation<Ty, Bits, Capabilities>;

        SIMD_OPERATION setzero() noexcept {
            SIMD_CALL(SSE, 128, float) _mm_setzero_ps(); 
            SIMD_CALL(AVX, 256, float) _mm256_setzero_ps(); 
            SIMD_CALL(AVX512F, 512, float) _mm512_setzero_ps(); 
            SIMD_CALL(SSE2, 128, int) _mm_setzero_si128();
            SIMD_CALL(AVX, 256, int) _mm256_setzero_si256();
            SIMD_CALL(AVX512F, 512, int) _mm512_setzero_si512();
        }
            
        SIMD_OPERATION setone() noexcept {
            SIMD_CALL(SSE, 128, float) _mm_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(AVX, 256, float) _mm256_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(AVX512F, 512, float) _mm512_set1_ps(std::bit_cast<float>(0xFFFFFFFF));
            SIMD_CALL(SSE2, 128, int) _mm_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
            SIMD_CALL(AVX, 256, int) _mm256_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
            SIMD_CALL(AVX512F, 512, int) _mm512_set1_epi32(std::bit_cast<int>(0xFFFFFFFF));
        }
            
        SIMD_OPERATION setr(float a1, float a2, float a3, float a4) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_setr_ps(a1, a2, a3, a4); 
        }

        SIMD_OPERATION setr(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8) noexcept {
            SIMD_CALL(AVX, 256, float) _mm256_setr_ps(a1, a2, a3, a4, a5, a6, a7, a8); 
        }
            
        SIMD_OPERATION setr(float a1, float  a2, float  a3, float  a4, float  a5, float  a6, float  a7, float  a8, 
                            float a9, float a10, float a11, float a12, float a13, float a14, float a15, float a16) noexcept {
            SIMD_CALL(AVX512F, 512, float) _mm512_setr_ps(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16); 
        }
            
        SIMD_OPERATION setr(int a1, int a2, int a3, int a4) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_setr_epi32(a1, a2, a3, a4);
        }

        SIMD_OPERATION setr(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) noexcept {
            SIMD_CALL(AVX, 256, int) _mm256_setr_epi32(a1, a2, a3, a4, a5, a6, a7, a8);
        }
            
        SIMD_OPERATION setr(int a1, int  a2, int  a3, int  a4, int  a5, int  a6, int  a7, int  a8,
                            int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16) noexcept {
            SIMD_CALL(AVX512F, 512, int) _mm512_setr_epi32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16); 
        }

        SIMD_OPERATION set1(float val) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_set1_ps(val);
            SIMD_CALL(AVX, 256, float) _mm256_set1_ps(val);
            SIMD_CALL(AVX512F, 512, float) _mm512_set1_ps(val);
        }

        SIMD_OPERATION set1(int val) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_set1_epi32(val);
            SIMD_CALL(AVX, 256, int) _mm256_set1_epi32(val);
            SIMD_CALL(AVX512F, 512, int) _mm512_set1_epi32(val);
        }
            
        // Must be aligned!
        SIMD_OPERATION load(float const* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_load_ps(addr);
            SIMD_CALL(AVX, 256, float) _mm256_load_ps(addr);
            SIMD_CALL(AVX512F, 512, float) _mm512_load_ps(addr);
        }

        // Must be aligned!
        SIMD_OPERATION load(int const* addr) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_load_si128(addr);
            SIMD_CALL(AVX, 256, int) _mm256_load_si256(addr);
            SIMD_CALL(AVX512F, 512, int) _mm512_load_si512(addr);
        }
            
        SIMD_OPERATION loadu(float const* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_loadu_ps(addr);
            SIMD_CALL(AVX, 256, float) _mm256_loadu_ps(addr);
            SIMD_CALL(AVX512F, 512, float) _mm512_loadu_ps(addr);
        }

        SIMD_OPERATION loadu(int const* addr) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_loadu_si128(addr);
            SIMD_CALL(AVX, 256, int) _mm256_loadu_si256(addr);
            SIMD_CALL(AVX512F, 512, int) _mm512_loadu_si512(addr);
        }

        SIMD_OPERATION store(simd_type a, float* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_store_ps(addr, a);
            SIMD_CALL(AVX, 256, float) _mm256_store_ps(addr, a);
            SIMD_CALL(AVX512F, 512, float) _mm512_store_ps(addr, a);
        }

        SIMD_OPERATION store(simd_type a, int* addr) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_store_si128(addr, a);
            SIMD_CALL(AVX, 256, int) _mm256_store_si256(addr, a);
            SIMD_CALL(AVX512F, 512, int) _mm512_store_si512(addr, a);
        }
            
        SIMD_OPERATION storeu(simd_type a, float* addr) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_storeu_ps(addr, a);
            SIMD_CALL(AVX, 256, float) _mm256_storeu_ps(addr, a);
            SIMD_CALL(AVX512F, 512, float) _mm512_storeu_ps(addr, a);
        }

        SIMD_OPERATION storeu(simd_type a, int* addr) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_storeu_si128(addr, a);
            SIMD_CALL(AVX, 256, int) _mm256_storeu_si256(addr, a);
            SIMD_CALL(AVX512F, 512, int) _mm512_storeu_si512(addr, a);
        }
            
        SIMD_OPERATION add(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_add_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_add_ps(a, b);
            SIMD_CALL(AVX512F, 512, float) _mm512_add_ps(a, b);
            SIMD_CALL(SSE2, 128, int) _mm_add_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_add_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_add_epi32(a, b);
        }
            
        SIMD_OPERATION sub(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sub_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_sub_ps(a, b);
            SIMD_CALL(AVX512F, 512, float) _mm512_sub_ps(a, b);
            SIMD_CALL(SSE2, 128, int) _mm_sub_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_sub_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_sub_epi32(a, b);
        }
            
        SIMD_OPERATION mul(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_mul_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_mul_ps(a, b);
            SIMD_CALL(AVX512F, 512, float) _mm512_mul_ps(a, b);
            SIMD_CALL(SSE4_1, 128, int) _mm_mullo_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_mullo_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_mullo_epi32(a, b);
        }
            
        SIMD_OPERATION div(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_div_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_div_ps(a, b);
            SIMD_CALL(AVX512F, 512, float) _mm512_div_ps(a, b);
            SIMD_CALL(SSE, 128, int) _mm_div_epi32(a, b);
            SIMD_CALL(AVX, 256, int) _mm256_div_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_div_epi32(a, b);
        }
            
        SIMD_OPERATION bit_and(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_and_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_and_ps(a, b);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_and_ps(a, b);
            SIMD_CALL(SSE2, 128, int) _mm_and_si128(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_and_si256(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_and_si512(a, b);
        }
            
        SIMD_OPERATION bit_or(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_or_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_or_ps(a, b);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_or_ps(a, b);
            SIMD_CALL(SSE2, 128, int) _mm_or_si128(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_or_si256(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_or_si512(a, b);
        }
            
        SIMD_OPERATION bit_xor(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_xor_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_xor_ps(a, b);
            SIMD_CALL(AVX512DQ, 512, float) _mm512_xor_ps(a, b);
            SIMD_CALL(SSE2, 128, int) _mm_xor_si128(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_xor_si256(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_xor_si512(a, b);
        }
            
        SIMD_OPERATION bit_not(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_xor_ps(a, _mm_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(AVX, 256, float) _mm256_xor_ps(a, _mm256_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(AVX512DQ, 512, float) _mm512_xor_ps(a, _mm512_set1_ps(std::bit_cast<float>(0xFFFFFFFF)));
            SIMD_CALL(SSE2, 128, int) _mm_xor_si128(a, _mm_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
            SIMD_CALL(AVX2, 256, int) _mm256_xor_si256(a, _mm256_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
            SIMD_CALL(AVX512F, 512, int) _mm512_xor_si512(a, _mm512_set1_epi32(std::bit_cast<int>(0xFFFFFFFF)));
        }

        SIMD_OPERATION cmpeq(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpeq_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_EQ_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_EQ_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmpeq_ps_mask(a, b));
            SIMD_CALL(SSE2, 128, int) _mm_cmpeq_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_cmpeq_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_cmpeq_epi32(a, b);
        }
            
        SIMD_OPERATION cmpneq(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpneq_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_NEQ_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_NEQ_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmpneq_ps_mask(a, b));
            SIMD_CALL(Capabilities, 128, int) bit_not(cmpeq(a, b));
            SIMD_CALL(Capabilities, 256, int) bit_not(cmpeq(a, b));
            SIMD_CALL(Capabilities, 512, int) bit_not(cmpeq(a, b));
        }
            
        SIMD_OPERATION cmpgt(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpgt_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_GT_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_GT_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_GT_OS));
            SIMD_CALL(SSE2, 128, int) _mm_cmpgt_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_cmpgt_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_cmpgt_epi32(a, b);
        }
            
        SIMD_OPERATION cmplt(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmplt_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_LT_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_LT_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_LT_OS));
            SIMD_CALL(SSE2, 128, int) _mm_cmplt_epi32(a, b);
            SIMD_CALL(Capabilities, 256, int) bit_or(cmpgt(b, a), cmpeq(b, a));
            SIMD_CALL(Capabilities, 512, int) bit_or(cmpgt(b, a), cmpeq(b, a));
        }
            
        SIMD_OPERATION cmpge(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmpge_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_GE_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_GE_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_GE_OS));
            SIMD_CALL(Capabilities, 128, int) bit_or(cmpgt(a, b), cmpeq(a, b));
            SIMD_CALL(Capabilities, 256, int) bit_or(cmpgt(a, b), cmpeq(a, b));
            SIMD_CALL(Capabilities, 512, int) bit_or(cmpgt(a, b), cmpeq(a, b));
        }
            
        SIMD_OPERATION cmple(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cmple_ps(a, b);
            SIMD_CALL(AVX, 128, float) _mm_cmp_ps(a, b, _CMP_LE_OS);
            SIMD_CALL(AVX, 256, float) _mm256_cmp_ps(a, b, _CMP_LE_OS);
            SIMD_CALL(AVX512F, 512, float) _mm512_from_mask_ps(_mm512_cmp_ps_mask(a, b, _CMP_LE_OS));
            SIMD_CALL(Capabilities, 128, int) bit_or(cmplt(a, b), cmpeq(a, b));
            SIMD_CALL(Capabilities, 256, int) bit_or(cmplt(a, b), cmpeq(a, b));
            SIMD_CALL(Capabilities, 512, int) bit_or(cmplt(a, b), cmpeq(a, b));
        }
            
        SIMD_OPERATION shift_left(simd_type a, simd_type b) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_sllv_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_sllv_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_sllv_epi32(a, b);
        }

        SIMD_OPERATION shift_left(simd_type a, std::integral auto b) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_slli_epi32(a, static_cast<int>(b));
            SIMD_CALL(AVX2, 256, int) _mm256_slli_epi32(a, static_cast<int>(b));
            SIMD_CALL(AVX512F, 512, int) _mm512_slli_epi32(a, static_cast<unsigned int>(b));
        }

        SIMD_OPERATION shift_right(simd_type a, simd_type b) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_srlv_epi32(a, b);
            SIMD_CALL(AVX2, 256, int) _mm256_srlv_epi32(a, b);
            SIMD_CALL(AVX512F, 512, int) _mm512_srlv_epi32(a, b);
        }

        SIMD_OPERATION shift_right(simd_type a, std::integral auto b) noexcept {
            SIMD_CALL(SSE2, 128, int) _mm_srli_epi32(a, static_cast<int>(b));
            SIMD_CALL(AVX2, 256, int) _mm256_srli_epi32(a, static_cast<int>(b));
            SIMD_CALL(AVX512F, 512, int) _mm512_srli_epi32(a, static_cast<unsigned int>(b));
        }

        // SSE3 | SSE
        SIMD_OPERATION _sum_general(simd_type a) noexcept {
            base vals[elements];
            store(a, vals);
            return std::accumulate(vals, vals + elements, base{});
        }
            
        // SSE3 | SSE
        SIMD_OPERATION _sum_128_f(__m128 a) noexcept {
            __m128 shuf = _mm_movehdup_ps(a);
            __m128 sums = _mm_add_ps(a, shuf);
            shuf = _mm_movehl_ps(shuf, sums);
            sums = _mm_add_ss(sums, shuf);
            return _mm_cvtss_f32(sums);
        }

        // AVX | SSE
        SIMD_OPERATION _sum_256_f(__m256 a) noexcept {
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
        SIMD_OPERATION _sum_512_f(__m512 a) noexcept {
            __m256 v0 = _mm512_castps512_ps256(a);
            __m256 v1 = _mm512_extractf32x8_ps(a, 1);
            __m256 x0 = _mm256_add_ps(v0, v1);
            return _sum_256_f(x0);
        }

        SIMD_OPERATION sum(simd_type a) noexcept {
            SIMD_CALL(SSE3 | SSE, 128, float) _sum_128_f(a);
            SIMD_CALL(AVX | SSE, 256, float) _sum_256_f(a);
            SIMD_CALL(AVX512F | AVX512DQ | AVX | SSE, 512, float) _sum_512_f(a);
            SIMD_CALL(Capabilities, 128, float) _sum_general(a);
            SIMD_CALL(Capabilities, 256, float) _sum_general(a);
            SIMD_CALL(Capabilities, 512, float) _sum_general(a);
            SIMD_CALL(Capabilities, 128, int) _sum_general(a);
            SIMD_CALL(Capabilities, 256, int) _sum_general(a);
            SIMD_CALL(Capabilities, 512, int) _sum_general(a);
        }

        SIMD_OPERATION trunc(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_trunc_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_trunc_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_trunc_ps(a);
        }

        SIMD_OPERATION floor(simd_type a) noexcept {
            SIMD_CALL(SSE4_1, 128, float) _mm_floor_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_floor_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_floor_ps(a);
        }

        SIMD_OPERATION ceil(simd_type a) noexcept {
            SIMD_CALL(SSE4_1, 128, float) _mm_ceil_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_ceil_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_ceil_ps(a);
        }

        SIMD_OPERATION round(simd_type a) noexcept {
            SIMD_CALL(AVX512F | AVX512VL, 128, float) _mm_roundscale_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
            SIMD_CALL(AVX512F | AVX512VL, 256, float) _mm256_roundscale_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
            SIMD_CALL(AVX512F | AVX512VL, 512, float) _mm512_roundscale_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
        }

        SIMD_OPERATION log(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_log_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_log_ps(a);
        }

        SIMD_OPERATION log2(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log2_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_log2_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_log2_ps(a);
        }

        SIMD_OPERATION log10(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_log10_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_log10_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_log10_ps(a);
        }

        SIMD_OPERATION sqrt(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sqrt_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_sqrt_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_sqrt_ps(a);
        }

        SIMD_OPERATION cbrt(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cbrt_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_cbrt_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_cbrt_ps(a);
        }

        SIMD_OPERATION exp(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_exp_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp_ps(a);
        }

        SIMD_OPERATION exp2(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp2_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_exp2_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp2_ps(a);
        }

        SIMD_OPERATION exp10(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_exp10_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_exp10_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_exp10_ps(a);
        }

        SIMD_OPERATION tanh(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_tanh_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_tanh_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_tanh_ps(a);
        }

        SIMD_OPERATION abs(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_andnot_ps(_mm_set1_ps(-0.0), a);
            SIMD_CALL(AVX, 256, float) _mm256_andnot_ps(_mm256_set1_ps(-0.0), a);
            SIMD_CALL(AVX512F | AVX512DQ, 512, float) _mm512_andnot_ps(_mm512_set1_ps(-0.0), a);
        }

        SIMD_OPERATION cos(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cos_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_cos_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_cos_ps(a);
        }

        SIMD_OPERATION cosh(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_cosh_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_cosh_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_cosh_ps(a);
        }

        SIMD_OPERATION sin(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sin_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_sin_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_sin_ps(a);
        }

        SIMD_OPERATION sinh(simd_type a) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_sinh_ps(a);
            SIMD_CALL(AVX, 256, float) _mm256_sinh_ps(a);
            SIMD_CALL(AVX512F, 512, float) _mm512_sinh_ps(a);
        }

        SIMD_OPERATION pow(simd_type a, simd_type b) noexcept {
            SIMD_CALL(SSE, 128, float) _mm_pow_ps(a, b);
            SIMD_CALL(AVX, 256, float) _mm256_pow_ps(a, b);
            SIMD_CALL(AVX512F, 512, float) _mm512_pow_ps(a, b);
        }

        template<class To>
        SIMD_OPERATION cast(simd_type a) noexcept {
            if constexpr (std::same_as<Ty, To>) return a;
            else if constexpr (std::same_as<To, int>) {
                SIMD_CALL(SSE2, 128, float) _mm_cvtps_epi32(a);
                SIMD_CALL(AVX, 256, float) _mm256_cvtps_epi32(a);
                SIMD_CALL(AVX512F, 512, float) _mm512_cvtps_epi32(a);
            } else if constexpr (std::same_as<To, float>) {
                SIMD_CALL(SSE2, 128, int) _mm_cvtepi32_ps(a);
                SIMD_CALL(AVX, 256, int) _mm256_cvtepi32_ps(a);
                SIMD_CALL(AVX512F, 512, int) _mm512_cvtepi32_ps(a);
            }
        }

        template<class To>
        SIMD_OPERATION reinterpret(simd_type a) noexcept {
            if constexpr (std::same_as<Ty, To>) return a;
            else if constexpr (std::same_as<To, int>) {
                SIMD_CALL(SSE2, 128, float) _mm_castps_si128(a);
                SIMD_CALL(AVX, 256, float) _mm256_castps_si256(a);
                SIMD_CALL(AVX512F, 512, float) _mm512_castps_si512(a);
            } else if constexpr (std::same_as<To, float>) {
                SIMD_CALL(SSE2, 128, int) _mm_castsi128_ps(a);
                SIMD_CALL(AVX, 256, int) _mm256_castsi256_ps(a);
                SIMD_CALL(AVX512F, 512, int) _mm512_castsi512_ps(a);
            }
        }

        SIMD_OPERATION gather(float const* data, simd_type a) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_i32gather_ps(data, a, sizeof(float));
            SIMD_CALL(AVX2, 256, int) _mm256_i32gather_ps(data, a, sizeof(float));
            SIMD_CALL(AVX512F, 512, int) _mm512_i32gather_ps(data, a, sizeof(float));
        }

        SIMD_OPERATION gather(int const* data, simd_type a) noexcept {
            SIMD_CALL(AVX2, 128, int) _mm_i32gather_epi32(data, a, sizeof(int));
            SIMD_CALL(AVX2, 256, int) _mm256_i32gather_epi32(data, a, sizeof(int));
            SIMD_CALL(AVX512F, 512, int) _mm512_i32gather_epi32(data, a, sizeof(int));
        }
    };

    template<class Ty, std::size_t Bits, simd_capability Capabilities>
    struct simd;
    
#define SIMD_BINARY_OP(OP, NAME)                                                                                \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
    concept can_##NAME = !requires(simd<Ty, Bits, Capabilities> a, simd<Ty, Bits, Capabilities> b) {            \
        { simd_operation<Ty, Bits, Capabilities>::NAME(a.value, b.value) } -> std::same_as<void>;               \
    };                                                                                                          \
                                                                                                                \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
        requires can_##NAME<Ty, Bits, Capabilities>                                                             \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a,                                                                         \
        simd<Ty, Bits, Capabilities> b) noexcept                                                                \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(a.value, b.value);                                  \
    }                                                                                                           \
                                                                                                                \
    template<class A, class Ty, std::size_t Bits, simd_capability Capabilities>                                 \
        requires (can_##NAME<Ty, Bits, Capabilities> &&                                                         \
            std::constructible_from<simd<Ty, Bits, Capabilities>, A>)                                           \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a, A b) noexcept                                                           \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(a.value, simd<Ty, Bits, Capabilities>{ b }.value);  \
    }                                                                                                           \
                                                                                                                \
    template<class A, class Ty, std::size_t Bits, simd_capability Capabilities>                                 \
        requires (can_##NAME<Ty, Bits, Capabilities> &&                                                         \
            std::constructible_from<simd<Ty, Bits, Capabilities>, A>)                                           \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
         A b, simd<Ty, Bits, Capabilities> a) noexcept                                                          \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(simd<Ty, Bits, Capabilities>{ b }.value, a.value);  \
    }
        
#define SIMD_BINARY_OP_SHIFT(OP, NAME)                                                                          \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
    concept can_##NAME = !requires(simd<Ty, Bits, Capabilities> a, simd<Ty, Bits, Capabilities> b) {            \
        { simd_operation<Ty, Bits, Capabilities>::NAME(a.value, b.value) } -> std::same_as<void>;               \
    };                                                                                                          \
                                                                                                                \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
        requires can_##NAME<Ty, Bits, Capabilities>                                                             \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a,                                                                         \
        simd<Ty, Bits, Capabilities> b) noexcept                                                                \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(a.value, b.value);                                  \
    }                                                                                                           \
                                                                                                                \
    template<class A, class Ty, std::size_t Bits, simd_capability Capabilities>                                 \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a, A b) noexcept                                                           \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(a.value, b);                                        \
    }                                                                                                           \
                                                                                                                \
    template<class A, class Ty, std::size_t Bits, simd_capability Capabilities>                                 \
        requires (can_##NAME<Ty, Bits, Capabilities> &&                                                         \
            std::constructible_from<simd<Ty, Bits, Capabilities>, A>)                                           \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
         A b, simd<Ty, Bits, Capabilities> a) noexcept                                                          \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(simd<Ty, Bits, Capabilities>{ b }.value, a.value);  \
    }
    
#define SIMD_UNARY_OP(OP, NAME)                                                                                 \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
    concept can_##NAME = !requires(simd<Ty, Bits, Capabilities> a) {                                            \
        { simd_operation<Ty, Bits, Capabilities>::NAME(a.value) } -> std::same_as<void>;                        \
    };                                                                                                          \
                                                                                                                \
    template<class Ty, std::size_t Bits, simd_capability Capabilities>                                          \
        requires can_##NAME<Ty, Bits, Capabilities>                                                             \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a) noexcept                                                                \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(a.value);                                           \
    }
        
#define SIMD_MEMBER_UNARY_OP(OP, NAME)                                                                          \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP() noexcept                                    \
        requires simd_operations::can_##NAME<Ty, Bits, Capabilities>                                            \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(value);                                             \
    }
        
#define SIMD_MEMBER_BINARY_OP(OP, NAME)                                                                         \
    KAIXO_INLINE simd<Ty, Bits, Capabilities> KAIXO_VECTORCALL OP(                                              \
        simd<Ty, Bits, Capabilities> a) noexcept                                                                \
        requires simd_operations::can_##NAME<Ty, Bits, Capabilities>                                            \
    {                                                                                                           \
        return simd_operation<Ty, Bits, Capabilities>::NAME(value, a.value);                                    \
    }

    SIMD_BINARY_OP(operator +, add);
    SIMD_BINARY_OP(operator -, sub);
    SIMD_BINARY_OP(operator *, mul);
    SIMD_BINARY_OP(operator /, div);
    SIMD_BINARY_OP(operator &, bit_and);
    SIMD_BINARY_OP(operator |, bit_or);
    SIMD_BINARY_OP(operator ^, bit_xor);
    SIMD_UNARY_OP(operator ~, bit_not);
    SIMD_BINARY_OP(operator ==, cmpeq);
    SIMD_BINARY_OP(operator !=, cmpneq);
    SIMD_BINARY_OP(operator >, cmpgt);
    SIMD_BINARY_OP(operator <, cmplt);
    SIMD_BINARY_OP(operator <=, cmple);
    SIMD_BINARY_OP(operator >=, cmpge);
    SIMD_BINARY_OP_SHIFT(operator >>, shift_right);
    SIMD_BINARY_OP_SHIFT(operator <<, shift_left);
    
    namespace simd_operations {
        SIMD_UNARY_OP(trunc, trunc);
        SIMD_UNARY_OP(floor, floor);
        SIMD_UNARY_OP(ceil, ceil);
        SIMD_UNARY_OP(round, round);
        SIMD_UNARY_OP(log, log);
        SIMD_UNARY_OP(log2, log2);
        SIMD_UNARY_OP(log10, log10);
        SIMD_UNARY_OP(sqrt, sqrt);
        SIMD_UNARY_OP(cbrt, cbrt);
        SIMD_UNARY_OP(exp, exp);
        SIMD_UNARY_OP(exp2, exp2);
        SIMD_UNARY_OP(exp10, exp10);
        SIMD_UNARY_OP(tanh, tanh);
        SIMD_UNARY_OP(abs, abs);
        SIMD_UNARY_OP(cos, cos);
        SIMD_UNARY_OP(cosh, cosh);
        SIMD_UNARY_OP(sin, sin);
        SIMD_UNARY_OP(sinh, sinh);
        SIMD_BINARY_OP(pow, pow);

        template<class Ty, std::size_t Bits, simd_capability Capabilities>
        concept can_sum = !requires(simd<Ty, Bits, Capabilities> a) {
            { simd_operation<Ty, Bits, Capabilities>::sum(a.value) } -> std::same_as<void>;
        };
 
        template<class Ty, std::size_t Bits, simd_capability Capabilities>
            requires can_sum<Ty, Bits, Capabilities>
        KAIXO_INLINE Ty KAIXO_VECTORCALL sum(
            simd<Ty, Bits, Capabilities> a) noexcept
        {
            return simd_operation<Ty, Bits, Capabilities>::sum(a.value);
        }

        template<class To, class Ty, std::size_t Bits, simd_capability Capabilities>
        concept can_cast = !requires(simd<Ty, Bits, Capabilities> a) {
            { simd_operation<Ty, Bits, Capabilities>::template cast<To>(a.value) } -> std::same_as<void>;
        };

        template<class To, class Ty, std::size_t Bits, simd_capability Capabilities>
            requires can_cast<To, Ty, Bits, Capabilities>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL cast(
            const simd<Ty, Bits, Capabilities>& a) noexcept
        {
            return simd_operation<Ty, Bits, Capabilities>::template cast<To>(a.value);
        }

        template<class To, class Ty, std::size_t Bits, simd_capability Capabilities>
        concept can_reinterpret = !requires(simd<Ty, Bits, Capabilities> a) {
            { simd_operation<Ty, Bits, Capabilities>::template reinterpret<To>(a.value) } -> std::same_as<void>;
        };

        template<class To, class Ty, std::size_t Bits, simd_capability Capabilities>
            requires can_reinterpret<To, Ty, Bits, Capabilities>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL reinterpret(
            const simd<Ty, Bits, Capabilities>& a) noexcept
        {
            return simd_operation<Ty, Bits, Capabilities>::template reinterpret<To>(a.value);
        }
    }
    
    template<class Ty, std::size_t Bits, simd_capability Capabilities>
    struct simd {
        constexpr static std::size_t bits = Bits;
        constexpr static std::size_t bytes = Bits / 8;
        constexpr static std::size_t elements = bytes / sizeof(Ty);
        constexpr static simd_capability capabilities = Capabilities;

        using base = Ty;
        using simd_type = underlying_simd_t<Ty, Bits>;
        using operation = simd_operation<Ty, Bits, Capabilities>;

        simd_type value{};

        KAIXO_INLINE static simd one() { return operation::setone(); }
        KAIXO_INLINE static simd zero() { return operation::setzero(); }
        KAIXO_INLINE static simd aligned(base const* addr) { return operation::load(addr); }
            
        KAIXO_INLINE simd() : value() {}
        KAIXO_INLINE simd(base const* addr) : value(operation::loadu(addr)) {}
        KAIXO_INLINE simd(base val) : value(operation::set1(val)) {}
        KAIXO_INLINE simd(simd_type val) : value(val) {}
        KAIXO_INLINE simd(bool val) : value(val ? one().value : zero().value) {}

        template<class ...Args> requires (sizeof...(Args) == elements && (std::same_as<base, Args> && ...))
        KAIXO_INLINE simd(Args ... args) : value(operation::setr(args...)) {}

        KAIXO_INLINE void KAIXO_VECTORCALL store(base* addr) const noexcept { operation::store(value, addr); }
        KAIXO_INLINE void KAIXO_VECTORCALL storeu(base* addr) const noexcept { operation::storeu(value, addr); }
        KAIXO_INLINE void KAIXO_VECTORCALL get(base const* addr) const noexcept { operation::storeu(value, addr); }
        KAIXO_INLINE void KAIXO_VECTORCALL aligned_get(base const* addr) const noexcept { operation::store(value, addr); }

        SIMD_MEMBER_UNARY_OP(trunc, trunc);
        SIMD_MEMBER_UNARY_OP(floor, floor);
        SIMD_MEMBER_UNARY_OP(ceil, ceil);
        SIMD_MEMBER_UNARY_OP(round, round);
        SIMD_MEMBER_UNARY_OP(log, log);
        SIMD_MEMBER_UNARY_OP(log2, log2);
        SIMD_MEMBER_UNARY_OP(log10, log10);
        SIMD_MEMBER_UNARY_OP(sqrt, sqrt);
        SIMD_MEMBER_UNARY_OP(cbrt, cbrt);
        SIMD_MEMBER_UNARY_OP(exp, exp);
        SIMD_MEMBER_UNARY_OP(exp2, exp2);
        SIMD_MEMBER_UNARY_OP(exp10, exp10);
        SIMD_MEMBER_UNARY_OP(tanh, tanh);
        SIMD_MEMBER_UNARY_OP(abs, abs);
        SIMD_MEMBER_UNARY_OP(cos, cos);
        SIMD_MEMBER_UNARY_OP(cosh, cosh);
        SIMD_MEMBER_UNARY_OP(sin, sin);
        SIMD_MEMBER_UNARY_OP(sinh, sinh);
        SIMD_MEMBER_BINARY_OP(pow, pow);

        KAIXO_INLINE base KAIXO_VECTORCALL sum() noexcept 
            requires simd_operations::can_sum<Ty, Bits, Capabilities> {
            return simd_operation<Ty, Bits, Capabilities>::sum(value);
        }
        
        template<class To> requires simd_operations::can_reinterpret<To, Ty, Bits, Capabilities>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL reinterpret() noexcept {
            return simd_operation<Ty, Bits, Capabilities>::template reinterpret<To>(value);
        }

        template<class To> requires simd_operations::can_cast<To, Ty, Bits, Capabilities>
        KAIXO_INLINE simd<To, Bits, Capabilities> KAIXO_VECTORCALL cast() noexcept {
            return simd_operation<Ty, Bits, Capabilities>::template cast<To>(value);
        }
    };

    template<class Simd>
    concept is_simd = requires() {
        typename Simd::base;
        typename Simd::simd_type;
        typename Simd::operation;
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
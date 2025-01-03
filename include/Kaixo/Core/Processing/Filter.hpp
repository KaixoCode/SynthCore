#pragma once
#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    enum class FilterType {
        LowPass4,
        LowPass,
        HighShelf,
        PeakingEQ,
        LowShelf,
        HighPass,
        HighPass4,
        BandPass,
        Notch,
        AllPass,
        Amount,
    };

    // ------------------------------------------------

    /**
     * Convert a parameter value for the filter type of an EQ to the filter type.
     * @param value normalized parameter value
     * @return the filter type
     */
    constexpr Processing::FilterType eqParamTypeToFilterType(ParamValue value) {
        constexpr FilterType indexToFilterType[]{
            FilterType::LowPass4,
            FilterType::LowPass,
            FilterType::HighShelf,
            FilterType::PeakingEQ,
            FilterType::LowShelf,
            FilterType::HighPass,
            FilterType::HighPass4,
        };
        constexpr std::size_t filterTypes = sizeof(indexToFilterType) / sizeof(FilterType);
        return indexToFilterType[normalToIndex(value, filterTypes)];
    };

    // ------------------------------------------------

    template<class MathQuality = Math, 
             std::size_t Parallel = 1, 
             std::size_t MaxPasses = 4,
             FilterType ...FilterTypes>
    struct Biquad {

        // ------------------------------------------------

        struct Coefficients {
            std::array<float, 3> a{};
            std::array<float, 3> b{};

            float a1a0{};
            float a2a0{};
            float b0a0{};
            float b1a0{};
            float b2a0{};
        };

        // ------------------------------------------------

        struct State {
            struct Pass {
                alignas(64) std::array<std::array<float, Parallel>, 3> y{};
                alignas(64) std::array<std::array<float, Parallel>, 3> x{};
            } pass[MaxPasses]{};
        };

        // ------------------------------------------------

        constexpr bool quadruple() const { return m_Type == FilterType::LowPass4 || m_Type == FilterType::HighPass4; }
        constexpr std::size_t passes() const { return m_Passes; }
        constexpr FilterType type() const { return m_Type; }
        constexpr void sampleRate(float sr) { set(sr, m_SampleRate); }
        constexpr void gain(float gain) { set(gain, m_Gain); }
        constexpr void frequency(float frequency) { set(frequency, m_Frequency); }
        constexpr void resonance(float q) { set(q, m_Q); }
        constexpr void passes(std::size_t p) { set(p, m_SetPasses); }
        constexpr void type(FilterType type) { set(type, m_Type); }
        constexpr void type(float t) {
            if constexpr (sizeof...(FilterTypes) == 0) {
                type(eqParamTypeToFilterType(t));
            } else {
                constexpr FilterType types[]{ FilterTypes... };
                type(types[normalToIndex(t, sizeof...(FilterTypes))]);
            }
        }

        // ------------------------------------------------

        bool bypass = false;

        // ------------------------------------------------

        constexpr Coefficients& getCoefficients() {
            if (dirty) recalculate();
            return m_Coefficients;
        }

        constexpr State& getState(std::size_t i) { return m_States[i]; }

        // ------------------------------------------------

        void reset() { std::memset(m_States, 0, sizeof(m_States)); }

        // ------------------------------------------------

        Stereo process(Stereo x) {
            Stereo result = { processBatch(x.l, 0), processBatch(x.r, 1), };
            finalizeBatches();
            return result;
        }

        /**
         * Process a single filter batch, used for left/right,
         * and possible Parallel filters used with SIMD.
         * @param in input type, can be simd type
         * @param index use 0 for left, and 1 for right
         * @param i start at the i'th parallel filter, used with simd
         */
        template<class Type> requires (is_simd<Type> || is_mono<Type>)
        Type processBatch(Type in, std::size_t index, std::size_t i = 0) {
            if (bypass) return in;
            auto& coeff = getCoefficients();
            auto& state = getState(index);

            if constexpr (MaxPasses != 1) {
                auto singlePass = [&](auto& pass, auto input) {
                    store(pass.x[m_0].data() + i, input);
                    store(pass.y[m_0].data() + i,
                          coeff.b0a0 * input
                        + coeff.b1a0 * load<Type>(pass.x[m_1].data(), i)
                        + coeff.b2a0 * load<Type>(pass.x[m_2].data(), i)
                        - coeff.a1a0 * load<Type>(pass.y[m_1].data(), i)
                        - coeff.a2a0 * load<Type>(pass.y[m_2].data(), i));
                };

                singlePass(state.pass[0], in);

                for (std::size_t j = 1; j < passes(); ++j) {
                    singlePass(state.pass[j], load<Type>(state.pass[j - 1].y[m_0].data(), i));
                }

                return load<Type>(state.pass[passes() - 1].y[m_0].data(), i);
            }
            else {
                auto& pass = state.pass[0];

                store(pass.x[m_0].data() + i, in);

                auto result =
                    coeff.b0a0 * in
                    + coeff.b1a0 * load<Type>(pass.x[m_1].data(), i)
                    + coeff.b2a0 * load<Type>(pass.x[m_2].data(), i)
                    - coeff.a1a0 * load<Type>(pass.y[m_1].data(), i)
                    - coeff.a2a0 * load<Type>(pass.y[m_2].data(), i);

                store(pass.y[m_0].data() + i, result);

                return result;
            }
        }

        void finalizeBatches() {
            auto backup = m_2;
            m_2 = m_1;
            m_1 = m_0;
            m_0 = backup;
        }

        // ------------------------------------------------

    protected:
        float m_SampleRate = 48000;
        float m_Frequency = 22000;
        float m_Gain = 0;
        float m_Q = 0;
        FilterType m_Type = FilterType::LowPass;
        bool dirty = true;
        Coefficients m_Coefficients;
        State m_States[2];
        std::size_t m_Passes = 1;
        std::size_t m_SetPasses = 1;
        std::size_t m_0 = 0;
        std::size_t m_1 = 1;
        std::size_t m_2 = 2;

        // ------------------------------------------------

        constexpr void set(auto v, auto& me) { if (v != me) { me = v; dirty = true; } }
        constexpr float normalizedFrequency() const { return MathQuality::clamp(m_Frequency / m_SampleRate, 0., 0.5); }
        constexpr float normalizedQ() const {
            using enum FilterType;
            switch (m_Type) {
            case LowPass4:
            case HighPass4:
                return m_Q * 0.19;
            case LowPass:
            case HighPass:
                return Math::Fast::powN<4>(m_Q);
            case HighShelf:
            case LowShelf:
                return 8 * m_Q / (Math::Fast::abs(m_Gain) + 1);
            case PeakingEQ:
                return 2 * m_Q + 0.2;
            case Notch:
                return 8 * (1 - m_Q) + 0.5;
            case BandPass:
                return 0.5 * m_Q;
            }
            return m_Q;
        }

        // ------------------------------------------------

        constexpr void recalculate() {
            dirty = false;
            constexpr float log10_2 = std::numbers::ln2 / std::numbers::ln10;
            using enum FilterType;
            const float frequency = normalizedFrequency();
            const float omega = 2 * std::numbers::pi * frequency;
            const float cosOmega = MathQuality::ncos(frequency);
            const float sinOmega = MathQuality::nsin(frequency);
            const float Q = normalizedQ();
            const float gain = m_Gain;
            auto& coeffs = m_Coefficients;

            if (quadruple()) m_Passes = 4;
            else m_Passes = m_SetPasses;

            const auto defaultA = [&](float alpha) {
                coeffs.a = {
                    1.0f + alpha,
                   -2.0f * cosOmega,
                    1.0f - alpha
                };
            };

            // ------------------------------------------------

            switch (m_Type) {

                // ------------------------------------------------

            case LowPass:
            case LowPass4: {
                defaultA(sinOmega / (2.0f * (Q * 6 + 0.4f)));
                coeffs.b = {
                    (1.0f - cosOmega) / 2.0f,
                    (1.0f - cosOmega),
                    (1.0f - cosOmega) / 2.0f
                };
            } break;

                // ------------------------------------------------

            case HighPass:
            case HighPass4: {
                defaultA(sinOmega / (2.0f * (Q * 6 + 0.4f)));
                coeffs.b = {
                    (1.0f + cosOmega) / 2.0f,
                   -(1.0f + cosOmega),
                    (1.0f + cosOmega) / 2.0f
                };
            } break;

                // ------------------------------------------------

            case BandPass: {
                defaultA(sinOmega * MathQuality::sinh((log10_2 / 2.0f) * (1 / (Q * 4 + 0.2f)) * (omega / sinOmega)));
                coeffs.b = {
                    sinOmega / 2.0f,
                    0.0f,
                   -sinOmega / 2.0f
                };
            } break;

                // ------------------------------------------------

            case Notch: {
                defaultA(sinOmega * MathQuality::sinh((log10_2 / 2.0f) * Q * (omega / sinOmega)));
                coeffs.b = {
                    1.0f,
                   -2.0f * cosOmega,
                    1.0f
                };
            } break;

                // ------------------------------------------------

            case AllPass: {
                float alpha = sinOmega / (2.0f * (Q * 4 + 0.2f));
                defaultA(alpha);
                coeffs.b = {
                    1.0f - alpha,
                   -2.0f * cosOmega,
                    1.0f + alpha
                };
            } break;

                // ------------------------------------------------

            case PeakingEQ: {
                float A = MathQuality::pow(10, gain / 40.0f);
                float alpha = sinOmega * MathQuality::sinh((log10_2 / 2.0f) * (Q * 4 + 0.2f) * (omega / sinOmega));
                coeffs.a = {
                    1.0f + alpha / A,
                   -2.0f * cosOmega,
                    1.0f - alpha / A
                };

                coeffs.b = {
                    1.0f + alpha * A,
                   -2.0f * cosOmega,
                    1.0f - alpha * A
                };
            } break;

                // ------------------------------------------------

            case LowShelf: {
                float A = MathQuality::pow(10, gain / 40.0f);
                float t = MathQuality::max((A + 1.0f / A) * (1.0f / (Q * 3 + 0.5f) - 1.0f) + 2, 0.0f);
                float alpha = (sinOmega / 2.0f) * MathQuality::sqrt(t);
                float sqrtAa = MathQuality::sqrt(A) * alpha;
                coeffs.a = {
                            (A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtAa,
                   -2.0f * ((A - 1.0f) + (A + 1.0f) * cosOmega),
                            (A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtAa,
                };

                coeffs.b = {
                           A * ((A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtAa),
                    2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosOmega),
                           A * ((A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtAa),
                };
            } break;

                // ------------------------------------------------

            case HighShelf: {
                float A = MathQuality::pow(10, gain / 40.0f);
                float t = MathQuality::max((A + 1.0f / A) * (1.0f / (Q * 3 + 0.5f) - 1.0f) + 2, 0.0f);
                float alpha = (sinOmega / 2.0f) * MathQuality::sqrt(t);
                float sqrtAa = MathQuality::sqrt(A) * alpha;
                coeffs.a = {
                            (A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtAa,
                    2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega),
                            (A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtAa,
                };

                coeffs.b = {
                           A * ((A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtAa),
                   -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega),
                           A * ((A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtAa),
                };
            } break;
            }

            coeffs.b0a0 = coeffs.b[0] / coeffs.a[0];
            coeffs.b1a0 = coeffs.b[1] / coeffs.a[0];
            coeffs.b2a0 = coeffs.b[2] / coeffs.a[0];
            coeffs.a1a0 = coeffs.a[1] / coeffs.a[0];
            coeffs.a2a0 = coeffs.a[2] / coeffs.a[0];
        }
    };

    // ------------------------------------------------

    inline float ellipticIntegral(float v) {
        constexpr int M = 4;
        float K = std::numbers::pi / 2;
        auto lastK = v;

        for (int i = 0; i < M; ++i) {
            lastK = std::pow(lastK / (1 + std::sqrt(1 - std::pow(lastK, 2.0))), 2.0);
            K *= 1 + lastK;
        }

        return K;
    }

    inline std::pair<float, float> ellipticIntegralK(float k) {
        return {
            ellipticIntegral(k),
            ellipticIntegral(std::sqrt(1 - k * k)),
        };
    }

    inline std::complex<float> asne(std::complex<float> w, float k) noexcept {
        constexpr int M = 4;

        float ke[M + 1];
        float* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1.f + std::sqrt(1.f - std::pow(*kei, 2.f))), 2.f);
            *++kei = next;
        }

        std::complex<float> last = w;

        for (int i = 1; i <= M; ++i)
            last = 2.f * last / ((1.f + ke[i]) * (1.f + std::sqrt(1.f - std::pow(ke[i - 1] * last, 2.f))));

        return 2.f / std::numbers::pi_v<float> * std::asin(last);
    }

    inline std::complex<float> sne(std::complex<float> u, float k) noexcept {
        constexpr int M = 4;

        float ke[M + 1];
        float* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1.f + std::sqrt(1 - std::pow(*kei, 2.f))), 2.f);
            *++kei = next;
        }

        std::complex<float> last = std::sin(u * static_cast<float>(std::numbers::pi / 2.f));

        for (int i = M - 1; i >= 0; --i)
            last = (1.f + ke[i + 1]) / (1.f / last + ke[i + 1] * last);

        return last;
    }

    inline std::complex<float> cde(std::complex<float> u, float k) noexcept {
        constexpr int M = 4;

        float ke[M + 1];
        float* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1.f + std::sqrt(1.f - std::pow(*kei, 2.f))), 2.f);
            *++kei = next;
        }

        std::complex<float> last = std::cos(u * static_cast<float>(std::numbers::pi / 2.f));

        for (int i = M - 1; i >= 0; --i)
            last = (1.f + ke[i + 1]) / (1.f / last + ke[i + 1] * last);

        return last;
    }

    // High order minimal phase infinite impulse response elliptic
    // lowpass filter used for anti-aliasing
    class EllipticParameters {
        double pf0 = 0;
        double psampleRate = 0;
    public:
        // Cannot change dynamically!
        double passbandAmplitudedB = -1;
        double stopbandAmplitudedB = -80;
        double normalisedTransitionWidth = 0.001;
        // Can change:
        double f0 = 20000;
        double sampleRate = 44100;
        FilterType type = FilterType::LowPass;

        struct Coeficients {
            int order = 1;
            double b[4];
            double a[4];

            Stereo state[4]{};
        };

        std::vector<Coeficients> coeficients;

        void recalculateParameters() {
            if (pf0 == f0 && psampleRate == sampleRate) return;

            pf0 = f0;
            psampleRate = sampleRate;

            assert(0 < sampleRate);
            assert(0 < f0 && f0 <= sampleRate * 0.5);
            assert(0 < normalisedTransitionWidth && normalisedTransitionWidth <= 0.5);
            assert(-20 < passbandAmplitudedB && passbandAmplitudedB < 0);
            assert(-300 < stopbandAmplitudedB && stopbandAmplitudedB < -20);

            float normalisedFrequency = f0 / sampleRate;

            float fp = normalisedFrequency - normalisedTransitionWidth / 2;
            assert(0.0 < fp && fp < 0.5);

            float fs = normalisedFrequency + normalisedTransitionWidth / 2;
            assert(0.0 < fs && fs < 0.5);

            float Ap = passbandAmplitudedB;
            float As = stopbandAmplitudedB;
            float Gp = Math::Fast::db_to_magnitude(Ap);
            float Gs = Math::Fast::db_to_magnitude(As);
            float epsp = std::sqrt(1.f / (Gp * Gp) - 1.f);
            float epss = std::sqrt(1.f / (Gs * Gs) - 1.f);

            float omegap = std::tan(std::numbers::pi * fp);
            float omegas = std::tan(std::numbers::pi * fs);

            float k = omegap / omegas;
            float k1 = epsp / epss;

            int N;

            auto [K, Kp]  = ellipticIntegralK(k);
            auto [K1, K1p]  = ellipticIntegralK(k1);

            N = std::round(std::ceil((K1p * K) / (K1 * Kp)));

            const std::size_t r = N % 2;
            const std::size_t L = (N - r) / 2;
            const float H0 = std::pow(Gp, 1.0 - r);

            std::vector<std::complex<float>> pa, za;
            pa.reserve(L);
            za.reserve(L);
            std::complex<float> j(0, 1);

            auto v0 = -j * (asne(j / epsp, k1) / (float)N);

            if (r == 1) pa.push_back(omegap * j * sne(j * v0, k));

            for (std::size_t i = 1; i <= L; ++i) {
                auto ui = (2 * i - 1.0f) / (float)N;
                auto zetai = cde(ui, k);

                pa.push_back(omegap * j * cde(ui - j * v0, k));
                za.push_back(omegap * j / (k * zetai));
            }

            std::vector<std::complex<float>> p, z, g;
            p.reserve(L + 1);
            z.reserve(L + 1);
            g.reserve(L + 1);

            if (r == 1) {
                p.push_back((1.f + pa[0]) / (1.f - pa[0]));
                g.push_back(0.5f * (1.f - p[0]));
            }

            for (std::size_t i = 0; i < L; ++i) {
                p.push_back((1.f + pa[i + r]) / (1.f - pa[i + r]));
                z.push_back(za.size() == 0 ? -1.f : (1.f + za[i]) / (1.f - za[i]));
                g.push_back((1.f - p[i + r]) / (1.f - z[i]));
            }

            coeficients.clear();

            if (r == 1) {
                auto b0 = static_cast<double> (H0 * std::real(g[0]));
                auto b1 = b0;
                auto a1 = static_cast<double> (-std::real(p[0]));

                coeficients.push_back({ .order = 1, .b = { b0, b1 }, .a = { 1.0, a1 } });
            }

            for (std::size_t i = 0; i < L; ++i) {
                auto gain = std::pow(std::abs(g[i + r]), 2.0);

                auto b0 = static_cast<double> (gain);
                auto b1 = static_cast<double> (std::real(-z[i] - std::conj(z[i])) * gain);
                auto b2 = static_cast<double> (std::real(z[i] * std::conj(z[i])) * gain);

                auto a1 = static_cast<double> (std::real(-p[i + r] - std::conj(p[i + r])));
                auto a2 = static_cast<double> (std::real(p[i + r] * std::conj(p[i + r])));

                coeficients.push_back({ .order = 2, .b = { b0, b1, b2 }, .a = { 1.0, a1, a2 } });
            }
        }

        void reset() {
            for (auto& c : coeficients) {
                for (auto& [l, r] : c.state) {
                    l = 0;
                    r = 0;
                }
            }
        }
    };

    // ------------------------------------------------

    class EllipticFilter {
    public:
        using Params = EllipticParameters;

        Stereo process(Stereo s, Params& p) {
            Stereo res = s;
            for (auto& c : p.coeficients) {
                auto output = (c.b[0] * res) + c.state[0];

                for (int j = 0; j < c.order - 1; ++j)
                    c.state[j] = (c.b[j + 1] * res) - (c.a[j + 1] * output) + c.state[j + 1];

                c.state[c.order - 1] = (c.b[c.order] * res) - (c.a[c.order] * output);
                res = output;
            }

            return res;
        }

        float process(float s, Params& p) {
            float res = s;
            for (auto& c : p.coeficients) {
                auto output = (c.b[0] * res) + c.state[0].l;

                for (int j = 0; j < c.order - 1; ++j)
                    c.state[j].l = (c.b[j + 1] * res) - (c.a[j + 1] * output) + c.state[j + 1].l;

                c.state[c.order - 1].l = (c.b[c.order] * res) - (c.a[c.order] * output);
                res = output;
            }

            return res;
        }
    };

    // ------------------------------------------------

    //template<class MathQuality = Math,
    //         std::size_t Parallel = 1,
    //         std::size_t MaxStages = 32>
    class AntiAliasFilter {
    public:

        // ------------------------------------------------
        
        using MathQuality = Math;
        constexpr static std::size_t Parallel = 1;
        constexpr static std::size_t MaxStages = 32;

        // ------------------------------------------------
        
        constexpr static std::size_t MaxOrder = MaxStages * 2; // Using 2nd order stages

        struct Stage {

            // ------------------------------------------------

            struct State {
                alignas(64) float state[3][Parallel]{};
            };

            // ------------------------------------------------

            State left{};
            State right{};

            // ------------------------------------------------

            std::size_t order = 2;
            float a[3]{};
            float b[3]{};

            // ------------------------------------------------

        };

        // ------------------------------------------------
        
        float sampleRate = 48000;
        float cutoff = 21000;
        float transitionWidth = 1000;
        float passbandAmplitude = 0.89;   // in magnitude; ~=  -1dB
        float stopbandAmplitude = 0.0001; // in magnitude; ~= -80dB

        // ------------------------------------------------

        Vector<Stage, MaxStages> stages;

        // ------------------------------------------------
        
        Stereo process(Stereo input) {
            Stereo res = input;
            for (Stage& stage : stages) {
                auto l = (stage.b[0] * res.l) + stage.left.state[0][0];
                auto r = (stage.b[0] * res.r) + stage.right.state[0][0];
            
                if (stage.order == 1) {
                    stage.left.state[0][0] = (stage.b[1] * res.l) - (stage.a[1] * l);
            
                    stage.right.state[0][0] = (stage.b[1] * res.r) - (stage.a[1] * r);
                } else {
                    stage.left.state[0][0] = (stage.b[1] * res.l) - (stage.a[1] * l) + stage.left.state[1][0];
                    stage.left.state[1][0] = (stage.b[2] * res.l) - (stage.a[2] * l);
            
                    stage.right.state[0][0] = (stage.b[1] * res.r) - (stage.a[1] * r) + stage.right.state[1][0];
                    stage.right.state[1][0] = (stage.b[2] * res.r) - (stage.a[2] * r);
                }
            
                res = { l, r };
            }
            
            return res;
        }

        // ------------------------------------------------
        
        void recalculateCoefficients() {
        
            // https://en.wikipedia.org/wiki/Elliptic_filter

            const float normalizedFrequency = Math::Fast::min(cutoff / sampleRate, 0.5);
            const float normalizedTransitionWidth = Math::Fast::min(transitionWidth / sampleRate, 0.5);
            const float passbandFrequency = Math::Fast::max(normalizedFrequency - normalizedTransitionWidth / 2, 0.0);
            const float stopbandFrequency = Math::Fast::min(normalizedFrequency + normalizedTransitionWidth / 2, 0.5);

            const float epsilonPassband = std::sqrt(1.f / (passbandAmplitude * passbandAmplitude) - 1.f);
            const float epsilonStopband = std::sqrt(1.f / (stopbandAmplitude * stopbandAmplitude) - 1.f);

            const float omegaPassband = std::tan(std::numbers::pi * passbandFrequency);
            const float omegaStopband = std::tan(std::numbers::pi * stopbandFrequency);
             
            // Calculate order using elliptic integral
            const float k = omegaPassband / omegaStopband;
            const float k1 = epsilonPassband / epsilonStopband;

            const auto [K, Kp] = ellipticIntegralK(k);
            const auto [K1, K1p] = ellipticIntegralK(k1);

            const std::size_t order2 = Math::min(Math::ceil((K1p * K) / (K1 * Kp)), MaxOrder);
            const std::size_t remainder = 0; // order % 2;
            const std::size_t nofStages = Math::min((order2 + 1) / 2, MaxStages);
            const std::size_t order = nofStages * 2;
            // ^^^ This ignores a potential additional 1st order filter
            //     since this is an anti-alias filter, the exact pass/stop band
            //     specifications aren't crucial, so this 1st order filter
            //     is simply left out for convenience.
            
            constexpr std::complex<float> j{ 0, 1 };
            const std::complex<float> v0 = -j * (asne(j / epsilonPassband, k1) / static_cast<float>(order));
            
            stages.clear();
            
            if (remainder == 1) {
                const std::complex<float> pa = omegaPassband * j * sne(j * v0, k);
                const std::complex<float> p = (1.f + pa) / (1.f - pa);
                const std::complex<float> g = 0.5f * (1.f - p);
            
                Stage& stage = stages.emplace_back();
                stage.order = 1;
                stage.a[1] = -std::real(p);
                stage.b[0] = std::real(g);
                stage.b[1] = stage.b[0];
            }
            
            for (std::size_t i = 0; i < nofStages; ++i) {
                const float ui = (2 * (i + 1) - 1.f) / static_cast<float>(order);
                const std::complex<float> pa = omegaPassband * j * cde(ui - j * v0, k);
                const std::complex<float> za = omegaPassband * j / (k * cde(ui, k));
                const std::complex<float> p = (1.f + pa) / (1.f - pa);
                const std::complex<float> z = (1.f + za) / (1.f - za);
                const std::complex<float> g = (1.f - p) / (1.f - z);
            
                const float gain = std::pow(std::abs(g), 2.f);
            
                Stage& stage = stages.emplace_back();
                stage.order = 2;
                stage.a[1] = std::real(-p - std::conj(p));
                stage.a[2] = std::real(p * std::conj(p));
                stage.b[0] = gain;
                stage.b[1] = std::real(-z - std::conj(z)) * gain;
                stage.b[2] = std::real(z * std::conj(z)) * gain;
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    struct AAFilter {
        double sampleRateIn = 44100;
        double sampleRateOut = 48000;

        Stereo process(Stereo s) {
            auto srate = sampleRateIn < sampleRateOut ? sampleRateOut : sampleRateIn;
            params.f0 = sampleRateOut / 2 - 2;
            params.sampleRate = srate;
            params.recalculateParameters();
            return filter.process(s, params);
        }
        
        float process(float s) {
            auto srate = sampleRateIn < sampleRateOut ? sampleRateOut : sampleRateIn;
            params.f0 = sampleRateOut / 2 - 2;
            params.sampleRate = srate;
            params.recalculateParameters();
            return filter.process(s, params);
        }

        void reset() {
            params.reset();
        }

        EllipticFilter filter;
        EllipticParameters params;
    };

    // ------------------------------------------------

    template<std::size_t N, class MathQuality = Math, std::size_t MaxPasses = 4, FilterType ...FilterTypes>
    class StereoEqualizer : public std::array<Biquad<MathQuality, 1, MaxPasses, FilterTypes...>, N>, public Module {
    public:

        // ------------------------------------------------

        Stereo input;
        Stereo output;

        // ------------------------------------------------

        void process() override {
            output = input;
            for (auto& filter : *this) {
                output = filter.process(output);
            }
            input = { 0, 0 };
        }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            for (auto& filter : *this) {
                filter.sampleRate(sampleRate);
            }
        }

        void reset() override {
            for (auto& filter : *this) {
                filter.reset();
            }
        }

        // ------------------------------------------------

    };

    // ------------------------------------------------

    template<std::size_t N, class MathQuality = Math, std::size_t Parallel = 1, std::size_t MaxPasses = 4, FilterType ...FilterTypes>
    class BatchEqualizer : public std::array<Biquad<MathQuality, Parallel, MaxPasses, FilterTypes...>, N>, public Module {
    public:

        // ------------------------------------------------
        
        void process() override {}; // No default process

        template<class Type>
        Type processBatch(Type in, std::size_t index, std::size_t i = 0) {
            for (auto& filter : *this) in = filter.processBatch(in, index, i);
            return in;
        }

        void finalizeBatches() {
            for (auto& filter : *this) filter.finalizeBatches();
        }

        // ------------------------------------------------

        void reset() override { for (auto& filter : *this) filter.reset(); }

        void prepare(double sampleRate, std::size_t maxBufferSize) override {
            for (auto& filter : *this) {
                filter.sampleRate(sampleRate);
            }
        }

        // ------------------------------------------------

    };

    // Simple specialization for 0 filters, does nothing
    template<class MathQuality, std::size_t Parallel, std::size_t MaxPasses, FilterType ...FilterTypes>
    class BatchEqualizer<0, MathQuality, Parallel, MaxPasses, FilterTypes...> {
    public:
        template<class Type>
        constexpr Type processBatch(Type value, std::size_t, std::size_t = 0) const noexcept { return value; }

        constexpr void finalizeBatches() {}
        constexpr void reset() {}
    };
}
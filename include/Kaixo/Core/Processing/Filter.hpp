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

    template<class Sample = double, class MathQuality = Math, std::size_t Parallel = 1, bool Quadruple = true>
    struct Biquad {

        // ------------------------------------------------

        struct Coefficients {
            std::array<double, 3> a;
            std::array<double, 3> b;

            double a1a0;
            double a2a0;
            double b0a0;
            double b1a0;
            double b2a0;
        };

        // ------------------------------------------------

        struct State {
            struct Pass {
                alignas(64) std::array<std::array<Sample, Parallel>, 3> y;
                alignas(64) std::array<std::array<Sample, Parallel>, 3> x;
            } pass[(Quadruple ? 4 : 1)];
        };

        // ------------------------------------------------

        constexpr bool quadruple() const { return m_Type == FilterType::LowPass4 || m_Type == FilterType::HighPass4; }
        constexpr void sampleRate(double sr) { set(sr, m_SampleRate); }
        constexpr void gain(double gain) { set(gain, m_Gain); }
        constexpr void frequency(double frequency) { set(frequency, m_Frequency); }
        constexpr void resonance(double q) { set(q, m_Q); }
        constexpr void type(FilterType type) { set(type, m_Type); }
        constexpr void type(double t) { type(eqParamTypeToFilterType(t)); }

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

            constexpr auto aat = [](auto ptr, auto i) constexpr -> decltype(auto) {
                if constexpr (std::floating_point<Type>) return ptr[i];
                else return Type::aligned(ptr + i);
            };

            constexpr auto store = [](auto ptr, auto value) constexpr -> decltype(auto) {
                if constexpr (std::floating_point<Type>) return *ptr = value;
                else return value.store(ptr);
            };

            if constexpr (Quadruple) {
                auto singlePass = [&](auto& pass, auto input) {
                    store(pass.x[m_0].data() + i, input);
                    store(pass.y[m_0].data() + i,
                        coeff.b0a0 * input
                        + coeff.b1a0 * aat(pass.x[m_1].data(), i)
                        + coeff.b2a0 * aat(pass.x[m_2].data(), i)
                        - coeff.a1a0 * aat(pass.y[m_1].data(), i)
                        - coeff.a2a0 * aat(pass.y[m_2].data(), i));
                    };

                singlePass(state.pass[0], in);

                if (quadruple()) {
                    for (std::size_t j = 1; j < 4; ++j) {
                        singlePass(state.pass[j], aat(state.pass[j - 1].y[m_0].data(), i));
                    }

                    return aat(state.pass[3].y[m_0].data(), i);
                }

                return aat(state.pass[0].y[m_0].data(), i);
            }
            else {
                auto& pass = state.pass[0];

                store(pass.x[m_0].data() + i, in);

                auto result =
                    coeff.b0a0 * in
                    + coeff.b1a0 * aat(pass.x[m_1].data(), i)
                    + coeff.b2a0 * aat(pass.x[m_2].data(), i)
                    - coeff.a1a0 * aat(pass.y[m_1].data(), i)
                    - coeff.a2a0 * aat(pass.y[m_2].data(), i);

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
        double m_SampleRate = 48000;
        double m_Frequency = 22000;
        double m_Gain = 0;
        double m_Q = 0;
        FilterType m_Type = FilterType::LowPass;
        bool dirty = true;
        Coefficients m_Coefficients;
        State m_States[2];
        std::size_t m_0 = 0;
        std::size_t m_1 = 1;
        std::size_t m_2 = 2;

        // ------------------------------------------------

        constexpr void set(auto v, auto& me) { if (v != me) { me = v; dirty = true; } }
        constexpr double normalizedFrequency() const { return MathQuality::clamp(m_Frequency / m_SampleRate, 0., 0.5); }
        constexpr double normalizedQ() const { 
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
            }
            return m_Q;
        }

        // ------------------------------------------------

        constexpr void recalculate() {
            dirty = false;
            constexpr double log10_2 = std::numbers::ln2 / std::numbers::ln10;
            using enum FilterType;
            const double frequency = normalizedFrequency();
            const double omega = 2 * std::numbers::pi * frequency;
            const double cosOmega = MathQuality::ncos(frequency);
            const double sinOmega = MathQuality::nsin(frequency);
            const double Q = normalizedQ();
            const double gain = m_Gain;
            auto& coeffs = m_Coefficients;

            const auto defaultA = [&](double alpha) {
                coeffs.a = {
                    1.0 + alpha,
                   -2.0 * cosOmega,
                    1.0 - alpha
                };
                };

            // ------------------------------------------------

            switch (m_Type) {

                // ------------------------------------------------

            case LowPass:
            case LowPass4: {
                defaultA(sinOmega / (2.0 * (Q * 6 + 0.4)));
                coeffs.b = {
                    (1.0 - cosOmega) / 2.0,
                    (1.0 - cosOmega),
                    (1.0 - cosOmega) / 2.0
                };
            } break;

                // ------------------------------------------------

            case HighPass:
            case HighPass4: {
                defaultA(sinOmega / (2.0 * (Q * 6 + 0.4)));
                coeffs.b = {
                    (1.0 + cosOmega) / 2.0,
                   -(1.0 + cosOmega),
                    (1.0 + cosOmega) / 2.0
                };
            } break;

                // ------------------------------------------------

            case BandPass: {
                defaultA(sinOmega * MathQuality::sinh((log10_2 / 2.0) * (1 / (Q * 4 + 0.2)) * (omega / sinOmega)));
                coeffs.b = {
                    sinOmega / 2.0,
                    0.0,
                   -sinOmega / 2.0
                };
            } break;

                // ------------------------------------------------

            case Notch: {
                defaultA(sinOmega * MathQuality::sinh((log10_2 / 2.0) * Q * (omega / sinOmega)));
                coeffs.b = {
                    1.0,
                   -2.0 * cosOmega,
                    1.0
                };
            } break;

                // ------------------------------------------------

            case AllPass: {
                double alpha = sinOmega / (2.0 * (Q * 4 + 0.2));
                defaultA(alpha);
                coeffs.b = {
                    1.0 - alpha,
                   -2.0 * cosOmega,
                    1.0 + alpha
                };
            } break;

                // ------------------------------------------------

            case PeakingEQ: {
                double A = MathQuality::pow(10, gain / 40.0);
                double alpha = sinOmega * MathQuality::sinh((log10_2 / 2.0) * (Q * 4 + 0.2) * (omega / sinOmega));
                coeffs.a = {
                    1.0 + alpha / A,
                   -2.0 * cosOmega,
                    1.0 - alpha / A
                };

                coeffs.b = {
                    1.0 + alpha * A,
                   -2.0 * cosOmega,
                    1.0 - alpha * A
                };
            } break;

                // ------------------------------------------------

            case LowShelf: {
                double A = MathQuality::pow(10, gain / 40.0);
                double t = MathQuality::max((A + 1.0 / A) * (1.0 / (Q * 3 + 0.5) - 1.0) + 2, 0.0);
                double alpha = (sinOmega / 2.0) * MathQuality::sqrt(t);
                double sqrtAa = MathQuality::sqrt(A) * alpha;
                coeffs.a = {
                           (A + 1.0) + (A - 1.0) * cosOmega + 2.0 * sqrtAa,
                   -2.0 * ((A - 1.0) + (A + 1.0) * cosOmega),
                           (A + 1.0) + (A - 1.0) * cosOmega - 2.0 * sqrtAa,
                };

                coeffs.b = {
                          A * ((A + 1.0) - (A - 1.0) * cosOmega + 2.0 * sqrtAa),
                    2.0 * A * ((A - 1.0) - (A + 1.0) * cosOmega),
                          A * ((A + 1.0) - (A - 1.0) * cosOmega - 2.0 * sqrtAa),
                };
            } break;

                // ------------------------------------------------

            case HighShelf: {
                double A = MathQuality::pow(10, gain / 40.0);
                double t = MathQuality::max((A + 1.0 / A) * (1.0 / (Q * 3 + 0.5) - 1.0) + 2, 0.0);
                double alpha = (sinOmega / 2.0) * MathQuality::sqrt(t);
                double sqrtAa = MathQuality::sqrt(A) * alpha;
                coeffs.a = {
                           (A + 1.0) - (A - 1.0) * cosOmega + 2.0 * sqrtAa,
                    2.0 * ((A - 1.0) - (A + 1.0) * cosOmega),
                           (A + 1.0) - (A - 1.0) * cosOmega - 2.0 * sqrtAa,
                };

                coeffs.b = {
                          A * ((A + 1.0) + (A - 1.0) * cosOmega + 2.0 * sqrtAa),
                   -2.0 * A * ((A - 1.0) + (A + 1.0) * cosOmega),
                          A * ((A + 1.0) + (A - 1.0) * cosOmega - 2.0 * sqrtAa),
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

    inline void ellipticIntegralK(double k, double& K, double& Kp) {
        constexpr int M = 4;

        K = std::numbers::pi_v<double> / 2;
        auto lastK = k;

        for (int i = 0; i < M; ++i) {
            lastK = std::pow(lastK / (1 + std::sqrt(1 - std::pow(lastK, 2.0))), 2.0);
            K *= 1 + lastK;
        }

        Kp = std::numbers::pi_v<double> / 2;
        auto last = std::sqrt(1 - k * k);

        for (int i = 0; i < M; ++i) {
            last = std::pow(last / (1.0 + std::sqrt(1.0 - std::pow(last, 2.0))), 2.0);
            Kp *= 1 + last;
        }
    }

    inline std::complex<double> asne(std::complex<double> w, double k) noexcept {
        constexpr int M = 4;

        double ke[M + 1];
        double* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1.0 + std::sqrt(1.0 - std::pow(*kei, 2.0))), 2.0);
            *++kei = next;
        }

        std::complex<double> last = w;

        for (int i = 1; i <= M; ++i)
            last = 2.0 * last / ((1.0 + ke[i]) * (1.0 + std::sqrt(1.0 - std::pow(ke[i - 1] * last, 2.0))));

        return 2.0 / std::numbers::pi_v<double> *std::asin(last);
    }

    inline std::complex<double> sne(std::complex<double> u, double k) noexcept {
        constexpr int M = 4;

        double ke[M + 1];
        double* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1 + std::sqrt(1 - std::pow(*kei, 2.0))), 2.0);
            *++kei = next;
        }

        // NB: the spurious cast to double here is a workaround for a very odd link-time failure
        std::complex<double> last = std::sin(u * (double)(std::numbers::pi_v<double> / 2));

        for (int i = M - 1; i >= 0; --i)
            last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

        return last;
    }

    inline std::complex<double> cde(std::complex<double> u, double k) noexcept {
        constexpr int M = 4;

        double ke[M + 1];
        double* kei = ke;
        *kei = k;

        for (int i = 0; i < M; ++i) {
            auto next = std::pow(*kei / (1.0 + std::sqrt(1.0 - std::pow(*kei, 2.0))), 2.0);
            *++kei = next;
        }

        // NB: the spurious cast to double here is a workaround for a very odd link-time failure
        std::complex<double> last = std::cos(u * (double)(std::numbers::pi_v<double> / 2));

        for (int i = M - 1; i >= 0; --i)
            last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

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

            auto normalisedFrequency = f0 / sampleRate;

            auto fp = normalisedFrequency - normalisedTransitionWidth / 2;
            assert(0.0 < fp && fp < 0.5);

            auto fs = normalisedFrequency + normalisedTransitionWidth / 2;
            assert(0.0 < fs && fs < 0.5);

            double Ap = passbandAmplitudedB;
            double As = stopbandAmplitudedB;
            auto Gp = Math::Fast::db_to_magnitude(Ap);
            auto Gs = Math::Fast::db_to_magnitude(As);
            auto epsp = std::sqrt(1.0 / (Gp * Gp) - 1.0);
            auto epss = std::sqrt(1.0 / (Gs * Gs) - 1.0);

            auto omegap = std::tan(std::numbers::pi * fp);
            auto omegas = std::tan(std::numbers::pi * fs);

            auto k = omegap / omegas;
            auto k1 = epsp / epss;

            int N;

            double K, Kp, K1, K1p;

            ellipticIntegralK(k, K, Kp);
            ellipticIntegralK(k1, K1, K1p);

            N = std::round(std::ceil((K1p * K) / (K1 * Kp)));

            const std::size_t r = N % 2;
            const std::size_t L = (N - r) / 2;
            const double H0 = std::pow(Gp, 1.0 - r);

            std::vector<std::complex<double>> pa, za;
            pa.reserve(L);
            za.reserve(L);
            std::complex<double> j(0, 1);

            auto v0 = -j * (asne(j / epsp, k1) / (double)N);

            if (r == 1) pa.push_back(omegap * j * sne(j * v0, k));

            for (std::size_t i = 1; i <= L; ++i) {
                auto ui = (2 * i - 1.0) / (double)N;
                auto zetai = cde(ui, k);

                pa.push_back(omegap * j * cde(ui - j * v0, k));
                za.push_back(omegap * j / (k * zetai));
            }

            std::vector<std::complex<double>> p, z, g;
            p.reserve(L + 1);
            z.reserve(L + 1);
            g.reserve(L + 1);

            if (r == 1) {
                p.push_back((1.0 + pa[0]) / (1.0 - pa[0]));
                g.push_back(0.5 * (1.0 - p[0]));
            }

            for (std::size_t i = 0; i < L; ++i) {
                p.push_back((1.0 + pa[i + r]) / (1.0 - pa[i + r]));
                z.push_back(za.size() == 0 ? -1.0 : (1.0 + za[i]) / (1.0 - za[i]));
                g.push_back((1.0 - p[i + r]) / (1.0 - z[i]));
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

    template<std::size_t N, class Sample = double, class MathQuality = Math, bool Quadruple = true>
    class StereoEqualizer : public std::array<Biquad<Sample, MathQuality, 1, Quadruple>, N>, public Module {
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

    template<std::size_t N, class Sample = double, class MathQuality = Math, std::size_t Parallel = 1, bool Quadruple = true>
    class BatchEqualizer : public std::array<Biquad<Sample, MathQuality, Parallel, Quadruple>, N>, public Module {
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
    template<class Sample, class MathQuality, std::size_t Parallel, bool Quadruple>
    class BatchEqualizer<0, Sample, MathQuality, Parallel, Quadruple> {
    public:
        template<class Type>
        constexpr Type processBatch(Type value, std::size_t, std::size_t = 0) const noexcept { return value; }

        constexpr void finalizeBatches() {}
        constexpr void reset() {}
    };
}
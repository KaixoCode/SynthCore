#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    static inline std::string format_impl(ParamValue v, std::int64_t decimals, std::string_view unit) {
        if (decimals < 0) return std::string{ unit };
        return std::format("{:.{}f}", v, decimals).append(unit);
    }

    // ------------------------------------------------

    struct FormatRange {
        ParamValue start = 0;
        ParamValue end = 1; // non-inclusive

        // ------------------------------------------------

        std::int64_t decimals = 2;
        double divide = 1;
        std::string_view unit{};

        // ------------------------------------------------

        bool within(ParamValue v) const { return v >= start && v < end; }

        // ------------------------------------------------

        std::string format(ParamValue v) const { return format_impl(v / divide, decimals, unit); }
    };

    // ------------------------------------------------

    template<std::size_t N>
    struct FormatterDefinition {
        std::int64_t decimals = 2;
        double divide = 1;
        std::string_view unit{};

        // ------------------------------------------------

        std::array<FormatRange, N> ranges{};

        // ------------------------------------------------

        std::string format(ParamValue v) const {
            for (auto& i : ranges) if (i.within(v)) return i.format(v);
            return format_impl(v / divide, decimals, unit);
        }

        ParamValue parse(std::string_view str) const {
            // TODO: proper implementation
            ParamValue value = 0;
            std::from_chars(str.data(), str.data() + str.size(), value);
            return value;
        }
    };

    // ------------------------------------------------

    class Formatter {
    public:
        using FormatCallback = std::string(*)(ParamValue);
        using ParseCallback = ParamValue(*)(std::string_view);

        // ------------------------------------------------

        constexpr static FormatCallback DefaultFormat = +[](ParamValue value) { return std::to_string(value); };
        constexpr static ParseCallback DefaultParse = +[](std::string_view str) { 
            ParamValue value = 0;
            std::from_chars(str.data(), str.data() + str.size(), value);
            return value;
        };

        // ------------------------------------------------

        constexpr Formatter(FormatCallback format = DefaultFormat, ParseCallback parse = DefaultParse) noexcept
            : m_Format(format), m_Parse(parse) {}

        // ------------------------------------------------

        constexpr std::string format(ParamValue value) const noexcept { return m_Format(value); }
        constexpr ParamValue parse(std::string_view value) const noexcept { return m_Parse(value); }

        // ------------------------------------------------

    private:
        FormatCallback m_Format;
        ParseCallback m_Parse;
    };

    // ------------------------------------------------

    namespace Formatters {
        constexpr FormatterDefinition<0> DefaultImpl{ .decimals = 2, .unit = "" };
        constexpr Formatter Default{
            [](ParamValue value) -> std::string { return DefaultImpl.format(value); },
            [](std::string_view str) -> ParamValue { return DefaultImpl.parse(str); },
        };

        constexpr FormatterDefinition<0> IntegerImpl{ .decimals = 0, .unit = "" };
        constexpr Formatter Integer{
            [](ParamValue value) -> std::string { return IntegerImpl.format(value); },
            [](std::string_view str) -> ParamValue { return IntegerImpl.parse(str); },
        };

        constexpr FormatterDefinition<2> DecibelsImpl{ .decimals = 0, .unit = " dB", .ranges{
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = " dB" },
            FormatRange{.start = -100, .end = 100, .decimals = 1, .unit = " dB" },
        } };
        constexpr Formatter Decibels{
            [](ParamValue value) -> std::string { return DecibelsImpl.format(value); },
            [](std::string_view str) -> ParamValue { return DecibelsImpl.parse(str); },
        };

        constexpr FormatterDefinition<2> PitchImpl{ .decimals = 0, .unit = " st", .ranges{
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = " st" },
            FormatRange{.start = -100, .end = 100, .decimals = 1, .unit = " st" },
        } };
        constexpr Formatter Pitch{
            [](ParamValue value) -> std::string { return PitchImpl.format(value); },
            [](std::string_view str) -> ParamValue { return PitchImpl.parse(str); },
        };

        constexpr FormatterDefinition<3> DetuneImpl{ .decimals = 0, .unit = " ct", .ranges{
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = " ct" },
            FormatRange{.start = -100, .end = 100, .decimals = 1, .unit = " ct" },
            FormatRange{.start = -1000, .end = 1000, .decimals = 0, .unit = " ct" },
        } };
        constexpr Formatter Detune{
            [](ParamValue value) -> std::string { return DetuneImpl.format(value); },
            [](std::string_view str) -> ParamValue { return DetuneImpl.parse(str); },
        };

        constexpr FormatterDefinition<0> TransposeImpl{ .decimals = 0, .unit = " st" };
        constexpr Formatter Transpose{
            [](ParamValue value) -> std::string { return TransposeImpl.format(value); },
            [](std::string_view str) -> ParamValue { return TransposeImpl.parse(str); },
        };

        constexpr FormatterDefinition<3> PercentImpl{ .decimals = 0, .unit = " %", .ranges{
            FormatRange{.start = -0.995, .end = 1, .decimals = 2, .unit = " %" },
            FormatRange{.start = -10, .end = 10, .decimals = 1, .unit = " %" },
            FormatRange{.start = -99.95, .end = 100, .decimals = 1, .unit = " %" }
        } };
        constexpr Formatter Percent{
            [](ParamValue value) -> std::string { return PercentImpl.format(value); },
            [](std::string_view str) -> ParamValue { return PercentImpl.parse(str); },
        };

        constexpr FormatterDefinition<3> TimesImpl{ .decimals = 0, .unit = "x", .ranges{
            FormatRange{.start = -100, .end = -10, .decimals = 1, .unit = "x" },
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = "x" },
            FormatRange{.start = 10, .end = 100, .decimals = 1, .unit = "x" },
        } };
        constexpr Formatter Times{
            [](ParamValue value) -> std::string { return TimesImpl.format(value); },
            [](std::string_view str) -> ParamValue { return TimesImpl.parse(str); },
        };

        constexpr FormatterDefinition<4> TimeImpl{ .decimals = 1, .divide = 1000, .unit = " s", .ranges{
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = " ms" },
            FormatRange{.start = -100, .end = 100, .decimals = 1, .unit = " ms" },
            FormatRange{.start = -1000, .end = 1000, .decimals = 0, .unit = " ms" },
            FormatRange{.start = -10000, .end = 10000, .decimals = 2, .divide = 1000, .unit = " s" },
        } };
        constexpr Formatter Time{
            [](ParamValue value) -> std::string { return TimeImpl.format(value); },
            [](std::string_view str) -> ParamValue { return TimeImpl.parse(str); },
        };

        constexpr FormatterDefinition<4> FrequencyImpl{ .decimals = 1, .divide = 1000, .unit = " kHz", .ranges{
            FormatRange{.start = -10, .end = 10, .decimals = 2, .unit = " Hz" },
            FormatRange{.start = -100, .end = 100, .decimals = 1, .unit = " Hz" },
            FormatRange{.start = -1000, .end = 1000, .decimals = 0, .unit = " Hz" },
            FormatRange{.start = -10000, .end = 10000, .decimals = 2, .divide = 1000, .unit = " kHz" },
        } };
        constexpr Formatter Frequency{
            [](ParamValue value) -> std::string { return FrequencyImpl.format(value); },
            [](std::string_view str) -> ParamValue { return FrequencyImpl.parse(str); },
        };

        constexpr FormatterDefinition<3> PanImpl{ .decimals = -1, .unit = "C", .ranges{
            FormatRange{.start = -100, .end = -0.01, .decimals = 1, .divide = -1, .unit = " L" },
            FormatRange{.start = -0.01, .end = 0.01, .decimals = -1, .unit = "C" },
            FormatRange{.start = 0.01, .end = 100, .decimals = 1, .unit = " R" },
        } };
        constexpr Formatter Pan{
            [](ParamValue value) -> std::string { return PanImpl.format(value); },
            [](std::string_view str) -> ParamValue { return PanImpl.parse(str); },
        };

        template<string_literal ...Names>
        constexpr FormatterDefinition<sizeof...(Names)> GroupImpl{ .ranges = []() {
            float index = 0;
            return std::array<FormatRange, sizeof...(Names)>{
                FormatRange{.start = index, .end = (index += 1), .decimals = -1, .unit = Names }...
            };
        }() };
        template<string_literal ...Names>
        constexpr Formatter Group{
            [](ParamValue value) -> std::string { return GroupImpl<Names...>.format(value); },
            [](std::string_view str) -> ParamValue { return Default.parse(str); },
        };

        template<string_literal ...Names>
        constexpr Formatter GroupMix{
            [](ParamValue v) {
                constexpr std::array<std::string_view, sizeof...(Names)> names{ Names... };

                std::size_t group1 = v;
                std::size_t group2 = v + 1;
                std::size_t mix1 = 100 * (1 - (v - group1));
                std::size_t mix2 = 100 - mix1;

                if (v == 0) return std::string{ names.front() };
                if (group2 > sizeof...(Names) - 1) return std::string{ names.back() };
                if (mix2 == 100) return std::string{ names[group2] };

                return std::string{ names[group1] } + " " + std::to_string(mix1) + " % - "
                     + std::string{ names[group2] } + " " + std::to_string(mix2) + " %";
            },
            [](std::string_view str) -> ParamValue { return Default.parse(str); },
        };
    }

    // ------------------------------------------------

    class Transform {
    public:
        using Callback = ParamValue(*)(ParamValue);
        constexpr static Callback Default = +[](ParamValue value) { return value; };

        constexpr ParamValue transform(ParamValue value) const noexcept { return m_Denormalize(value); }
        constexpr ParamValue normalize(ParamValue value) const noexcept { return m_Normalize(value); }

        constexpr Transform(Callback denorm = Default, Callback norm = Default) noexcept
            : m_Denormalize(denorm), m_Normalize(norm){}

    private:
        Callback m_Denormalize;
        Callback m_Normalize;
    };

    // ------------------------------------------------

    namespace Transformers {
        constexpr Transform Default{
            [](ParamValue v) -> ParamValue { return v; },
            [](ParamValue v) -> ParamValue { return v; },
        };

        template<ParamValue Start, ParamValue End>
        constexpr Transform Range{
            [](ParamValue v) -> ParamValue { return v * (End - Start) + Start; },
            [](ParamValue v) -> ParamValue { return (v - Start) / (End - Start); },
        };

        template<ParamValue Start, ParamValue End, auto Pow>
        constexpr Transform Power{
            [](ParamValue v) -> ParamValue { return Math::pow(v, Pow) * (End - Start) + Start; },
            [](ParamValue v) -> ParamValue { return Math::pow((v - Start) / (End - Start), 1. / Pow); },
        };

        template<ParamValue Start, ParamValue End, auto Pow>
        constexpr Transform InvPower = {
            [](ParamValue v) -> ParamValue { return (1 - Math::pow(1 - v, Pow)) * (End - Start) + Start; },
            [](ParamValue v) -> ParamValue { return 1 - Math::pow(1 - (v - Start) / (End - Start), 1. / Pow); },
        };

        template<ParamValue Start, ParamValue End>
        constexpr Transform Decibel{
            [](ParamValue v) -> ParamValue { return Math::magnitude_to_db(v * (End - Start) + Start); },
            [](ParamValue v) -> ParamValue { return (Math::db_to_magnitude(v) - Start) / (End - Start); },
        };

        template<std::size_t Elements>
        constexpr Transform Group{
            [](ParamValue v) -> ParamValue { return normalToIndex(v, Elements); },
            [](ParamValue v) -> ParamValue { return v / (Elements - 1); },
        };

        template<std::size_t Start, std::size_t Size>
        constexpr Transform Integer{
            [](ParamValue v) -> ParamValue { return normalToIndex(v, Size) + Start; },
            [](ParamValue v) -> ParamValue { return (v - Start) / (Size - 1); }
        };

        template<std::int64_t Start, std::int64_t End>
        constexpr Transform Transpose{
            [](ParamValue v) -> ParamValue { return Math::floor(v * (End - Start)) + Start; },
            [](ParamValue v) -> ParamValue { return (v - Start) / (End - Start); },
        };

        template<ParamValue F1, ParamValue F2>
        constexpr Transform Log{
            [](ParamValue v) -> ParamValue { return Math::magnitude_to_log<F1, F2>(v); },
            [](ParamValue v) -> ParamValue { return Math::log_to_magnitude<F1, F2>(v); },
        };
    }
}
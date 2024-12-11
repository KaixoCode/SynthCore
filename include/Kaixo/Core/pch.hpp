
// ------------------------------------------------

#ifdef __cplusplus 
// ^^ For some reason updating to new JUCE tries to compile this PCH 
//    as C somehow somewhere. My guess is JUCE added some C code and
//    since this is a PCH file it tries to include it there as well?
//    So just exclude the contents if this isn't C++, because it
//    fails to build.

// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include <algorithm>
#include <any>
#include <array>
#include <bitset>
#include <cassert>
#include <charconv>
#include <cmath>
#include <codecvt>
#include <complex>
#include <condition_variable>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numbers>
#include <numeric>
#include <random>
#include <ranges>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <valarray>
#include <vector>

// ------------------------------------------------

namespace Kaixo {

    // ------------------------------------------------

    enum class VersionType {
        Release = 0x13143,
        Demo = 0x46363,
        Snapshot = 0x69626,
        Unknown = Demo, // Default to Demo
    };

    constexpr std::string_view versionTypeString = SYNTH_VersionType;
    constexpr VersionType versionType = versionTypeString == "RELEASE" ? VersionType::Release
        : versionTypeString == "SNAPSHOT" ? VersionType::Snapshot
        : versionTypeString == "DEMO" ? VersionType::Demo
        : VersionType::Unknown;

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
    using NoteID = std::uint64_t;

    constexpr NoteID NoNoteID = static_cast<NoteID>(-1);

    // ------------------------------------------------

}

// ------------------------------------------------

#include "basic_simd.hpp"
#include "basic_json.hpp"

#include "Kaixo/Utils/utils.hpp"
#include "Kaixo/Utils/string_literal.hpp"
#include "Kaixo/Utils/thread_pool.hpp"
#include "Kaixo/Utils/Containers.hpp"
#include "Kaixo/Utils/Random.hpp"
#include "Kaixo/Utils/Timer.hpp"
#include "Kaixo/Utils/utils.hpp"
#include "Kaixo/Utils/Math.hpp"
#include "Kaixo/Utils/StringUtils.hpp"

#include "Kaixo/Core/Processing/Stereo.hpp"

// ------------------------------------------------

#endif

// ------------------------------------------------

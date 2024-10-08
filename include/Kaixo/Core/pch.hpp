
// ------------------------------------------------

#pragma once

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
#include <expected>

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

#include "Kaixo/Utils/json.hpp"
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

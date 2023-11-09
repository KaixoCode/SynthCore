#pragma once
#include <chrono>

namespace Kaixo {

	// ------------------------------------------------

	template<class Clock = std::chrono::steady_clock>
	struct Timer {
		using clock = Clock;

		// ------------------------------------------------

		clock::time_point start = clock::now();

		// ------------------------------------------------

		constexpr Timer() = default;

		// ------------------------------------------------

		template<
			class Unit = std::chrono::seconds,
			class Granularity = std::chrono::nanoseconds>
		constexpr float time() const noexcept(noexcept(1. * std::chrono::duration_cast<Granularity>(clock::now() - start).count())) {
			using unit = Unit;
			using granularity = Granularity;
			constexpr float ratio = 1. / std::chrono::duration_cast<granularity>(unit::duration(1)).count();
			return ratio * std::chrono::duration_cast<granularity>(clock::now() - start).count();
		}
	};
}
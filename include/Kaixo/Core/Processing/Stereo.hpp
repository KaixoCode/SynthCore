#pragma once

namespace Kaixo::Processing {

	// ------------------------------------------------
	
	using Sample = double;

	// ------------------------------------------------

	struct alignas(8) Stereo {

		// ------------------------------------------------

		Sample l = 0;
		Sample r = 0;

		// ------------------------------------------------

		constexpr Stereo& operator=(Sample other) { l = other, r = other; return *this; }
		constexpr Stereo& operator+=(const Stereo& other) { l += other.l, r += other.r; return *this; }
		constexpr Stereo& operator-=(const Stereo& other) { l -= other.l, r -= other.r; return *this; }
		constexpr Stereo& operator*=(const Stereo& other) { l *= other.l, r *= other.r; return *this; }
		constexpr Stereo& operator/=(const Stereo& other) { l /= other.l, r /= other.r; return *this; }
		constexpr Stereo& operator+=(Sample other) { l += other, r += other; return *this; }
		constexpr Stereo& operator-=(Sample other) { l -= other, r -= other; return *this; }
		constexpr Stereo& operator*=(Sample other) { l *= other, r *= other; return *this; }
		constexpr Stereo& operator/=(Sample other) { l /= other, r /= other; return *this; }
		constexpr friend Stereo operator-(const Stereo& a) { return { -a.l, -a.r }; }
		constexpr friend Stereo operator+(const Stereo& a, const Stereo& b) { return { a.l + b.l, a.r + b.r }; }
		constexpr friend Stereo operator-(const Stereo& a, const Stereo& b) { return { a.l - b.l, a.r - b.r }; }
		constexpr friend Stereo operator*(const Stereo& a, const Stereo& b) { return { a.l * b.l, a.r * b.r }; }
		constexpr friend Stereo operator/(const Stereo& a, const Stereo& b) { return { a.l / b.l, a.r / b.r }; }
		constexpr friend Stereo operator+(const Stereo& a, Sample b) { return { a.l + b, a.r + b }; }
		constexpr friend Stereo operator-(const Stereo& a, Sample b) { return { a.l - b, a.r - b }; }
		constexpr friend Stereo operator*(const Stereo& a, Sample b) { return { a.l * b, a.r * b }; }
		constexpr friend Stereo operator/(const Stereo& a, Sample b) { return { a.l / b, a.r / b }; }
		constexpr friend Stereo operator+(Sample a, const Stereo& b) { return { a + b.l, a + b.r }; }
		constexpr friend Stereo operator-(Sample a, const Stereo& b) { return { a - b.l, a - b.r }; }
		constexpr friend Stereo operator*(Sample a, const Stereo& b) { return { a * b.l, a * b.r }; }
		constexpr friend Stereo operator/(Sample a, const Stereo& b) { return { a / b.l, a / b.r }; }
		constexpr friend bool operator==(const Stereo& a, const Stereo& b) { return a.l == b.l && a.r == b.r; }
		constexpr friend bool operator!=(const Stereo& a, const Stereo& b) { return a.l != b.l && a.r != b.r; }
		constexpr friend bool operator==(const Stereo& a, Sample b) { return a.l == b && a.r == b; }
		constexpr friend bool operator!=(const Stereo& a, Sample b) { return a.l != b && a.r != b; }
		constexpr friend bool operator==(Sample a, const Stereo& b) { return a == b.l && a == b.r; }
		constexpr friend bool operator!=(Sample a, const Stereo& b) { return a != b.l && a != b.r; }
		constexpr Sample& operator[](bool value) { return value ? r : l; }
		constexpr const Sample& operator[](bool value) const { return value ? r : l; }
		constexpr Sample sum() const noexcept { return r + l; }
		constexpr Sample average() const noexcept { return sum() / 2; }
	};
}

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	template<class Ty> concept is_stereo = std::same_as<Ty, Processing::Stereo>;
}
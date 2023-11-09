#pragma once

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	struct Color {

		// ------------------------------------------------

		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;

		// ------------------------------------------------

		operator juce::Colour() const { return juce::Colour::fromRGBA(r, g, b, a); }

		// ------------------------------------------------

		template<class ...Args>
			requires (std::same_as<std::decay_t<Args>, Color> && ...)
		constexpr static Color lerp(float l, Args&& ...args) {
			constexpr std::size_t count = sizeof...(Args);
			const float progress = Math::Fast::clamp((count - 1) * l, 0, count - 1);
			Color result{ 0, 0, 0, 0 };
			std::size_t index = 0;
			float ratio = 0;
			((ratio = 1 - Math::Fast::min(1, Math::Fast::abs(progress - index)),
				result.r += ratio * args.r,
				result.g += ratio * args.g,
				result.b += ratio * args.b,
				result.a += ratio * args.a,
				++index
				), ...);

			return result;
		}

		// ------------------------------------------------

	};

	// ------------------------------------------------

}
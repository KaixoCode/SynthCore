#pragma once
#include <random>

namespace Kaixo {

	struct Random {
		std::uniform_real_distribution<float> distribution{ 0, 1 };
		std::random_device device;
		std::default_random_engine engine{ device() };

		float next() { return distribution(engine); }
	};
}
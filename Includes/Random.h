#pragma once

#include <random>
#include <chrono>

int inline GetIntValue(int min, int max)
{
	std::mt19937_64 generator;
	generator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(min, max);

	return distribution(generator);
}

double inline GetDoubleValue(double min, double max)
{
	std::mt19937_64 generator;
	generator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<double> distribution(min, max);

	return distribution(generator);
}

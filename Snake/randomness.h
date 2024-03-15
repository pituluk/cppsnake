#include <concepts>
#include <limits>
#include <random>
#pragma once
#undef min //Windows.h screwing up
#undef max
template <std::integral T>
T randomInt(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	thread_local std::default_random_engine gen{ std::random_device()() };
	std::uniform_int_distribution<T> dist(min, max);
	T result = dist(gen);
	return result;
}
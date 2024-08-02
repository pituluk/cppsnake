#include <concepts>
#include <limits>
#include <random>
#pragma once
#ifndef NOMINMAX
#define randomnessminmax 1
#pragma push_macro("min")
#pragma push_macro("max")
#undef min 
#undef max
#endif
template <std::integral T>
T randomInt(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
	thread_local std::default_random_engine gen{ std::random_device()() };
	std::uniform_int_distribution<T> dist(min, max);
	T result = dist(gen);
	return result;
}
#ifdef randomnessminmax
#pragma pop_macro("max")
#pragma pop_macro("min")
#endif
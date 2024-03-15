#include <span>
#include <cstdint>
#pragma once
class mdwrap // class to wrap 2d indexing onto a 1d array made specifically for this game so it has almost no functionality
{
public:
	mdwrap(std::vector<char>& a, std::uint16_t x) : a(a), w(x) {}
	//no iterators because std::mdspan doesnt have them either,
	std::span<char> operator[](std::size_t y)
	{
		std::size_t offset = y * w;
		return std::span<char>(a.begin() + offset, w);
	}

	std::vector<char>& a;
	std::uint16_t w;
};
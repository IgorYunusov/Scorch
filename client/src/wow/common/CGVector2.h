#pragma once
#include <cmath>
#include <string>

struct Vector2 {
	float_t x;
	float_t y;

	std::string toString() const { return std::string("Vector2 (" + std::to_string(x) + " " + std::to_string(y) + ")"); }
};
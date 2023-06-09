#pragma once
#include <cmath>
#include <string>

#pragma pack(push, 1)
struct CGVector3
{
	CGVector3() : x(0), y(0), z(0) {}
	CGVector3(float_t x, float_t y, float_t z) : x(x), y(y), z(z) {}

	std::string toString() const { return std::string("Vector3 (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + " )"); }
	inline CGVector3 operator- (const CGVector3& v) const { return CGVector3(x - v.x, y - v.y, z - v.z); }
	inline float_t squaredLength() const { return x * x + y * y + z * z; }

	float_t distanceTo(CGVector3* other);
	float_t angleBetween(CGVector3* other);

	float_t x;
	float_t y;
	float_t z;
};
#pragma pack(pop)
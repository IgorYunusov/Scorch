#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct CGGuid
{
	uint64_t high;
	uint64_t low;

	bool isEmpty() { return high == 0 && low == 0; }

	bool operator==(const CGGuid& rhs) const {
		return (this->high == rhs.high && this->low == rhs.low);
	}

	std::string toString() const { return std::string("Guid (" + std::to_string(high) + " " + std::to_string(low) + ")"); }
};
#pragma pack(pop)

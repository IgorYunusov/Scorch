#pragma once
#include "Navigator.h"

namespace Navigation
{
	void Release();
	bool CalculatePath(uint32_t mapId, Vector3* start, Vector3* destination);
	std::vector<Vector3> const& ActivePath();
}
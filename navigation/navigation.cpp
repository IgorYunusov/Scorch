#include "Navigation.h"

namespace
{
	Navigator* activeNavigator = nullptr;
}

void Navigation::Release() {
	if (activeNavigator != nullptr)
	{
		delete activeNavigator;
		activeNavigator = nullptr;
	}
}

bool Navigation::CalculatePath(uint32_t mapId, Vector3* start, Vector3* destination)
{
	// first load
	if (activeNavigator == nullptr)
	{
		activeNavigator = new Navigator();
		if (!activeNavigator->initialize(mapId)) {
			delete activeNavigator;
			activeNavigator = nullptr;
			return false;
		}
	}
	// have other map, only ever want one loaded
	else if (activeNavigator->getMapId() != mapId)
	{
		delete activeNavigator;
		activeNavigator = nullptr;

		activeNavigator = new Navigator();
		if (!activeNavigator->initialize(mapId)) {
			delete activeNavigator;
			activeNavigator = nullptr;
			return false;
		}
	}

	return activeNavigator->calculatePath(start, destination);
}

std::vector<Vector3> const& Navigation::ActivePath()
{
	if (activeNavigator != nullptr)
		return activeNavigator->getGenerator()->GetPath();
	else
		return std::vector<Vector3>();
}
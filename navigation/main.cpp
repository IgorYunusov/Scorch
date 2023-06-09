#include <iostream>

#include "Navigation.h"

int main()
{
	uint32_t mapId = 530;
	//Vector3 start = Vector3{ 8850.49f, -6091.11f, -1.52f };
	//Vector3 end = Vector3{ 8805.98f, -6099.19f, 4.72f };

	Vector3 start = Vector3{ -1836.38, 5310.72, -12.43 };
	Vector3 end = Vector3{ -2621.22, 4390.34, 34.39 };

	// -1836.38, 5310.72, -12.43
	// -2621.22, 4390.34, 34.39

	if (Navigation::CalculatePath(mapId, &start, &end))
	{
		printf("\tSuccessful Calculation\n");
	}

	printf("Steps: %u\n", Navigation::ActivePath().size());
	for (auto& step : Navigation::ActivePath())
		printf("\tStep: %s\n", step.toString().c_str());

	Navigation::Release();
	std::cin.get();
	return 0;
}
 
// path CalculatePath(uint32_t mapId, Vector3* source, Vector3* destination)
// void Release()

#pragma once
#include "wow/Offsets.h"
#include "wow/common/CGVector3.h"

#pragma pack(push, 1)
struct CGWorldFrame
{
	char padding_0x198[0x198];
	float_t fovY;
	float_t fovX;

	bool worldToScreen(CGVector3* input, CGVector3* output);
};
#pragma pack(pop)

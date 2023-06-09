#include "pch.h"
#include "wow/core/CGWorldFrame.h"
#include "wow/Offsets.h"

auto CGWorldFrame_GetScreenCoordinates = reinterpret_cast<bool(__fastcall*)(CGWorldFrame * self, CGVector3 * input, CGVector3 * output, bool unk)>(BaseAddress() + Offsets::CGWorldFrame_GetScreenCoordinates);

bool CGWorldFrame::worldToScreen(CGVector3* input, CGVector3* output)
{
	if (!CGWorldFrame_GetScreenCoordinates(this, input, output, false))
		return false;

	output->x = (output->x / this->fovX) * 1280.0f;
	output->y = 720.0f - (720.0f * (output->y / this->fovY));
	return true;
}

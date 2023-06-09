#include "pch.h"
#include "Drawing.h"

#include "wow/core/CGCore.h"

bool Drawing::DrawPoint(CGVector3* input, float_t radius)
{
	static ImColor pointColor(0.f, 2.f, 1.f, 1.f);
	static CGVector3 screenPosition = CGVector3{ 0, 0, 0 };

	if (!CGCore::WorldFrame()->worldToScreen(input, &screenPosition))
		return false;

	ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(screenPosition.x, screenPosition.y), radius, pointColor);
	return true;
}

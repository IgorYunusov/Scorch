#include "pch.h"
#include "wow/core/CGCore.h"
#include "wow/Offsets.h"

static auto ClickToMove_MoveTo = reinterpret_cast<void(__fastcall*)(CGActivePlayer*, CGVector3)>(BaseAddress() + Offsets::MoveTo);
static auto ClickToMove_FaceTo = reinterpret_cast<void(__fastcall*)(CGActivePlayer*, float_t)>(BaseAddress() + Offsets::FaceTo);

void CGCore::MoveTo(CGVector3* destination) {
	ClickToMove_MoveTo(Player(), *destination);
}

void CGCore::FaceTo(float_t angle) {
	ClickToMove_FaceTo(Player(), angle);
}

CGGuidList<CGContainer> CGCore::EquippedBags()
{
	return CGGuidList<CGContainer> { EquippedBagGuids(), 4 };
}

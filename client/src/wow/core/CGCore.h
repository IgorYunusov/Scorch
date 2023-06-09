#pragma once
#include "wow/ui/CGFrame.h"
#include "wow/core/CGWorldFrame.h"
#include "wow/core/CGObjectManager.h"
#include "wow/entity/CGContainer.h"
#include "wow/entity/CGActivePlayer.h"

namespace CGCore {

	inline bool InGame() { return (*reinterpret_cast<int8_t*>(BaseAddress() + Offsets::InGame) >> 2) & 1; }
	inline uint32_t MapId() { return reinterpret_cast<uint32_t(__fastcall*)()>(BaseAddress() + Offsets::GetMapId)(); }

	inline uint32_t GameTime() { return reinterpret_cast<uint32_t(__fastcall*)()>(BaseAddress() + Offsets::GameTime)(); }
	inline uint32_t LastEventTime() { return *reinterpret_cast<int32_t*>(BaseAddress() + Offsets::LastEventTime); }
	inline void SetEventTime() { *reinterpret_cast<int32_t*>(BaseAddress() + Offsets::LastEventTime) = GameTime(); }

	inline CGFrameContainer* FrameContainer() { return *reinterpret_cast<CGFrameContainer**>(BaseAddress() + Offsets::FrameContainer); }
	inline CGObjectManager* ObjectManager() { return *reinterpret_cast<CGObjectManager**>(BaseAddress() + Offsets::ObjectManager); }
	inline CGWorldFrame* WorldFrame() { return *reinterpret_cast<CGWorldFrame**>(BaseAddress() + Offsets::WorldFrame); }

	inline CGGuid* PlayerGuid() { return reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::PlayerGuid); }
	inline CGGuid* TargetGuid() { return reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::TargetGuid); }
	inline CGGuid* PetGuid() { return reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::PetGuid); }
	inline CGGuid* MouseOverGuid() { return reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::MouseOverGuid); }
	inline CGGuid* EquippedBagGuids() { return reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::EquippedBagGuids); }

	inline CGActivePlayer* Player() { return reinterpret_cast<CGActivePlayer*>(ObjectManager()->getByGuid(PlayerGuid())); }
	inline CGUnit* Target() { return reinterpret_cast<CGUnit*>(ObjectManager()->getByGuid(TargetGuid())); }
	inline CGUnit* Pet() { return reinterpret_cast<CGUnit*>(ObjectManager()->getByGuid(PetGuid())); }
	inline CGObject* MouseOver() { return reinterpret_cast<CGObject*>(ObjectManager()->getByGuid(MouseOverGuid())); }

	CGGuidList<CGContainer> EquippedBags();

	void MoveTo(CGVector3* targetPosition);
	void FaceTo(float_t targetAngle);
}



#pragma once
#include "wow/entity/CGObject.h"
#include <cstdint>
#include <cmath>

#pragma pack(push, 1)
struct CGGameObjectDescriptor : public CGObjectDescriptor
{
	// sizeof(CGObjectData) = 0x001C
	CGGuid createdBy;
	CGGuid guildGuid;
	uint32_t displayedId;
	uint32_t flags;
	float_t parentRotation;
	uint32_t factionTemplate;
	uint32_t level;
	uint8_t state;
	uint8_t typeId;
	uint16_t percentHealth;
	uint32_t spellVisualId;
	uint32_t stateSpellVisualId;
	uint32_t spawnTrackingStateAnimId;
	uint32_t spawnTrackingStateAnimKitId;
};

struct CGGameObject : public CGObject
{
	CGGameObjectDescriptor* getDescriptor() { return reinterpret_cast<CGGameObjectDescriptor*>(this->descriptor); }

	bool createdBy(CGObject* creator) {
		CGGuid* ownerGuid = &creator->guid;
		CGGuid* guid = &this->getDescriptor()->createdBy;
		return ownerGuid->high == guid->high && ownerGuid->low == guid->low;
	}

	bool isAnimating()
	{
		return *reinterpret_cast<bool*>((uintptr_t)this + Offsets::CGGameObject_AnimationStatus);
	}
};
#pragma pack(pop)
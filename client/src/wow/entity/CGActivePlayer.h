#pragma once
#include "wow/entity/CGPlayer.h"

enum EquipmentSlot : uint32_t
{
    HEAD,
    NECK,
    SHOULDER,
    SHIRT,
    CHEST,
    BELT,
    LEGS,
    FEET,
    WRIST,
    GLOVES,
    FINGER1,
    FINGER2,
    TRINKET1,
    TRINKET2,
    CLOAK,
    MAINHAND,
    OFFHAND,
    RANGED,
    TABARD,
};

#pragma pack(push, 1)
struct CGActivePlayerDescriptor : public CGPlayerDescriptor
{
	CGGuid invSlots[129];
};

struct CGActivePlayer : public CGPlayer
{
	CGGuidList<CGItem> getEquipment() { return CGGuidList<CGItem> { this->getDescriptor()->invSlots, 18, 0 }; }
	CGGuidList<CGItem> getInventory() { return CGGuidList<CGItem> { this->getDescriptor()->invSlots, 16, 23 }; }
	CGActivePlayerDescriptor* getDescriptor() { return reinterpret_cast<CGActivePlayerDescriptor*>(this->descriptor); }
};
#pragma pack(pop)
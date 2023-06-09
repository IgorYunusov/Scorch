#pragma once
#include "wow/entity/CGItem.h"

#pragma pack(push, 1)
struct CGContainerDescriptor : public CGItemDescriptor
{
	// sizeof(CGItemData) = 0x0140
	CGGuid slots[36]; // 0x140 - 0x0380
	uint32_t numSlots; // 0x0380 - 0x0384
};

struct CGContainer : public CGItem
{
	CGContainerDescriptor* getDescriptor() { 
		return reinterpret_cast<CGContainerDescriptor*>(this->descriptor);
	}

	CGGuidList<CGItem> getItems() { 
		auto descriptor = this->getDescriptor();
		return CGGuidList<CGItem> { descriptor->slots, descriptor->numSlots };
	}
};
#pragma pack(pop)
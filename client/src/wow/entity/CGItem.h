#pragma once
#include "wow/entity/CGObject.h"

enum class ItemQuality : uint8_t
{
	Poor = 0x00,
	Common = 0x01,
	Uncommon = 0x02,
	Rare = 0x03,
	Epic = 0x04,
	Legendary = 0x05,
	Artifact = 0x06,
	Heirloom = 0x07,
	WowToken = 0x08,
	Unknown = 0x09
};

enum class BagFamily : uint32_t
{
	Unspecified = 0,
	Quiver = 1,
	AmmoPouch = 2,
	SoulBag = 4,
	LeatherworkingBag = 8,
	InscriptionBag = 16,
	HerbBag = 32,
	EnchantingBag = 64,
	EngineeringBag = 128,
	Keyring = 256,
	GemBag = 512,
	MiningBag = 1024,
	Unknown = 2048,
	VanityPets = 4096
};

#pragma pack(push, 1)
struct CGItemDescriptor : public CGObjectDescriptor
{
	// sizeof(CGObjectData) = 0x001C
	CGGuid owner; // 0x01C - 0x02C
	CGGuid containedIn; // 0x02C - 0x03C
	CGGuid creator; // 0x03C - 0x04C
	CGGuid giftCreator; // 0x04C - 0x05C
	uint32_t stackCount; // 0x5C - 0x60
	uint32_t expiration; // 0x60 - 0x64
	char padding_0x120[0xBC]; // 0x64 - 0x120
	uint32_t durability; // 0x120 - 0x124
	uint32_t maxDurability; // 0x124 - 0x128
	uint32_t createPlayedTime; // 0x0128 - 0x012C
	uint32_t modifiersMask; // 0x012C - 0x0130
	uint32_t context; // 0x0130 - 0x0134
	uint64_t artifactXP; // 0x0134 - 0x013C
	uint32_t itemAppearanceModID; // 0x013C - 0x0140
};

struct ItemRecord
{
	char padding_0x064[0x064];
	BagFamily bagFamily; // 0x064 - 0x068

	char padding_0x160[0xF8];
	ItemQuality quality;
};

struct CGItem : public CGObject
{
	CGItemDescriptor* getDescriptor() { return reinterpret_cast<CGItemDescriptor*>(this->descriptor); }
	
	void useOn(CGGuid* targetGuid) 
	{
		CGGuid* unk = reinterpret_cast<CGGuid*>(BaseAddress() + Offsets::UseItemParam);
		reinterpret_cast<bool(__fastcall*)(CGItem*, CGGuid*, bool, CGGuid*)>(BaseAddress() + Offsets::UseItem)(this, targetGuid, 0, unk);
	}

	void useOn(CGObject* target)
	{
		return useOn(&target->guid);
	}

	void use() {
		CGGuid target = CGGuid{ 0, 0 };
		this->useOn(&target);
	}
};
#pragma pack(pop)
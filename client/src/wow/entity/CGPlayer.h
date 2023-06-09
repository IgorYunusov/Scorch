#pragma once
#include "wow/entity/CGUnit.h"

enum class PlayerClass : uint8_t
{
	Warrior = 0,
	Paladin = 1,
	Hunter = 3,
	Rogue = 4,
	Priest = 5,
	Shaman = 7,
	Mage = 8,
	Warlock = 9,
	Druid = 11
};

#pragma pack(push, 1)
struct CGPlayerDescriptor : public CGUnitDescriptor
{
	// sizeof(CGUnitData) = 0x035C
	CGGuid duelArbiter; // 0x035C - 0x036C
	CGGuid wowAccount; // 0x036C - 0x037C
	CGGuid lootTargetGuid; // 0x037C - 0x038C
	uint32_t playerFlags; // 0x038C - 0x0390
	uint32_t playerFlagsEx; // 0x0390 - 0x0394
	uint32_t guildRankId;
	uint32_t guildDeleteDate;
	uint32_t guildLevel;
	uint32_t inebriation;
	uint32_t pvpTitle;
	uint32_t arenaFaction;
	uint32_t pvpRank;
	uint32_t duelTeam;
	uint32_t guildTimeStamp; // 0x3AC - 0x3B0
	char padding_questlog[0x640]; // 0x3B0 - 0x9F0
	char padding_visibleItems[0x98]; // 0x9F0 - 0xA88
	uint32_t playerTitle; // 0xA88 - 0xA8C
	uint32_t fakeInebriation; // 0xA8C - 0xA90
	uint32_t virtualPlayerRealm; // 0xA90 - 0xA94
	uint32_t currentSpecId; // 0xA94 - 0xA98
	uint32_t taxiMountAnimKitId; // 0xA98 - 0xA9C
	char padding_avgItemLevel[0x10]; // 0xA9C - 0xAAC
	uint32_t currentBattlePetBreedQuality; // 0xAAC - 0xAB0
	uint32_t honorLevel; // 0xAB0 - 0xAB4
	char padding_customizationChoices[0x120]; // 0xAB4 - 0xBD4
};

struct CGPlayer : public CGUnit
{
	CGPlayerDescriptor* getDescriptor() { return reinterpret_cast<CGPlayerDescriptor*>(this->descriptor); }
	bool isLooting() { return !this->getDescriptor()->lootTargetGuid.isEmpty(); }
};
#pragma pack(pop)
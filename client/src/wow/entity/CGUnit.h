#pragma once
#include "wow/core/CGCore.h"
#include "wow/entity/CGObject.h"

enum UnitDynFlags
{
	UNIT_DYNFLAG_NONE = 0x0000,
	UNIT_DYNFLAG_HIDE_MODEL = 0x0002, // Object model is not shown with this flag
	UNIT_DYNFLAG_LOOTABLE = 0x0004,
	UNIT_DYNFLAG_TRACK_UNIT = 0x0008,
	UNIT_DYNFLAG_TAPPED = 0x0010, // Lua_UnitIsTapped
	UNIT_DYNFLAG_SPECIALINFO = 0x0020,
	UNIT_DYNFLAG_DEAD = 0x0040,
	UNIT_DYNFLAG_REFER_A_FRIEND = 0x0080
};

#pragma pack(push, 1)
struct UnitAura
{
	char padding_0x8C[0x08C];	// 0x00 - 0x08C
	int32_t spellId;			// 0x08C - 0x90
	char padding_0xB0[0x20];	
};

struct CGUnitDescriptor : public CGObjectDescriptor
{
	// sizeof(CGObjectDescriptor) = 0x001C
	CGGuid charm;				// 0x1C - 0x2C
	CGGuid summon;				// 0x2C - 0x3C
	CGGuid critter;				// 0x3C - 0x4C
	CGGuid charmedBy;			// 0x4C - 0x5C
	CGGuid summonedBy;			// 0x5C - 0x6C
	CGGuid createdBy;			// 0x6C - 0x7C
	CGGuid demonCreator;		// 0x7C - 0x8C
	CGGuid lookAtControllerTarget;	// 0x8C - 0x9C
	CGGuid target;					// 0x9C - 0xAC
	CGGuid battlePetCompanionGUID; // 0x00AC - 0x00BC
	uint64_t battlePetDBID; // 0x00BC - 0x00C4
	uint64_t channelData; // 0x00C4 - 0x00CC
	uint32_t summonedByHomeRealm; // 0x00CC - 0x00D0
	uint8_t race; // 0x00D0 - 0x00D1
	uint8_t classId; // 0x00D1 - 0x00D2
	uint8_t playerClassId; // 0x00D2 - 0x00D3
	uint8_t sex; // 0x00D3 - 0x00D4
	uint32_t displayPower; // 0x00D4 - 0x00D8
	uint32_t overrideDisplayPower; // 0x00D8 - 0x00DC 
	uint64_t health; // 0x00DC - 0x00E4
	uint64_t power; // 0x00E4 - 0x00EC
	char padding_0x00FC[0x0010]; // 0x00EC - 0x00FC
	uint64_t maxHealth; // 0x00FC - 0x0104
	uint64_t maxPower; // 0x0104 - 0x010C
	char padding_0x035C[0x0250]; // 0x10C - 0x35C 
};

struct CGUnit : public CGObject
{
	char	 padding_0x15F0[0x15F0 - sizeof(CGObject)];		// sizeof(CGObject) - 0x15F0
	uint32_t movementFlags;									// 0x15F0 - 0x15F4
	char	 padding_0x1898[0x1898 - 0x15F4];				// 0x15F4 - 0x1898
	int32_t  mountId;										// 0x1898 - 0x189C
	char	 padding_0x19C0[0x124];							// 0x189C - 0x19C0
	int32_t  castingSpellId;								// 0x19C0 - 0x19C4
	char	 padding_0x1A18[0x54];							// 0x19C4 - 0x1A18
	int32_t  channelingSpellId;								// 0x1A18 - 0x1A1C

	DEFINE_DATA_ACCESSOR(currentSpeed, float_t, Offsets::CGUnit_MovementStruct, Offsets::CGUnit_MovementStruct_CurrentSpeed)
	DEFINE_DATA_ACCESSOR(runSpeed, float_t, Offsets::CGUnit_MovementStruct, Offsets::CGUnit_MovementStruct_RunSpeed)
	DEFINE_DATA_ACCESSOR(flightSpeed, float_t, Offsets::CGUnit_MovementStruct, Offsets::CGUnit_MovementStruct_FlightSpeed)
	DEFINE_DATA_ACCESSOR(swimSpeed, float_t, Offsets::CGUnit_MovementStruct, Offsets::CGUnit_MovementStruct_SwimSpeed)
	DEFINE_DATA_ACCESSOR(combatStatusFlag, uint32_t, Offsets::CGUnit_CombatStruct, Offsets::CGUnit_CombatStruct_CombatStatusFlag)

	CGUnitDescriptor* getDescriptor() { return reinterpret_cast<CGUnitDescriptor*>(this->descriptor); }
	bool inCombat() { return this->combatStatusFlag() >> 19; }
	bool isTapped() { return (this->getDescriptor()->dynamicFlags & UnitDynFlags::UNIT_DYNFLAG_TAPPED) == UnitDynFlags::UNIT_DYNFLAG_TAPPED; }
	bool isLootable() { return (this->getDescriptor()->dynamicFlags & UnitDynFlags::UNIT_DYNFLAG_LOOTABLE) == UnitDynFlags::UNIT_DYNFLAG_LOOTABLE; }
	bool isDead() { return (this->getDescriptor()->dynamicFlags & UnitDynFlags::UNIT_DYNFLAG_DEAD) == UnitDynFlags::UNIT_DYNFLAG_DEAD; }
	bool isCasting() { return this->castingSpellId != 0 || this->channelingSpellId != 0; }
	bool isTargeting(CGUnit* target) { return this->getDescriptor()->target == target->guid; }

	UnitAura* auraHead() { return reinterpret_cast<UnitAura*>((uintptr_t)this + Offsets::CGUnit_AuraTable); }
	uint32_t auraCount() { return *reinterpret_cast<uint32_t*>((uintptr_t)this + Offsets::CGUnit_AuraCount); }
	bool hasAura(int32_t spellId) {
		auto auras = this->auraHead();
		for (size_t index = 0; index < this->auraCount(); ++index)
		{
			UnitAura* currentAura = &auras[index];
			if (currentAura && currentAura->spellId == spellId)
				return true;
		}
		return false;
	}
};
#pragma pack(pop)

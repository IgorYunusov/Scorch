#pragma once

#define DEFINE_DATA_ACCESSOR(PROPERTY_NAME, PROPERTY_TYPE, DATA_OFFSET, PROPERTY_OFFSET) \
	inline PROPERTY_TYPE PROPERTY_NAME() { \
		uintptr_t dataStore = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(this) + DATA_OFFSET); \
		return *reinterpret_cast<PROPERTY_TYPE*>(dataStore + PROPERTY_OFFSET); \
	} \
\

inline bool ValidPointer(void *ptr) { return !(reinterpret_cast<uintptr_t>(ptr) & 1) && ptr; }

inline uintptr_t BaseAddress()
{
	return reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
}

namespace Offsets
{
	inline uintptr_t CGGameUI_CanPerformAction = 0x16C70C0; 

	inline uintptr_t GameTime = 0x11575D0;
	inline uintptr_t SpriteLeftClick = 0x16306C0;
	inline uintptr_t SpriteRightClick = 0x1630AB0;
	inline uintptr_t FindSlotBySpellId = 0x169B850;
	inline uintptr_t CastSpellBySlot = 0x1699550;
	inline uintptr_t GetBagAndSlot = 0x1860F40;
	inline uintptr_t IsAutoRepeatAction = 0x17C1190;
	inline uintptr_t HasSpellPower = 0x10659B0;
	inline uintptr_t GetSpellCooldown = 0x10613D0;
	inline uintptr_t SpellMinMaxRange = 0x105E8F0;
	inline uintptr_t MoveTo = 0x1263C50;
	inline uintptr_t FaceTo = 0x12639A0;
	inline uintptr_t UseItem = 0x12AB2F0;
	inline uintptr_t RepopMe = 0x11C06D0;
	inline uintptr_t RetrieveCorpse = 0x1615750;
	inline uintptr_t IsCurrentSpell = 0x106AE70;
	inline uintptr_t GetNumLootItems = 0x173A9C0;
	inline uintptr_t GetItemRecord = 0x904790;
	inline uintptr_t SellItem = 0x11C072C;
	inline uintptr_t Dismount = 0x123E240;
	inline uintptr_t SelectGossipOption = 0x1712970;
	inline uintptr_t InGame = 0x30E7F24;
	inline uintptr_t GetBaseFromToken = 0x1AA7270;
	inline uintptr_t GetItemPointer = 0x1346460;
	inline uintptr_t FrameContainer = 0x2E3FE60;
	inline uintptr_t GetMapId = 0x13FBFA0;
	inline uintptr_t SetPetAction = 0x1747760;
	inline uintptr_t CorpsePosition = 0x2CD3AB0;
	inline uintptr_t UseItemParam = 0x2E56A20;
	inline uintptr_t WorldFrameIntersect = 0x1AC80F0;
	inline uintptr_t GetWorld = 0x2075E0;
	inline uintptr_t WorldFrame = 0x31C9C98;
	inline uintptr_t LastEventTime = 0x2E3FE68;
	inline uintptr_t RepeatingSpellId = 0x2E56C04;
	inline uintptr_t ActiveMerchantGuid = 0x312CFE8;
	inline uintptr_t ObjectManager = 0x2EF4068;
	inline uintptr_t LootWindow = 0x31246A0;
	inline uintptr_t GossipNumOptions = 0x311D2AC;
	inline uintptr_t GossipOptions = 0x311DAB0;
	inline uintptr_t ActiveGossiperGuid = 0x3123310;
	inline uintptr_t EquippedBagGuids = 0x31375E0;
	inline uintptr_t CGMailInfo_SendItems = 0x311D000;
	inline uintptr_t CGMailInfo_ActiveMailboxGuid = 0x311CFD8;
	inline uintptr_t PlayerGuid = 0x2DCFCE0;
	inline uintptr_t TargetGuid = 0x2E45E10;
	inline uintptr_t PetGuid = 0x3124588;
	inline uintptr_t MouseOverGuid = 0x30E7F48;
	inline uintptr_t CSimpleEditBox_SetText = 0x245B090;
	inline uintptr_t CGWorldFrame_GetScreenCoordinates = 0x1C18C70;

	inline uintptr_t CGUnit_CastingSpellId = 0x19C0;
	inline uintptr_t CGUnit_ChannelingSpellId = 0x1A18;
	inline uintptr_t CGUnit_CombatStruct = 0x0188;
	inline uintptr_t CGUnit_CombatStruct_CombatStatusFlag = 0x0158;

	inline uintptr_t CGUnit_MovementStruct = 0x0198;
	inline uintptr_t CGUnit_MovementStruct_CurrentSpeed = 0xA0;
	inline uintptr_t CGUnit_MovementStruct_RunSpeed = 0xA8;
	inline uintptr_t CGUnit_MovementStruct_FlightSpeed = 0xB8;
	inline uintptr_t CGUnit_MovementStruct_SwimSpeed = 0xB0;

	inline uintptr_t CGUnit_AuraCount = 0x1B20;
	inline uintptr_t CGUnit_AuraTable = 0x1B24;
	inline uintptr_t CGPlayer_MountId = 0x1898;
	inline uintptr_t CGGameObject_AnimationStatus = 0x014C;

}
#pragma once
#include "wow/Offsets.h"
#include "wow/common/CGGuid.h"
#include "wow/common/CGVector3.h"

enum class ObjectType : uint8_t
{
	Object = 0,
	Item = 1,
	Container = 2,
	Unit = 3,
	Player = 4,
	ActivePlayer = 5,
	GameObject = 6,
	Dynamic = 7,
	Corpse = 8,
	AreaTrigger = 9,
	Scene = 10,
	Conversation = 11
};

#pragma pack(push, 1)
struct CGObjectDescriptor
{
	CGGuid guid;               // 0x00 - 0x10
	uint32_t entryID;          // 0x10 - 0x14
	uint32_t dynamicFlags;     // 0x14 - 0x18
	float_t scale;             // 0x18 - 0x1C
};

struct CGObject
{
	uintptr_t vmt;             // 0x00 - 0x08

	char padding_0x0010[0x08]; // 0x08 - 0x10
	uintptr_t descriptor;      // 0x10 - 0x18

	char padding_0x0020[0x08]; // 0x18 - 0x20
	ObjectType type;           // 0x20 - 0x21

	char padding_0x0030[0x0F]; // 0x21 - 0x30
	CGObject* linked;          // 0x30 - 0x38

	char padding_0x0050[0x18]; // 0x38 - 0x50
	uint32_t guidHash;         // 0x50 - 0x54

	char padding_0x0058[0x04]; // 0x54 - 0x58
	CGGuid guid;               // 0x58 - 0x68 

	char padding_0x0070[0x08]; // 0x68 - 0x70
	CGObject* next;            // 0x70 - 0x78

	const char* getName() { return (*reinterpret_cast<const char* (__fastcall**)(CGObject*)>(this->vmt + (sizeof(uintptr_t) * 15)))(this); }
	float_t getHeight() { return (*reinterpret_cast<float_t(__fastcall**)(CGObject*)>(this->vmt + (sizeof(uintptr_t) * 14)))(this); }
	float_t getFacing() { return (*reinterpret_cast<float_t(__fastcall**)(CGObject*)>(this->vmt + (sizeof(uintptr_t) * 58)))(this); }
	CGObjectDescriptor* getDescriptor() { return reinterpret_cast<CGObjectDescriptor*>(this->descriptor); }

	CGVector3 getPosition() {
		CGVector3 output = { 0, 0, 0 };
		(*reinterpret_cast<CGVector3 * (__fastcall**)(CGObject*, CGVector3*)>(this->vmt + (sizeof(uintptr_t) * 56)))(this, &output);
		return output;
	}

	CGVector3 getHeadPosition() {
		CGVector3 position = this->getPosition();
		position.z += this->getHeight();
		return position;
	}

	void select() { reinterpret_cast<int64_t(__fastcall*)(CGGuid*)>(BaseAddress() + Offsets::SpriteLeftClick)(&this->guid); }
	void interactWith() { reinterpret_cast<int64_t(__fastcall*)(CGGuid*)>(BaseAddress() + Offsets::SpriteRightClick)(&this->guid); }
};
#pragma pack(pop)
#pragma once
#include <cstdint>

#include "wow/Offsets.h"

#pragma pack(push, 1)
struct CGFrame
{
	uintptr_t vmt;             // 0x08

	char padding_0x20[0x18];   // 0x20
	char* name;                // 0x28

	char padding_0xC8[0xA0];   // 0x28 - 0xC8
	uint32_t visibilityFlags;  // 0xC8 - 0xCC

	char padding_0x2E0[0x21C]; // 0xCC - 0x2E8
	CGFrame* next;             // 0x2E8 - 0x2F0

	bool isVisible() { return (visibilityFlags >> 10) & 1; }
	const char* getType() { return (*reinterpret_cast<const char* (__fastcall**)(const CGFrame*)>(vmt + sizeof(uintptr_t) * 9))(this); }
	void leftClick() { (*reinterpret_cast<void(__fastcall**)(CGFrame*, const char*, uint32_t, int64_t)>(vmt + sizeof(uintptr_t) * 68))(this, "LeftButton", 0, 0); }
	void rightClick() { (*reinterpret_cast<void(__fastcall**)(CGFrame*, const char*, uint32_t, int64_t)>(vmt + sizeof(uintptr_t) * 68))(this, "RightButton", 0, 0); }
};

struct CGEditBox : public CGFrame
{
	// sizeof(CGFrame) = 0x2F0
	char padding_0x340[0x50]; // 0x2F0 - 0x340
	char* text;

	void setText(const char* text) {
		return reinterpret_cast<void(__fastcall*)(CGEditBox*, const char*, const char*)>(BaseAddress() + Offsets::CSimpleEditBox_SetText)(this, text, 0);
	}
};

struct CGFrameContainer
{
	char padding_0x0F18[0x0F18]; //0x0000 - 0x0F18
	CGFrame* head;
};
#pragma pack(pop)
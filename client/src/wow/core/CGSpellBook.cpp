#pragma once
#include "pch.h"
#include "wow/core/CGSpellBook.h"

static auto Script_C_GetSpellCooldown = reinterpret_cast<int64_t(__fastcall*)(uint32_t, uint32_t, int32_t, int64_t*, int64_t*, int64_t*, uint64_t, uint64_t, int64_t*)>(BaseAddress() + Offsets::GetSpellCooldown);

static int64_t GetSpellCooldown(CGSpellBook::SpellId spellId, int32_t isPet)
{
	int64_t enabled = 0;
	int64_t duration = 0, startTime = 0, modRate = 1;
	Script_C_GetSpellCooldown(spellId, 0, isPet, &duration, &startTime, &enabled, 0, 0, &modRate);
	if (duration <= 0)
		return 0;
	return duration - (CGCore::GameTime() - startTime);
}

bool CGSpellBook::IsSpellLearned(SpellId spellId)
{
	return CGSpellBook::GetSpellSlotBySpellId(spellId) != 0;
}

bool CGSpellBook::CanCast(SpellId spellId)
{
	return CGSpellBook::CanCast(spellId, CGCore::Player());
}

bool CGSpellBook::CanCast(SpellId spellId, CGUnit* target)
{
	return CGSpellBook::IsSpellLearned(spellId) && !CGSpellBook::IsOnCooldown(spellId)
		&& CGSpellBook::HasSpellPower(spellId) && CGSpellBook::InRange(spellId, target);
}

bool CGSpellBook::HasSpellPower(SpellId spellId)
{
	uint32_t unk0 = 0;
	uint32_t unk1 = 0;
	return reinterpret_cast<bool(__fastcall*)(CGUnit * player, SpellId spellId, uint32_t * flag1, uint32_t * flag2)>(BaseAddress() + Offsets::HasSpellPower)(CGCore::Player(), spellId, &unk0, &unk1);
}

bool CGSpellBook::InRange(SpellId spellId, CGUnit* target)
{
	float_t minRange = 0;
	float_t maxRange = 0;
	reinterpret_cast<bool(__fastcall*)(CGUnit*, SpellId, float*, float*, CGObject*)>(BaseAddress() + Offsets::SpellMinMaxRange)(nullptr, spellId, &minRange, &maxRange, nullptr);

	CGVector3 targetPosition = target->getPosition();
	float_t distance = CGCore::Player()->getPosition().distanceTo(&targetPosition);

	if (distance > maxRange)
		return false;

	return true;
}

bool CGSpellBook::CastSpellById(SpellId spellId)
{
	return CGSpellBook::CastSpellById(spellId, CGCore::Player());
}

bool CGSpellBook::CastSpellById(SpellId spellId, CGUnit* target)
{
	SpellSlot spellSlot = GetSpellSlotBySpellId(spellId);

	if (spellSlot < 0)
		return false;

	reinterpret_cast<int64_t(__fastcall*)(SpellSlot, int32_t, CGObject*, uint8_t, uint8_t)>(BaseAddress() + Offsets::CastSpellBySlot)(spellSlot, 0, target, 0, 0);
	return true;
}

bool CGSpellBook::IsOnCooldown(SpellId spellId)
{
	return GetSpellCooldown(spellId, 0) > 0;
}
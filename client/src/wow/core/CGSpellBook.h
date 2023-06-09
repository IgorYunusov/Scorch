#pragma once
#include <cmath>
#include "wow/core/CGCore.h"

namespace CGSpellBook
{
	typedef int32_t SpellId;
	typedef int32_t SpellSlot;

	inline SpellId GetRepeatingSpellId() 
	{
		return *reinterpret_cast<SpellId*>(BaseAddress() + Offsets::RepeatingSpellId); 
	}

	inline SpellSlot GetSpellSlotBySpellId(SpellId spellId)
	{ 
		return reinterpret_cast<SpellSlot(__fastcall*)(SpellId spellId, bool isPet)>(BaseAddress() + Offsets::FindSlotBySpellId)(spellId, false);
	}
	
	inline bool IsCurrentSpell(SpellId spellId) 
	{ 
		return reinterpret_cast<bool(__fastcall*)(SpellId)>(BaseAddress() + Offsets::IsCurrentSpell)(spellId);
	}

	bool HasSpellPower(SpellId spellId);

	bool InRange(SpellId spellId, CGUnit* target);

	bool CastSpellById(SpellId spellId, CGUnit* target);

	bool CastSpellById(SpellId spellId);

	bool IsOnCooldown(SpellId spellId);

	bool CanCast(SpellId spellId, CGUnit* target);

	bool CanCast(SpellId spellId);

	bool IsSpellLearned(SpellId spellId);
}
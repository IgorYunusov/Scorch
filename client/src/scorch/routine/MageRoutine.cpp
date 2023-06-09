#include "pch.h"
#include <vector>
#include "scorch/routine/MageRoutine.h"
#include "wow/core/CGCore.h"
#include "wow/core/CGSpellBook.h"

static CGSpellBook::SpellId frostArmor = 168;

static CGSpellBook::SpellId frostBolt = 116;
static CGSpellBook::SpellId fireBlast = 2136;

static std::vector<CGUnit*> AttackingUnits() {
	auto objectManager = CGCore::ObjectManager();
	auto player = CGCore::Player();

	std::vector<CGUnit*> attackingUnits;
	objectManager->enumerateObjects([&player, &attackingUnits](CGObject* object) -> void {
		if (object->type != ObjectType::Unit)
			return;

		CGUnit* unit = reinterpret_cast<CGUnit*>(object);
		if (unit->inCombat() && unit->isTargeting(player))
			attackingUnits.push_back(unit);
	});

	return attackingUnits;
}

uint32_t MageRoutine::onRest() {
	auto player = CGCore::Player();

	if (player->isCasting())
	{
		return 200;
	}

	if (!player->hasAura(frostArmor)) 
	{
		CGSpellBook::CastSpellById(frostArmor, player);
		return 250;
	}
	
	return 0;
}

uint32_t MageRoutine::onCombat() { 
	auto player = CGCore::Player();
	auto target = CGCore::Target();

	if (player->isCasting() || !target)
		return 200;

	if (CGSpellBook::CanCast(fireBlast, target))
	{
		CGSpellBook::CastSpellById(fireBlast, target);
		return 100;
	}

	if (CGSpellBook::CanCast(frostBolt, target))
	{
		CGSpellBook::CastSpellById(frostBolt, target);
		return 400;
	}

	return 1000; 
}
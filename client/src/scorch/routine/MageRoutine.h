#pragma once
#include "scorch/routine/CombatRoutine.h"

class MageRoutine : public CombatRoutine
{
	uint32_t onRest();
	uint32_t onCombat();
};
#pragma once
#include "scorch/script/Script.h"
#include "scorch/routine/CombatRoutine.h"

class CombatScript : public Script
{
public:
	CombatScript(CombatRoutine* combatRoutine) : Script() {
		this->combatRoutine = combatRoutine;
	}

protected:
	uint32_t onUpdate() { return this->combatRoutine->update(); }
	void onDraw();

private:
	CombatRoutine* combatRoutine;

};
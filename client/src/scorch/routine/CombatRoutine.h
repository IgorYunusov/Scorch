#pragma once
#include "wow/core/CGCore.h"

class CombatRoutine
{
public:
	CombatRoutine() { }
	~CombatRoutine() { }

	uint32_t update() {
		auto player = CGCore::Player();
		if (player->inCombat())
			return this->onCombat();

		uint32_t restResult = this->onRest();
		if (restResult > 0)
			return restResult;

		return this->onCombat();
	}

protected:
	virtual uint32_t onRest() = 0;
	virtual uint32_t onCombat() = 0;
};
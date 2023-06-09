#include "pch.h"
#include "scorch/script/FishingScript.h"

#include "wow/core/CGCore.h"
#include "wow/core/CGSpellBook.h"

static bool activeBobberPredicate(CGGameObject* gameObject)
{
	return strcmp(gameObject->getName(), "Fishing Bobber") == 0 && gameObject->createdBy(CGCore::Player());
}

static bool lureItemPredicate(CGItem* item)
{
	return strcmp(item->getName(), "Shiny Bauble") == 0;
}

uint32_t FishingScript::onUpdate() {
	auto objectManager = CGCore::ObjectManager();
	auto player = CGCore::Player();
	auto mainHand = player->getEquipment()[EquipmentSlot::MAINHAND];

	if (!mainHand)
	{
		this->state = "No fishing poll equipped";
		return 2000;
	}

	if (player->isLooting())
	{
		this->state = "Looting";
		return 400;
	}

	if (player->castingSpellId != 7731 && player->channelingSpellId != 7731)
	{
		if (player->isCasting())
		{
			this->state = "We are probably applying lure.";
			return 500;
		}

		if (!player->hasAura(8082))
		{
			CGItem* lure = objectManager->findItem(lureItemPredicate);
			if (!lure)
			{
				this->state = "No lures available.";
				return 2000;
			}

			lure->useOn(mainHand);
			this->state = "Applying Lure";
			return 500;
		}

		CGSpellBook::CastSpellById(7731);
		this->state = "Casting Fishing";
		return 400;
	}

	CGGameObject* bobber = objectManager->findGameObject(activeBobberPredicate);
	if (bobber && bobber->isAnimating())
	{
		bobber->interactWith();
		this->state = "Fish on the hook!";
		return 500;
	}

	this->state = "Fishing...";
	return 250;
}

void FishingScript::onDraw() {
	Script::onDraw();
	ImGui::Text("State: %s", this->getState().c_str());
}
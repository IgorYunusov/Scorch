#include "pch.h"
#include "scorch/Client.h"
#include "scorch/navigation/Navigation.h"

#include "wow/core/CGCore.h"
#include "wow/core/CGSpellBook.h"
#include "wow/entity/CGUnit.h"
#include "wow/entity/CGContainer.h"
#include "wow/entity/CGGameObject.h"
#include "wow/Offsets.h"

#include "scorch/graphics/Drawing.h"
#include "scorch/script/CombatScript.h"
#include "scorch/routine/MageRoutine.h"

void renderFrameDebug()
{
	auto frameContainer = CGCore::FrameContainer();
	auto currentFrame = frameContainer->head;
	while (ValidPointer(currentFrame))
	{
		ImGui::Text("Type: %s , Text: %s", currentFrame->getType(), currentFrame->name);
		currentFrame = currentFrame->next;
	}
}

void renderObjectDebug()
{
	auto currentObject = CGCore::ObjectManager()->head;
	while (ValidPointer(currentObject))
	{
		ImGui::Text("Name: %s", currentObject->getName());
		currentObject = currentObject->next;
	}
}

void renderLocalDebug() {
	CGPlayer* player = CGCore::Player();

	if (player)
	{
		ImGui::Text("%s", player->getPosition().toString().c_str());
		if (ImGui::Button("Copy Position"))
		{
			ImGui::SetClipboardText(player->getPosition().toString().c_str());
		}
		ImGui::Text("Current Speed: %f", player->currentSpeed());
		ImGui::Text("Run Speed: %f", player->runSpeed());
		ImGui::Text("Casting Id: %i", player->castingSpellId);
		ImGui::Text("Channeling Id: %i", player->channelingSpellId);
		ImGui::Text("Mount Id: %i", player->mountId);
	}
}

bool renderTargetPath() {
	auto target = CGCore::Target();
	if (!target)
		return false;

	CGVector3 playerPos = CGCore::Player()->getPosition();
	CGVector3 targetPos = target->getPosition();

	if (Navigation::CalculatePath(CGCore::MapId(), &playerPos, &targetPos))
	{
		std::vector<CGVector3> path = Navigation::ActivePath();
		for (int32_t index = 0; index < path.size(); ++index)
		{
			Drawing::DrawPoint(&path.at(index));
		}

		return path.size() > 2;
	}

	return false;
}

void renderInventoryDebug() {
	CGActivePlayer* player = CGCore::Player();

	auto equipmentIter = player->getEquipment();
	ImGui::Text("Equipment");
	for (size_t index = 0; index < equipmentIter.size; ++index)
	{
		auto currentItem = equipmentIter[index];
		if (!currentItem)
		{
			ImGui::Text("\t[%i] Empty", index);
			continue;
		}
		ImGui::Text("\t[%i] %s", index, currentItem->getName());
	}
	ImGui::Separator();

	auto inventoryIter = player->getInventory();
	ImGui::Text("Inventory");
	for (size_t index = 0; index < inventoryIter.size; ++index)
	{
		auto currentItem = inventoryIter[index];
		if (!currentItem)
		{
			ImGui::Text("\t[%i] Empty", index);
			continue;
		}
		ImGui::Text("\t[%i] %s", index, currentItem->getName());
	}
	ImGui::Separator();

	auto bagIter = CGCore::EquippedBags();
	for (size_t index = 0; index < bagIter.size; ++index) {
		auto currentBag = bagIter[index];

		if (!currentBag) {
			ImGui::Text("[%i] Empty", index);
			continue;
		}

		ImGui::Text("[%i] Bag: %s, Slots: %i", index, currentBag->getName(), currentBag->getDescriptor()->numSlots);

		auto itemIter = currentBag->getItems();
		for (size_t itemIndex = 0; itemIndex < itemIter.size; ++itemIndex)
		{
			auto currentItem = itemIter[itemIndex];
			if (!currentItem) {
				ImGui::Text("\t[%i] Empty", itemIndex);
				continue;
			}
			ImGui::Text("\t[%i] %s", itemIndex, currentItem->getName());
		}
	}
}

void renderAuraDebug() {
	ImGui::Text("Player Auras");
	CGActivePlayer* player = CGCore::Player();

	if (!player)
	{
		ImGui::Text("No player...");
		return;
	}

	uint32_t auraCount = player->auraCount();
	ImGui::Text("Aura Count: %u", auraCount);

	for (size_t index = 0; index < auraCount; ++index)
	{
		UnitAura* currentAura = &player->auraHead()[index];
		if(currentAura && currentAura->spellId != 0)
			ImGui::Text("Aura: %i", currentAura->spellId);
	}
}

static Script* activeScript = nullptr;
static CombatRoutine* activeRoutine = nullptr;

void Client::Initialize() {
	activeRoutine = new MageRoutine();
	activeScript = new CombatScript(activeRoutine);
}

void Client::Shutdown() { 
	if (activeScript) { delete activeScript; }
	activeScript = nullptr;

	if (activeRoutine) { delete activeRoutine; }
	activeRoutine = nullptr;
}

void Client::Render() {

	if (!CGCore::InGame())
	{
		ImGui::Text("Not In Game.");
		return;
	}

	if (ImGui::BeginTabBar("ClientTabBar"))
	{
		if (ImGui::BeginTabItem("Local"))
		{
			renderLocalDebug();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Scripting"))
		{
			if (!activeScript)
			{
				ImGui::Text("No Active Script");
			}
			else {
				activeScript->update();
				activeScript->draw();
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Auras"))
		{
			renderAuraDebug();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Inventory"))
		{
			renderInventoryDebug();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Objects"))
		{
			renderObjectDebug();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Frames"))
		{
			renderFrameDebug();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}
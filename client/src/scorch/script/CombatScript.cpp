#include "pch.h"

#include "scorch/script/CombatScript.h"
#include "scorch/graphics/Drawing.h"

void CombatScript::onDraw()
{
	auto currentObject = CGCore::ObjectManager()->head;
	CGVector3 currentPosition = { 0, 0, 0 };
	while (ValidPointer(currentObject))
	{
		if (currentObject->type != ObjectType::Unit)
		{
			currentObject = currentObject->next;
			continue;
		}

		currentPosition = currentObject->getPosition();
		Drawing::DrawPoint(&currentPosition);
		currentObject = currentObject->next;
	}
}

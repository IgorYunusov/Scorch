#pragma once
#include "scorch/script/Script.h"

class FishingScript : public Script
{
protected:
	uint32_t onUpdate();
	void onDraw();
};
#pragma once
#include <string>
#include <chrono>
#include "wow/core/CGCore.h"

class Script {

public:
	Script() { }
	~Script() { }

	std::string getState() { return this->state; }
	uint32_t getStartTime() { return this->m_startTime; }
	uint32_t getRunTime() { return CGCore::GameTime() - this->m_startTime; }

	void update() {
		if (!this->needUpdate())
			return;

		this->m_updateDelta = this->onUpdate();
		this->m_lastUpdate = CGCore::GameTime();
		CGCore::SetEventTime();
	}

	void draw() {
		this->onDraw();
	}

protected:
	std::string state = "Default";
	virtual uint32_t onUpdate() = 0;
	virtual void onDraw() { ImGui::Text("Script::onDraw"); }

private:
	uint32_t m_lastUpdate = 0;
	uint32_t m_updateDelta = 0;
	uint32_t m_startTime = 0;

	bool needUpdate() { return (CGCore::GameTime() - this->m_lastUpdate) >= this->m_updateDelta; }
};
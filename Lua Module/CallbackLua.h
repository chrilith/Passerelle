#pragma once

#include "CallbackHandler.h"

class CallbackLua : public CallbackHandler {

public:
	void PushEvent(Event *event) { Process(event); }
	Event *PopEvent() { return NULL; }
	const CallbackMode GetMode() const { return CBM_DIRECT; }
};

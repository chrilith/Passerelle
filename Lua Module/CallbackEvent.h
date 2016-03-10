#pragma once

#include <queue>
#include "CallbackHandler.h"

class CallbackEvent : public CallbackHandler {

protected:
	std::queue<Event *> _queue;

public:
	virtual void PushEvent(Event *event);
	virtual Event *PopEvent();
	virtual const CallbackMode GetMode() const { return CBM_EVENT; }

	~CallbackEvent() {
		while (Event *e = PopEvent()) {
			delete e;
		}
	}
};

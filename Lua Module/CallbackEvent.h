#pragma once
#pragma warning(push)
#pragma warning(disable : 4577)	// noexcept

#include <queue>
#include "CallbackHandler.h"

#pragma warning(pop)

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


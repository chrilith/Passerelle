#pragma once

#include "CallbackEvent.h"
#include "Config.h"

#ifdef WITH_FSUIPC

#define OFFSET 0x736D

class CallbackFSUIPC : public CallbackEvent {

public:
	CallbackFSUIPC();
	~CallbackFSUIPC();

	void PushEvent(Event *event);
	const CallbackMode GetMode() const { return CBM_FSUIPC; }

private:
	bool _initialized;
};

#endif

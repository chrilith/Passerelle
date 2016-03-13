#include "CallbackFSUIPC.h"

#ifdef WITH_FSUIPC
#include "FSUIPC_User.h"

CallbackFSUIPC::CallbackFSUIPC() {
	DWORD dwResult;
	_initialized = (FSUIPC_Open(SIM_ANY, &dwResult) == 1);
}

void CallbackFSUIPC::PushEvent(Event *event) {
	if (!_initialized)
		return;

	DWORD dwResult;
	static char poll = 0;

	CallbackEvent::PushEvent(event);
	poll = (poll++) % 2;

	if (FSUIPC_Write(OFFSET, sizeof(poll), &poll, &dwResult)) {
		FSUIPC_Process(&dwResult);
	}
}

CallbackFSUIPC::~CallbackFSUIPC() {
	FSUIPC_Close();
}

#endif
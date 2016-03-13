#pragma once

#include <Windows.h>
#include "DeviceEvent.h"

struct PageChangeEvent : DeviceEvent {
	PageChangeEvent(int type, lua_State *L) : DeviceEvent(type, L) {}
	DWORD dwPage;
	bool bSetActive;
};

#pragma once

#include <Windows.h>
#include "DeviceEvent.h"

struct ButtonChangeEvent : DeviceEvent {
	ButtonChangeEvent(int type, lua_State *L) : DeviceEvent(type, L) {}
	DWORD dwButtons;
};

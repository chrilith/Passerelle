#pragma once

#include "DeviceEvent.h"

struct DeviceChangeEvent : DeviceEvent {
	DeviceChangeEvent(int type, lua_State *L) : DeviceEvent(type, L) {}
	bool bAdded;
};

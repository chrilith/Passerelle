#pragma once

#include "Event.h"

struct DeviceEvent : Event {
	DeviceEvent(int type, lua_State *L) : Event(type, L) {}

	const char *devMod;
	int devIdx;
};

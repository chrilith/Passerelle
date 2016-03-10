#pragma once

#include "ButtonChangeEvent.h"
#include "DeviceChangeEvent.h"
#include "PageChangeEvent.h"
#include "Import/Source/ThreadLock.h"

enum CallbackMode {
	CBM_DIRECT = 1,
	CBM_EVENT,
	CBM_FSUIPC
};

class CallbackHandler {
protected:
	CThreadLock _lock;

public:
	virtual void PushEvent(Event *event) = 0;
	virtual Event *PopEvent() = 0;
	void Process(Event *event);
	virtual const CallbackMode GetMode() const = 0;

protected:
	void ButtonChange(ButtonChangeEvent *event);
	void DeviceChange(DeviceChangeEvent *event);
	void PageChange(PageChangeEvent *event);

public:
	static CallbackHandler *Factory(int mode);
};

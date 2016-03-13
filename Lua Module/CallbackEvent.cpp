#include "CallbackEvent.h"
#include "Debug.h"

void CallbackEvent::PushEvent(Event *event) {
	_lock.Acquire();
	_queue.push(event);
	_lock.Release();
}

Event *CallbackEvent::PopEvent() {
	Event *event = NULL;

	_lock.Acquire();
	if (_queue.size() > 0) {
		event = _queue.front();
		_queue.pop();
	}
	_lock.Release();
	
	return event;
}

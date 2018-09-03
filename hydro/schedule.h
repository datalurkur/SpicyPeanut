#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include "commandqueue.h"
#include "state.h"
#include "util.h"

#include <algorithm>
#include <list>

class CommandQueue;

class Event
{
public:
	enum Type
	{
		ChangeState = 0,
		SampleData
	};

public:
	Event(Type type, long long timeInMinutes);

	long long getTime() const;
	Type getType() const;

private:
	Type _type;
	long long _time;
};

class ChangeStateEvent : public Event
{
public:
	ChangeStateEvent(long long timeInMinutes, State::Property property, bool value);

	State::Property getProperty() const;
	bool getValue() const;

private:
	State::Property _property;
	bool _value;
};

class SampleDataEvent : public Event
{
public:
	SampleDataEvent(long long timeInMinutes);
};

class Schedule
{
public:
	Schedule();
	virtual ~Schedule();

	void addEvent(std::shared_ptr<Event> event);

	void start(std::shared_ptr<CommandQueue> commandQueue);
	void stop();

private:
	void processEvents(std::shared_ptr<CommandQueue> commandQueue);

private:
	State _state;

	std::list<std::shared_ptr<Event>> _events;

	std::chrono::system_clock::time_point _reference;

	int _eventIndex;
	std::mutex _mutex;
	std::shared_ptr<std::thread> _thread;
	std::shared_ptr<CancellableWait> _awaiter;
};

#endif
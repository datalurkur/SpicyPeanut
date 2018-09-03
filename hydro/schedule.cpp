#include "command.h"
#include "log.h"
#include "schedule.h"

#include <ctime>

Event::Event(Type type, long long timeInMinutes) : _type(type), _time(timeInMinutes)
{ }

long long Event::getTime() const { return _time; }
Event::Type Event::getType() const { return _type; }

ChangeStateEvent::ChangeStateEvent(long long timeInMinutes, State::Property property, bool value) : Event(Event::Type::ChangeState, timeInMinutes), _property(property), _value(value)
{ }
State::Property ChangeStateEvent::getProperty() const { return _property; }
bool ChangeStateEvent::getValue() const { return _value; }

SampleDataEvent::SampleDataEvent(long long timeInMinutes) : Event(Event::Type::SampleData, timeInMinutes)
{ }

Schedule::Schedule() : _awaiter(nullptr), _thread(nullptr), _eventIndex(0)
{

}

Schedule::~Schedule()
{
	stop();
}

void Schedule::addEvent(std::shared_ptr<Event> event)
{
	_events.push_back(event);
}

void Schedule::start(std::shared_ptr<CommandQueue> commandQueue)
{
	stop();

	LogInfo("Starting schedule");
	std::lock_guard<std::mutex> lock(_mutex);

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tNow = std::chrono::system_clock::to_time_t(now);
	tm* tpNow = std::localtime(&tNow);
	tpNow->tm_hour = 0;
	tpNow->tm_min = 0;
	tpNow->tm_sec = 0;
	_reference = std::chrono::system_clock::from_time_t(std::mktime(tpNow));

	_events.sort([](std::shared_ptr<Event> a, std::shared_ptr<Event> b) -> bool { return a->getTime() < b->getTime(); });

	_thread = std::make_shared<std::thread>(&Schedule::processEvents, this, commandQueue);
}

void Schedule::stop()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_thread == nullptr) { return; }
	if (_awaiter != nullptr) { _awaiter->cancel(); }
	if (_thread->joinable()) { _thread->join(); }
	_thread = nullptr;
}

void Schedule::processEvents(std::shared_ptr<CommandQueue> commandQueue)
{
	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(_mutex);

			// Compute minutes since midnight
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			std::chrono::minutes msm = std::chrono::duration_cast<std::chrono::minutes>(now - _reference);
			long long minutesSinceMidnight = msm.count();
			LogInfo("It has been " << minutesSinceMidnight << " minutes since midnight");

			// Collect events that have passed and apply state
			State newState = _state;
			bool sampleData = false;
			std::list<std::shared_ptr<Event>>::iterator itr = _events.begin();
			std::advance(itr, _eventIndex);
			while (itr != _events.end())
			{
				if ((*itr)->getTime() > minutesSinceMidnight)
				{
					break;
				}

				switch ((*itr)->getType())
				{
				case Event::Type::ChangeState:
				{
					std::shared_ptr<ChangeStateEvent> cse = std::static_pointer_cast<ChangeStateEvent>(*itr);
					newState.setProperty(cse->getProperty(), cse->getValue());
					LogInfo("Setting property " << cse->getProperty() << " to " << cse->getValue() << " at past time " << cse->getTime());
					break;
				}
				case Event::Type::SampleData:
				{
					std::shared_ptr<SampleDataEvent> sde = std::static_pointer_cast<SampleDataEvent>(*itr);
					LogInfo("Marking sample data at past time " << sde->getTime());
					sampleData = true;
					break;
				}
				default:
					LogInfo("Unknown event type");
					break;
				}

				++_eventIndex;
				++itr;
			}

			// Apply state delta
			std::map<State::Property, bool> updates;
			_state.getDelta(newState, updates);
			for (std::pair<State::Property, bool> kvp : updates)
			{
				LogInfo("Queueing setprop command to key " << kvp.first << " to " << kvp.second);
				commandQueue->queueCommand(std::make_shared<SetPropertyCommand>(kvp.first, kvp.second));
			}

			// Sample data
			if (sampleData)
			{
				LogInfo("Sampling data");
				// FIXME
			}

			// Build awaiter for next event
			if (itr == _events.end())
			{
				LogInfo("Clearing awaiter, a new day has come");
				std::chrono::system_clock::time_point nextMidnight = _reference + std::chrono::hours(24);
				_awaiter = std::make_shared<WaitForTime>(nextMidnight);
				_reference = nextMidnight;
			}
			else
			{
				std::chrono::system_clock::time_point nextBreak = _reference + std::chrono::minutes((*itr)->getTime());
				LogInfo("Setting awaiter to halt until " << Log::GetReadableTime(nextBreak));
				_awaiter = std::make_shared<WaitForTime>(nextBreak);
			}
		}

		// Wait on awaiter
		if (_awaiter != nullptr)
		{
			LogInfo("Waiting...");
			bool ready = _awaiter->wait();
			if (!ready) { break; }
		}
	}
}
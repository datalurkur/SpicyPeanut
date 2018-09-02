#include "command.h"
#include "commandqueue.h"
#include "plan.h"
#include "util.h"
#include "log.h"

Plan::Plan(const std::string& name, std::shared_ptr<Command> command, std::shared_ptr<CancellableWait> awaiter):
    _name(name), _command(command), _awaiter(awaiter), _thread(nullptr), _stopped(false)
{ }

Plan::~Plan()
{
    stop();
}

void Plan::start(std::shared_ptr<CommandQueue> commandQueue)
{
    if (_thread != nullptr) { stop(); }
    LogInfo("Starting plan: " << _name);
    _stopped = false;
    _thread = std::make_shared<std::thread>(&Plan::executePlanLoop, this, commandQueue);
}

void Plan::stop()
{
    if (_thread == nullptr) { return; }
    LogInfo("Stopping plan: " << _name);
    _stopped = true;
    _awaiter->cancel();
    if (_thread->joinable())
    {
        _thread->join();
    }
}

void Plan::executePlanLoop(std::shared_ptr<CommandQueue> commandQueue)
{
    while (!_stopped)
    {
        LogDebug("Plan waiting for execute condition: " << _name);
        if (!_awaiter->wait()) { continue;  }

        LogDebug("Executing command for plan: " << _name);
        commandQueue->queueCommand(_command);
        _awaiter = _awaiter->next();
        if (_awaiter == nullptr)
        {
            LogDebug("No more commands to process");
            break;
        }
    }
    LogInfo("Plan has stopped: " << _name);
}

RecurringPlan::RecurringPlan(const std::string& name, std::shared_ptr<Command> command, long long frequencyInSeconds, long long delayInSeconds):
    Plan(name, command, std::make_shared<WaitForTime>(frequencyInSeconds, delayInSeconds))
{ }
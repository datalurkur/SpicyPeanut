#ifndef _PLAN_H_
#define _PLAN_H_

#include <atomic>
#include <memory>
#include <mutex>

class CancellableWait;
class Command;
class CommandQueue;

class Plan
{
public:
    Plan(const std::string& name, std::shared_ptr<Command> command, std::shared_ptr<CancellableWait> awaiter);
    virtual ~Plan();

    void start(std::shared_ptr<CommandQueue> commandQueue);
    void stop();

private:
    void executePlanLoop(std::shared_ptr<CommandQueue> commandQueue);

private:
    std::string _name;

    std::shared_ptr<Command> _command;
    std::shared_ptr<CancellableWait> _awaiter;

    std::shared_ptr<std::thread> _thread;

    std::atomic<bool> _stopped;
};

class RecurringPlan : public Plan
{
public:
    RecurringPlan(const std::string& name, std::shared_ptr<Command> command, long long frequencyInSeconds, long long delayInSeconds);
};

class OnOffPlan : public RecurringPlan
{
public:
};

#endif
#include "commandqueue.h"
#include "schedule.h"
#include "plan.h"
#include "log.h"

Schedule::Schedule() { }
Schedule::~Schedule() { stop(); }

void Schedule::addPlan(std::shared_ptr<Plan> plan)
{
    _plans.push_back(plan);
}

void Schedule::start(std::shared_ptr<CommandQueue> commandQueue)
{
    LogInfo("Starting scheduler");
    for (std::shared_ptr<Plan> plan : _plans)
    {
        plan->start(commandQueue);
    }
}

void Schedule::stop()
{
    LogInfo("Stopping scheduler");
    for (std::shared_ptr<Plan> plan : _plans)
    {
        plan->stop();
    }
}
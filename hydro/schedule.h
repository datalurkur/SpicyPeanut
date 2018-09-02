#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <list>

class CommandQueue;
class Plan;

class Schedule
{
public:
    Schedule();
    virtual ~Schedule();

    void addPlan(std::shared_ptr<Plan> plan);

    void start(std::shared_ptr<CommandQueue> commandQueue);
    void stop();

private:
    std::list<std::shared_ptr<Plan>> _plans;
};

#endif
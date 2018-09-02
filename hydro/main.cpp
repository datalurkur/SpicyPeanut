#include <memory>

#include "command.h"
#include "commandqueue.h"
#include "log.h"
#include "plan.h"
#include "schedule.h"

int main()
{
    Log::Init("");

    std::shared_ptr<CommandQueue> commandQueue = std::make_shared<CommandQueue>();

    std::shared_ptr<Schedule> schedule = std::make_shared<Schedule>();

    std::shared_ptr<FloodCommand> floodCommand = std::make_shared<FloodCommand>();
    std::shared_ptr<DrainCommand> drainCommand = std::make_shared<DrainCommand>();
    std::shared_ptr<SamplePHCommand> samplePHCommand = std::make_shared<SamplePHCommand>();

    std::shared_ptr<CompositeCommand> getSamplesCommand = std::make_shared<CompositeCommand>();
    getSamplesCommand->addCommand(samplePHCommand);

    std::shared_ptr<RecurringPlan> sample = std::make_shared<RecurringPlan>("Get Data Samples", getSamplesCommand, 15, 0);
    std::shared_ptr<RecurringPlan> flood = std::make_shared<RecurringPlan>("Flood", floodCommand, 60, 0);
    std::shared_ptr<RecurringPlan> drain = std::make_shared<RecurringPlan>("Drain", drainCommand, 60, 15);

    schedule->addPlan(sample);
    schedule->addPlan(flood);
    schedule->addPlan(drain);

    schedule->start(commandQueue);

    // TODO - Make this a thread
    while (true)
    {
        bool commandReady = commandQueue->waitForNextCommand();
        if (!commandReady) { continue; }
        do
        {
            std::shared_ptr<Command> cmd = commandQueue->dequeueCommand();
            if (cmd == nullptr) { break; }
            cmd->execute();
        } while (true);
    }
}
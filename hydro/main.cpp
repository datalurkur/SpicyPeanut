#include <memory>

#include "command.h"
#include "commandqueue.h"
#include "log.h"
#include "schedule.h"
#include "ucinterface.h"

std::shared_ptr<CommandQueue> commandQueue;

int main()
{
    Log::Init("");
    UCInterface::Init();

    std::shared_ptr<Schedule> schedule = std::make_shared<Schedule>();

    // Lights on in the morning
    schedule->addEvent(std::make_shared<ChangeStateEvent>(
        60 * 7,
        State::Property::LightOn,
        true
    ));

    // Lights off in the evening
    schedule->addEvent(std::make_shared<ChangeStateEvent>(
        60 * 21,
        State::Property::LightOn,
        false
    ));

    // Watering schedule
    int timesToWater = 8;
    for (int i = 0; i < timesToWater; ++i)
    {
        int start = (24 / timesToWater) * i * 60;
        int finish = start + 15;

        // Start watering
        schedule->addEvent(std::make_shared<ChangeStateEvent>(
            start,
            State::Property::ReservoirFlooded,
            true
        ));

        // Finish watering
        schedule->addEvent(std::make_shared<ChangeStateEvent>(
            finish,
            State::Property::ReservoirFlooded,
            false
        ));
    }

    // Temperature / humidity sampling
    int timesToSample = 48;
    for (int i = 0; i < timesToSample; ++i)
    {
        int time = 60 * 24 * i / timesToSample;
        schedule->addEvent(std::make_shared<SampleDataEvent>(time));
    }

    commandQueue = std::make_shared<CommandQueue>();
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

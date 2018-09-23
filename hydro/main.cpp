#include <memory>

#include "command.h"
#include "commandqueue.h"
#include "dbinterface.h"
#include "globalstate.h"
#include "log.h"
#include "schedule.h"
#include "socketlistener.h"
#include "ucinterface.h"

/*
TODO:
- Add i2c code
*/

static std::shared_ptr<CommandQueue> commandQueue = nullptr;

void signalHandler(int sig)
{
    LogInfo("Halting command queue");
    commandQueue->interrupt();
}

int main()
{
    Log::Init(true, "");

    DBInterface::Init();
    UCInterface::Init();

    commandQueue = std::make_shared<CommandQueue>();

    std::shared_ptr<Schedule> schedule = std::make_shared<Schedule>();
    std::shared_ptr<GlobalState> state = std::make_shared<GlobalState>();
    std::shared_ptr<SocketListener> rcListener = std::make_shared<SocketListener>(commandQueue);

    //UCInterface::Instance->setECParameter("TDS", true);

    // Lights on in the morning (4am)
    schedule->addEvent(std::make_shared<ChangeStateEvent>(
        60 * 4,
        State::Property::LightOn,
        true
    ));

    // Lights off in the evening (10pm)
    schedule->addEvent(std::make_shared<ChangeStateEvent>(
        60 * 22,
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

    schedule->start(commandQueue);

    bool commandReady = false;
    while (commandReady = commandQueue->waitForNextCommand())
    {
        std::shared_ptr<Command> cmd = nullptr;
        while (cmd = commandQueue->dequeueCommand())
        {
            cmd->execute(state);
        }
    }

    LogInfo("Spicyd exiting");
    return 0;
}

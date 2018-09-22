#include "command.h"
#include "config.h"
#include "dbinterface.h"
#include "globalstate.h"
#include "log.h"
#include "ucinterface.h"

CompositeCommand::CompositeCommand()
{ }

void CompositeCommand::addCommand(std::shared_ptr<Command> subCommand)
{
    _subCommands.push_back(subCommand);
}

void CompositeCommand::execute(std::shared_ptr<GlobalState> state)
{
    for (std::shared_ptr<Command> subCommand : _subCommands)
    {
        subCommand->execute(state);
    }
}

SetPropertyCommand::SetPropertyCommand(State::Property property, bool value): _property(property), _value(value)
{ }

void SetPropertyCommand::execute(std::shared_ptr<GlobalState> state)
{
	LogInfo("Setting property " << _property << " to " << _value);
    switch (_property)
    {
    case State::Property::LightOn:
        UCInterface::Instance->setLightState(_value);
        break;
    case State::Property::ReservoirFlooded:
        UCInterface::Instance->setReservoirState(_value);
        break;
    default:
        LogInfo("Unknown property type");
        break;
    }
}

void SampleDataCommand::execute(std::shared_ptr<GlobalState> state)
{
    if (!state->getDataCollectionEnabled())
    {
        LogInfo("Data collection is disabled, skipping");
        return;
    }

    LogInfo("Sampling data");
    bool sampledDHT = false;
    for (int i = 0; i < DHT22_RETRIES; ++i)
    {
        if (UCInterface::Instance->sampleDHT22())
        {
            float temperature = UCInterface::Instance->getLastTemperature();
            float humidity    = UCInterface::Instance->getLastHumidity();

            DBInterface::Instance->logSample(DBInterface::SampleType::temperature, temperature);
            DBInterface::Instance->logSample(DBInterface::SampleType::humidity,    humidity);

            sampledDHT = true;
            break;
        }
    }
    if (!sampledDHT)
    {
        LogInfo("Failed to get data from DHT22");
    }

    // FIXME - Add more sampling
}

SetDataCollectionState::SetDataCollectionState(bool enabled): _enabled(enabled)
{ }

void SetDataCollectionState::execute(std::shared_ptr<GlobalState> state)
{
    state->setDataCollectionEnabled(_enabled);
}
#include "command.h"
#include "config.h"
#include "log.h"
#include "ucinterface.h"

CompositeCommand::CompositeCommand()
{ }

void CompositeCommand::addCommand(std::shared_ptr<Command> subCommand)
{
    _subCommands.push_back(subCommand);
}

void CompositeCommand::execute()
{
    for (std::shared_ptr<Command> subCommand : _subCommands)
    {
        subCommand->execute();
    }
}

SetPropertyCommand::SetPropertyCommand(State::Property property, bool value): _property(property), _value(value)
{ }

void SetPropertyCommand::execute()
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

void SampleDataCommand::execute()
{
    LogInfo("Sampling data");
    bool sampledDHT = false;
    for (int i = 0; i < DHT22_RETRIES; ++i)
    {
        if (UCInterface::Instance->sampleDHT22())
        {
            float lastTemperature = UCInterface::Instance->getLastTemperature();
            float humidity = UCInterface::Instance->getLastHumidity();

            // FIXME - Log these values

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
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

    float pHValue;
    if (UCInterface::Instance->getPH(false, pHValue))
    {
        DBInterface::Instance->logSample(DBInterface::SampleType::ph, pHValue);
    }
    else
    {
        LogInfo("Failed to sample pH, calibration may be required");
    }

    float ecValue;
    float tdsValue;
    if (UCInterface::Instance->getEC(false, ecValue, tdsValue))
    {
        DBInterface::Instance->logSample(DBInterface::SampleType::ec, ecValue);
        DBInterface::Instance->logSample(DBInterface::SampleType::tds, tdsValue);
    }
    else
    {
        LogInfo("Failed to sample EC, calibration may be required");
    }
}

SetDataCollectionStateCommand::SetDataCollectionStateCommand(bool enabled): _enabled(enabled)
{ }

void SetDataCollectionStateCommand::execute(std::shared_ptr<GlobalState> state)
{
    LogInfo("Setting data collection state to " << (_enabled ? "on" : "off"));
    state->setDataCollectionEnabled(_enabled);
}

SetCalibrationPointCommand::SetCalibrationPointCommand(UCInterface::CalibrationType type, UCInterface::CalibrationPoint point): _type(type), _point(point)
{ }

void SetCalibrationPointCommand::execute(std::shared_ptr<GlobalState> state)
{
    switch (_type)
    {
    case UCInterface::CalibrationType::pH:
        UCInterface::Instance->setPHCalibrationPoint(_point);
        break;
    case UCInterface::CalibrationType::EC:
        UCInterface::Instance->setECCalibrationPoint(_point);
        break;
    default:
        LogWarn("Unknown calibration type " << _type);
        break;
    }
}

void ResetProbesCommand::execute(std::shared_ptr<GlobalState> state)
{
    UCInterface::Instance->resetProbes();
}

#include "device.h"
#include "log.h"
#include "state.h"
#include "ucinterface.h"

State::State()
{
    for (int i = 0; i < NumProperties; ++i)
    {
        Property prop = (Property)i;
        std::shared_ptr<BinaryDevice> device = UCInterface::Instance->getBinaryDeviceForProperty(prop);
        if (device == nullptr)
        {
            LogWarn("No device found matching property " << prop);
            continue;
        }
        _properties[i] = device->getState();
        _settable[i] = !device->isInput();
    }
}

void State::setProperty(Property prop, bool value)
{
    _properties[(int)prop] = value;
}

bool State::isSettable(Property prop) const
{
    return _settable[(int)prop];
}

State& State::operator=(State& other)
{
    for (int i = 0; i < NumProperties; ++i)
    {
        _properties[i] = other._properties[i];
    }
    return *this;
}

bool State::operator==(const State& other) const
{
    for (int i = 0; i < NumProperties; ++i)
    {
        if (_properties[i] != other._properties[i]) { return false; }
    }
    return true;
}

bool State::operator!=(const State& other) const
{
    return !(*this == other);
}

void State::getDelta(const State& next, std::map<Property, bool>& delta) const
{
    for (int i = 0; i < NumProperties; ++i)
    {
        if (_properties[i] != next._properties[i])
        {
            delta[(Property)i] = next._properties[i];
        }
    }
}

std::string State::GetPropertyName(State::Property property)
{
    switch (property)
    {
    case LightOn:
        return std::string("Light On");
    case ReservoirFlooded:
        return std::string("Reservoir Flooded");
    case OxygenatorOn:
        return std::string("Oxygenator On");
    case DrainCycleActive:
        return std::string("Drain Cycle Active");
    case FillCycleActive:
        return std::string("Fill Cycle Active");
    case ReservoirEmpty:
        return std::string("Reservoir Empty");
    case ReservoirFull:
        return std::string("Reservoir Full");
    case FillReservoirEmpty:
        return std::string("Fill Reservoir Empty");
    default:
        return std::string("Unknown Property");
    }
}

#include "state.h"
#include "ucinterface.h"

State::State()
{
    for (int i = 0; i < NumProperties; ++i)
    {
        switch ((Property)i)
        {
        case Property::LightOn:
            _properties[i] = UCInterface::Instance->getLightState();
            break;
        case Property::ReservoirFlooded:
            _properties[i] = UCInterface::Instance->getReservoirState();
            break;
        default:
            _properties[i] = false;
            break;
        }
    }
}

void State::setProperty(Property prop, bool value)
{
    _properties[prop] = value;
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

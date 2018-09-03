#include "state.h"

State::State()
{
    for (int i = 0; i < NumProperties; ++i)
    {
        _properties[i] = false;
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

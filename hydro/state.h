#ifndef _STATE_H_
#define _STATE_H_

#include <map>

class State
{
public:
    enum Property
    {
        LightOn = 0,
        ReservoirFlooded,
        NumProperties
    };

public:
    State();

    void setProperty(Property prop, bool value);

    State& operator=(State& other);
    bool operator==(const State& other) const;
    bool operator!=(const State& other) const;

    void getDelta(const State& next, std::map<Property, bool>& delta) const;

private:
    bool _properties[Property::NumProperties];
};

#endif

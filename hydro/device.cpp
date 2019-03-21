#include "device.h"
#include "log.h"

#if !_WINDOWS_BUILD
#include "wiringPi.h"
#endif

BinaryDevice::BinaryDevice(const std::string& name, int pin, bool invert, bool input):
    _name(name), _pin(pin), _invert(invert), _input(input)
{
    pinMode(pin, (input ? INPUT : OUTPUT));
    // Disable binary output devices by default
    if (!_input) { setState(false); }
}

bool BinaryDevice::isInput() const
{
    return _input;
}

void BinaryDevice::setState(bool enabled)
{
    bool targetState = (enabled ^ _invert);
    LogInfo("Setting " << _name << " on pin " << _pin << " to " << (enabled ? "on" : "off") << " / " << (targetState ? "HIGH" : "LOW"));
#if !_WINDOWS_BUILD
    digitalWrite(_pin, targetState ? HIGH : LOW);
#endif
}

bool BinaryDevice::getState() const
{
    bool ret = false;
#if _WINDOWS_BUILD
    ret = true;
#else
    ret = ((digitalRead(_pin) ^ _invert) == HIGH);
#endif
    LogInfo("Current " << _name << " state is " << (ret ? "on" : "off"));
    return ret;
}

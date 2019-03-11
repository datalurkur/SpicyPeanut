#ifndef DEVICE_H
#define DEVICE_H

#include <string>

class BinaryDevice
{
public:
    BinaryDevice(const std::string& name, int pin, bool invert, bool input);

    bool isInput() const;

    void setState(bool enabled);
    bool getState() const;

private:
    std::string _name;
    int _pin;
    bool _invert;
    bool _input;
};

#endif

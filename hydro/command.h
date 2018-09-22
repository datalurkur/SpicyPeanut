#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "state.h"
#include "ucinterface.h"

#include <list>
#include <memory>

class GlobalState;

class Command
{
public:
    virtual void execute(std::shared_ptr<GlobalState> state) = 0;
};

class CompositeCommand : public Command
{
public:
    CompositeCommand();

    void addCommand(std::shared_ptr<Command> subCommand);

    virtual void execute(std::shared_ptr<GlobalState> state);

private:
    std::list<std::shared_ptr<Command>> _subCommands;
};

class SetPropertyCommand : public Command
{
public:
    SetPropertyCommand(State::Property property, bool value);

    virtual void execute(std::shared_ptr<GlobalState> state);

private:
    State::Property _property;
    bool _value;
};

class SampleDataCommand : public Command
{
public:
    virtual void execute(std::shared_ptr<GlobalState> state);
};

class SetDataCollectionStateCommand : public Command
{
public:
    SetDataCollectionStateCommand(bool enabled);

    virtual void execute(std::shared_ptr<GlobalState> state);

private:
    bool _enabled;
};

class SetCalibrationPointCommand : public Command
{
public:
    SetCalibrationPointCommand(UCInterface::CalibrationType type, UCInterface::CalibrationPoint point);

    virtual void execute(std::shared_ptr<GlobalState> state);

private:
    UCInterface::CalibrationType _type;
    UCInterface::CalibrationPoint _point;
};

class ResetProbesCommand : public Command
{
public:
    virtual void execute(std::shared_ptr<GlobalState> state);
};

#endif

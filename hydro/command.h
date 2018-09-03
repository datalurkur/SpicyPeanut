#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "state.h"

#include <list>
#include <memory>

class Command
{
public:
    virtual void execute() = 0;
};

class CompositeCommand : public Command
{
public:
    CompositeCommand();

    void addCommand(std::shared_ptr<Command> subCommand);

    virtual void execute();

private:
    std::list<std::shared_ptr<Command>> _subCommands;
};

class SetPropertyCommand : public Command
{
public:
	SetPropertyCommand(State::Property property, bool value);

	virtual void execute();

private:
	State::Property _property;
	bool _value;
};

class SamplePHCommand : public Command
{
public:
    virtual void execute();
};

#endif
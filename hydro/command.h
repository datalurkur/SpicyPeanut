#ifndef _COMMAND_H_
#define _COMMAND_H_

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

class FloodCommand : public Command
{
public:
    virtual void execute();
};

class DrainCommand : public Command
{
public:
    virtual void execute();
};

class SamplePHCommand : public Command
{
public:
    virtual void execute();
};

#endif
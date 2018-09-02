#include "command.h"
#include "log.h"

CompositeCommand::CompositeCommand()
{ }

void CompositeCommand::addCommand(std::shared_ptr<Command> subCommand)
{
    _subCommands.push_back(subCommand);
}

void CompositeCommand::execute()
{
    for (std::shared_ptr<Command> subCommand : _subCommands)
    {
        subCommand->execute();
    }
}

void FloodCommand::execute()
{
    LogInfo("Flooding root system");
}

void DrainCommand::execute()
{
    LogInfo("Draining root system");
}

void SamplePHCommand::execute()
{
    LogInfo("Sampling reservoir PH");
}
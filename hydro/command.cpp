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

SetPropertyCommand::SetPropertyCommand(State::Property property, bool value): _property(property), _value(value)
{ }

void SetPropertyCommand::execute()
{
	LogInfo("Setting property " << _property << " to " << _value);
}

void SamplePHCommand::execute()
{
    LogInfo("Sampling reservoir PH");
}
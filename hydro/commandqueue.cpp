#include "command.h"
#include "commandqueue.h"
#include "util.h"
#include "log.h"

CommandQueue::CommandQueue()
{
    _awaiter = std::make_shared<WaitForCompletion>();
}

CommandQueue::~CommandQueue()
{
    interrupt();
}

void CommandQueue::queueCommand(std::shared_ptr<Command> command)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _commands.push(command);
    _awaiter->complete();
}

std::shared_ptr<Command> CommandQueue::dequeueCommand()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_commands.empty()) { return nullptr; }
    std::shared_ptr<Command> ret = _commands.front();
    _commands.pop();
    return ret;
}

bool CommandQueue::waitForNextCommand()
{
    _awaiter->reset();
    return _awaiter->wait();
}

void CommandQueue::interrupt()
{
    _awaiter->cancel();
}
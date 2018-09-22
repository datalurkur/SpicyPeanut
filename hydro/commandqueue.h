#ifndef _COMMANDQUEUE_H_
#define _COMMANDQUEUE_H_

#include <memory>
#include <queue>
#include <mutex>

class WaitForCompletion;
class Command;

class CommandQueue
{
public:
    CommandQueue();
    virtual ~CommandQueue();

    void queueCommand(std::shared_ptr<Command> command);
    std::shared_ptr<Command> dequeueCommand();

    bool waitForNextCommand();
    void interrupt();

private:
    std::shared_ptr<WaitForCompletion> _awaiter;

    std::mutex _mutex;
    std::queue<std::shared_ptr<Command>> _commands;
};

#endif
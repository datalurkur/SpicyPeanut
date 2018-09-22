#ifndef _COMMANDLISTENER_H_
#define _COMMANDLISTENER_H_

#include <memory>
#include <thread>

class CommandQueue;

class CommandListener
{
private:
    const unsigned int kTimeoutSeconds = 1;

public:
    CommandListener(int socketDescriptor, std::shared_ptr<CommandQueue> commandQueue);
    virtual ~CommandListener();

    void startListening();
    void stopListening();

private:
    void listenThreadLoop();
    void readCommand();

private:
    int _socketDescriptor;
    std::shared_ptr<CommandQueue> _commandQueue;

    bool _keepListening;
    std::shared_ptr<std::thread> _listenThread;
};

#endif

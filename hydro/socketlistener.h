#ifndef _SOCKETLISTENER_H_
#define _SOCKETLISTENER_H_

#include <list>
#include <memory>
#include <thread>

class CommandListener;
class CommandQueue;

class SocketListener
{
private:
    const unsigned int kTimeoutSeconds = 3;

public:
    SocketListener(std::shared_ptr<CommandQueue> commandQueue);
    virtual ~SocketListener();

    void startListening();
    void stopListening();

private:
    void listenThreadLoop();
    void handleNewClient();

private:
    bool _keepListening;
    int _socketDescriptor;
    std::shared_ptr<std::thread> _listenThread;

    std::shared_ptr<CommandQueue> _commandQueue;
    std::list<std::shared_ptr<CommandListener>> _listeners;
};

#endif

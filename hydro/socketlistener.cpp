#include "commandlistener.h"
#include "log.h"
#include "protocol.h"
#include "socketlistener.h"

#include <cstring>

#if _WINDOWS_BUILD
#include <WinSock2.h>
#define close closesocket
#else
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

SocketListener::SocketListener(std::shared_ptr<CommandQueue> commandQueue): _commandQueue(commandQueue)
{
    startListening();
}

SocketListener::~SocketListener()
{
    stopListening();
    for (std::shared_ptr<CommandListener> listener : _listeners)
    {
        listener->stopListening();
    }
}

void SocketListener::startListening()
{
    if (_listenThread != nullptr) { return; }

    _keepListening = true;
    _listenThread = std::make_shared<std::thread>(&SocketListener::listenThreadLoop, this);
}

void SocketListener::stopListening()
{
    if (_listenThread == nullptr) { return; }

    _keepListening = false;
    if (_listenThread->joinable())
    {
        _listenThread->join();
    }
    _listenThread = nullptr;
}

void SocketListener::listenThreadLoop()
{
    // Create listen socket
    _socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketDescriptor < 0)
    {
        LogWarn("Failed to open listen socket");
        return;
    }

    // Allow socket reuse
    int reuse = 1;
    int reuseResult = setsockopt(_socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    if (reuseResult < 0)
    {
        LogWarn("Failed to allow socket reuse");
        return;
    }

    // Bind listen socket
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(kRemoteCommandPort);
    int bindResult = bind(_socketDescriptor, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bindResult < 0)
    {
        LogWarn("Failed to bind listen socket");
        return;
    }

    int listenResult = listen(_socketDescriptor, 32);
    if (listenResult < 0)
    {
        LogWarn("Failed to listen on bound socket");
        close(_socketDescriptor);
        return;
    }

    while (_keepListening)
    {
        timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = kTimeoutSeconds;
        timeout.tv_usec = 0;

        fd_set readSet, writeSet, exceptionSet;
        int selectResult = select(_socketDescriptor + 1, &readSet, &writeSet, &exceptionSet, &timeout);
        if (selectResult == -1 || FD_ISSET(_socketDescriptor, &exceptionSet))
        {
            // Error while waiting
            LogWarn("Error waiting for clients to connect");
            break;
        }
        else if (selectResult == 0)
        {
            // Hit timeout
            continue;
        }
        else if (FD_ISSET(_socketDescriptor, &readSet))
        {
            // Incoming connection
            handleNewClient();
        }
    }

    close(_socketDescriptor);
}

void SocketListener::handleNewClient()
{
    sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    unsigned int sizeOfClientAddr = sizeof(clientAddr);
    int clientDescriptor = accept(_socketDescriptor, (struct sockaddr *)&clientAddr, &sizeOfClientAddr);
    if (clientDescriptor < 0)
    {
        LogWarn("Failed to accept incoming client connection");
        return;
    }

    std::shared_ptr<CommandListener> newCommandListener = std::make_shared<CommandListener>(clientDescriptor, _commandQueue);
    _listeners.push_back(newCommandListener);
}

#include "commandlistener.h"
#include "log.h"
#include "protocol.h"
#include "socketlistener.h"

#include <cstring>

#if _WINDOWS_BUILD
#include <WinSock2.h>
#define close closesocket
#else
#include <errno.h>
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

#if _WINDOWS_BUILD
    WSADATA wsaData;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != NO_ERROR) { return; }
#endif

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

#if _WINDOWS_BUILD
    WSACleanup();
#endif
}

void SocketListener::listenThreadLoop()
{
    LogInfo("Setting up listen socket");

    // Create listen socket
    _socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketDescriptor < 0)
    {
        LogWarn("Failed to open listen socket");
        reportLastSocketError();
        return;
    }

/*
    // Allow socket reuse
    int reuse = 1;
    int reuseResult = setsockopt(_socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    if (reuseResult < 0)
    {
        LogWarn("Failed to allow socket reuse");
        return;
    }
*/

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
        reportLastSocketError();
        return;
    }

    int listenResult = listen(_socketDescriptor, 32);
    if (listenResult < 0)
    {
        LogWarn("Failed to listen on bound socket");
        reportLastSocketError();
        close(_socketDescriptor);
        return;
    }

    while (_keepListening)
    {
        timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = kTimeoutSeconds;
        timeout.tv_usec = 0;

        fd_set readSet, exceptionSet;
        FD_ZERO(&readSet);
        FD_SET(_socketDescriptor, &readSet);
        FD_ZERO(&exceptionSet);
        FD_SET(_socketDescriptor, &exceptionSet);

        int selectResult = select(_socketDescriptor + 1, &readSet, NULL, &exceptionSet, &timeout);
        if (selectResult == -1 || FD_ISSET(_socketDescriptor, &exceptionSet))
        {
            // Error while waiting
            LogWarn("Error waiting for clients to connect");
            reportLastSocketError();
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
        else
        {
            LogInfo("Unexpected scenario");
        }
    }

    close(_socketDescriptor);
}

void SocketListener::handleNewClient()
{
    LogInfo("Incoming connection");

    sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
#if _WINDOWS_BUILD
    int sizeOfClientAddr = sizeof(clientAddr);
#else
    unsigned int sizeOfClientAddr = sizeof(clientAddr);
#endif
    int clientDescriptor = accept(_socketDescriptor, (struct sockaddr *)&clientAddr, &sizeOfClientAddr);
    if (clientDescriptor < 0)
    {
        LogWarn("Failed to accept incoming client connection");
        reportLastSocketError();
        return;
    }

    std::shared_ptr<CommandListener> newCommandListener = std::make_shared<CommandListener>(clientDescriptor, _commandQueue);
    _listeners.push_back(newCommandListener);
}

void SocketListener::reportLastSocketError()
{
#if _WINDOWS_BUILD
    int errorCode = WSAGetLastError();
    switch (errorCode)
    {
    case WSANOTINITIALISED:
        LogError("Socket layer used without being initialized");
        break;
    case WSAEFAULT:
        LogError("Failed to allocate resources for socket operation");
        break;
    case WSAENETDOWN:
        LogError("Network subsystem failure");
        break;
    case WSAEINVAL:
        LogError("Invalid parameters");
        break;
    case WSAEINTR:
        LogError("A blocking operation was cancelled");
        break;
    case WSAEINPROGRESS:
        LogError("A socket operation is already in progress");
        break;
    case WSAENOTSOCK:
        LogError("Invalid socket descriptor");
        break;
    }
#else
    LogError("Socket error - " << strerror(errno));
#endif
}
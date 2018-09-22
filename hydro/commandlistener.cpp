#include "command.h"
#include "commandlistener.h"
#include "commandqueue.h"
#include "log.h"
#include "protocol.h"

#include <cstring>

#if _WINDOWS_BUILD
#include <WinSock2.h>
#define close closesocket
#else
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

CommandListener::CommandListener(int socketDescriptor, std::shared_ptr<CommandQueue> commandQueue) : _socketDescriptor(socketDescriptor), _commandQueue(commandQueue)
{
    LogInfo("Accepted client connection, awaiting commands");
    startListening();
}

CommandListener::~CommandListener()
{
    stopListening();
}

void CommandListener::startListening()
{
    if (_listenThread != nullptr) { return; }

    _keepListening = true;
    _listenThread = std::make_shared<std::thread>(&CommandListener::listenThreadLoop, this);
}

void CommandListener::stopListening()
{
    if (_listenThread == nullptr) { return; }

    _keepListening = false;
    if (_listenThread->joinable())
    {
        _listenThread->join();
    }
    _listenThread = nullptr;
}

void CommandListener::listenThreadLoop()
{
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
            LogWarn("Error while waiting for data from client");
            break;
        }
        else if (selectResult == 0)
        {
            // Hit timeout
            continue;
        }
        else if (FD_ISSET(_socketDescriptor, &readSet))
        {
            readCommand();
        }
    }

    close(_socketDescriptor);
    LogInfo("Client connection closed");
}

void CommandListener::readCommand()
{
    unsigned char commandBuffer[kRemoteCommandSize];
    unsigned int bytesRead = recv(_socketDescriptor, (char*)&commandBuffer[0], kRemoteCommandSize, MSG_WAITALL);
    if (bytesRead != kRemoteCommandSize) { return; }
    switch (commandBuffer[0])
    {
    case RemoteCommand::SuspendSampling:
        _commandQueue->queueCommand(std::make_shared<SetDataCollectionStateCommand>(false));
        break;
    case RemoteCommand::ResumeSampling:
        _commandQueue->queueCommand(std::make_shared<SetDataCollectionStateCommand>(true));
        break;
    case RemoteCommand::CalibratePHMid:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::pH, UCInterface::CalibrationPoint::Mid));
        break;
    case RemoteCommand::CalibratePHLow:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::pH, UCInterface::CalibrationPoint::Low));
        break;
    case RemoteCommand::CalibratePHHigh:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::pH, UCInterface::CalibrationPoint::High));
        break;
    case RemoteCommand::CalibrateECDry:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::EC, UCInterface::CalibrationPoint::Dry));
        break;
    case RemoteCommand::CalibrateECLow:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::EC, UCInterface::CalibrationPoint::Low));
        break;
    case RemoteCommand::CalibrateECHigh:
        _commandQueue->queueCommand(std::make_shared<SetCalibrationPointCommand>(UCInterface::CalibrationType::EC, UCInterface::CalibrationPoint::High));
        break;
    case RemoteCommand::ResetProbes:
        _commandQueue->queueCommand(std::make_shared<ResetProbesCommand>());
    default:
        LogWarn("Unrecognized remote command " << commandBuffer[0]);
    }
}

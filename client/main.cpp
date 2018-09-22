#include "protocol.h"

#include <cstring>
#include <iostream>
#include <memory>

#if _WINDOWS_BUILD
#include <WinSock2.h>
#define close closesocket
#else
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

bool done = false;
int socketDescriptor = 0;

void signalHandler(int sig)
{
    close(socketDescriptor);
    done = true;
}

void sendCommand(RemoteCommand type)
{
    char commandBuffer[kRemoteCommandSize];
    commandBuffer[0] = (unsigned char)type;

    int bytesSent = send(socketDescriptor, commandBuffer, kRemoteCommandSize, 0);
    if (bytesSent != kRemoteCommandSize)
    {
        std::cout << "Failed to send command" << std::endl;
    }
}

int main()
{
#if _WINDOWS_BUILD
    WSADATA wsaData;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != NO_ERROR)
    {
        std::cout << "Failed to initialize Winsock" << std::endl;
        return -1;
    }
#endif

    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor < 0)
    {
        std::cout << "Failed to create socket" << std::endl;
        return -1;
    }

    hostent* server = gethostbyname("localhost");
    if (server == NULL)
    {
        std::cout << "Failed to get server by hostname" << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    memmove(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serverAddr.sin_port = htons(kRemoteCommandPort);

    int connectResult = connect(socketDescriptor, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connectResult < 0)
    {
        std::cout << "Failed to connect to server" << std::endl;
        return -1;
    }

    std::cout << "Connected with result " << connectResult << std::endl;

    while (!done)
    {
        int selection;
        std::cout << "Make a selection:" << std::endl;
        std::cout << " 1.  Exit" << std::endl;
        std::cout << " 2.  Suspend Data Collection" << std::endl;
        std::cout << " 3.  Resume Data Collection" << std::endl;
        std::cout << " 4.  Calibrate pH Probe (Mid  - Step 1)" << std::endl;
        std::cout << " 5.  Calibrate pH Probe (Low  - Step 2)" << std::endl;
        std::cout << " 6.  Calibrate pH Probe (High - Step 3)" << std::endl;
        std::cout << " 7.  Calibrate EC Probe (Dry  - Step 1)" << std::endl;
        std::cout << " 8.  Calibrate EC Probe (Low  - Step 2)" << std::endl;
        std::cout << " 9.  Calibrate EC Probe (High - Step 3)" << std::endl;
        std::cout << " 10. Reset Probes (Calibration Done)" << std::endl;
        std::cin >> selection;
        switch (selection)
        {
        case 1:
            done = true;
            break;
        case 2:
            sendCommand(RemoteCommand::SuspendSampling);
            break;
        case 3:
            sendCommand(RemoteCommand::ResumeSampling);
            break;
        case 4:
            sendCommand(RemoteCommand::CalibratePHMid);
            break;
        case 5:
            sendCommand(RemoteCommand::CalibratePHLow);
            break;
        case 6:
            sendCommand(RemoteCommand::CalibratePHHigh);
            break;
        case 7:
            sendCommand(RemoteCommand::CalibrateECDry);
            break;
        case 8:
            sendCommand(RemoteCommand::CalibrateECLow);
            break;
        case 9:
            sendCommand(RemoteCommand::CalibrateECHigh);
            break;
        case 10:
            sendCommand(RemoteCommand::ResetProbes);
            break;
        default:
            std::cout << "Invalid selection" << std::endl;
        }
    }

    close(socketDescriptor);

#if _WINDOWS_BUILD
    WSACleanup();
#endif

    return 0;
}

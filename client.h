#pragma once

#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    Client(const char* serverIp, int port);
    ~Client();

    bool init();
    bool connectToServer();
    void communicate();
    void cleanup();

private:
    void receiveResponse();
    void receiveFile(const std::string& userName);
    void sendFile(const std::string& filePath);
    void ensureUserFolderExists(const std::string& userName);

private:
    const char* serverIp;
    int port;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;
};

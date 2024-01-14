#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    Client(const char* serverIp, int port) : serverIp(serverIp), port(port), clientSocket(INVALID_SOCKET) {}

    ~Client() {
        cleanup();
    }

    bool init() {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }

        // Create socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            cleanup();
            return false;
        }

        // Set up server address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        // Use regular string for IP address
        InetPton(AF_INET, serverIp, &serverAddr.sin_addr);

        return true;
    }

    bool connectToServer() {
        // Connect to the server
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
            cleanup();
            return false;
        }

        return true;
    }

    void communicate() {
        std::string userInput;
        std::cout << "Enter command (LIST to get file list): ";
        std::getline(std::cin, userInput);

        send(clientSocket, userInput.c_str(), (int)userInput.length(), 0);

        if (userInput == "LIST") {
            receiveFileList();
        } else {
            // General response that the command is not supported
            receiveResponse();
        }
    }

    // Receive the list response from the server
    void receiveFileList() {
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "File list from server:\n" << buffer << std::endl;
        }
    }

    // Receive the general response from the server
    void receiveResponse() {
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received from server: " << buffer << std::endl;
        }
    }

    void cleanup() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }

        WSACleanup();
    }

private:
    const char* serverIp;
    int port;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;
};

int main() {
    const char* serverIp = "127.0.0.1";
    int port = 12345;

    Client client(serverIp, port);

    if (client.init() && client.connectToServer()) {
        client.communicate();
        client.cleanup();
    }

    return 0;
}
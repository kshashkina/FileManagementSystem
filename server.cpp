#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    Server(int port) : port(port), serverSocket(INVALID_SOCKET) {}

    ~Server() {
        cleanup();
    }

    bool init() {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            cleanup();
            return false;
        }

        // Set up server address
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        // Bind the socket
        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            cleanup();
            return false;
        }

        std::cout << "Server initialized on port " << port << std::endl;
        return true;
    }

    bool startListening() {
        // Listen for incoming connections
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            cleanup();
            return false;
        }

        std::cout << "Server listening for incoming connections..." << std::endl;
        return true;
    }

    void acceptConnection() {
        // Accept a client connection
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            cleanup();
            return;
        }

        // Receive data from the client
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received data: " << buffer << std::endl;

            // Send a response back to the client
            const char* response = "Hello, client! This is the server.";
            send(clientSocket, response, (int)strlen(response), 0);
        }

        // Clean up
        closesocket(clientSocket);
    }

    void cleanup() {
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        WSACleanup();
    }

private:
    int port;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;
};

int main() {
    Server server(12345);

    if (server.init() && server.startListening()) {
        server.acceptConnection();
        server.cleanup();
    }

    return 0;
}

#include <iostream>
#include <WinSock2.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <vector>

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

            if (strncmp(buffer, "LIST", 4) == 0) {
                sendFileList(clientSocket);
            }
            else if (strncmp(buffer, "DELETE", 6) == 0){
                handleDeleteCommand(clientSocket, buffer);
            }
            else if (strncmp(buffer, "GET", 3) == 0){
                handleGetCommand(clientSocket, buffer);
            }
            else {
                const char* response = "Unknown command. Valid commands: LIST";
                send(clientSocket, response, (int)strlen(response), 0);
            }
        }

        // Clean up
        closesocket(clientSocket);
    }

    void sendFileList(SOCKET clientSocket) {
        std::ostringstream fileList;

        for (const auto& entry : std::filesystem::directory_iterator("C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage")) {
            fileList << entry.path().filename().string() << "\n";
        }

        const std::string& fileListStr = fileList.str();
        send(clientSocket, fileListStr.c_str(), (int)fileListStr.length(), 0);
    }

    void handleDeleteCommand(SOCKET clientSocket, const char* buffer) {
        std::string fileName(buffer + 7);
        fileName = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + fileName;

        if (std::filesystem::exists(fileName)) {
            if (std::filesystem::remove(fileName)) {
                const char* response = "File deleted successfully.";
                send(clientSocket, response, (int)strlen(response), 0);
            } else {
                const char* response = "Failed to delete the file. Check file permissions.";
                send(clientSocket, response, (int)strlen(response), 0);
            }
        } else {
            const char* response = "File not found. Check the file name and try again.";
            send(clientSocket, response, (int)strlen(response), 0);
        }
    }

    void handleGetCommand(SOCKET clientSocket, const char* buffer) {
        std::string fileName(buffer + 4);
        fileName = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + fileName;

        if (std::filesystem::exists(fileName)) {
            std::ifstream file(fileName, std::ios::binary);
            if (file.is_open()) {
                // Calculating file size
                file.seekg(0, std::ios::end);
                size_t fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                // Creating a buffer sized as text in order not to reallocate memory
                std::vector<char> fileContent(fileSize);
                file.read(fileContent.data(), fileSize);

                send(clientSocket, fileContent.data(), static_cast<int>(fileSize), 0);
            } else {
                const char* response = "Failed to open the file for reading.";
                send(clientSocket, response, (int)strlen(response), 0);
            }
        } else {
            const char* response = "File not found. Check the file name and try again.";
            send(clientSocket, response, (int)strlen(response), 0);
        }
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
        while (true){
            server.acceptConnection();
        }
    }

    return 0;
}

#include <iostream>
#include <WinSock2.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>

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
        serverAddr.sin_family = AF_INET; //ipv4
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

        while (conected) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
                cleanup();
                return false;
            }

            std::thread(&Server::acceptConnection, this, clientSocket).detach();
        }
        return true;
    }

    void acceptConnection(SOCKET clientSocket) {
        std::string userName;

        while (true) {
            char buffer[1024];
            memset(buffer, 0, 1024);
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Received data: " << buffer << std::endl;

                if (strncmp(buffer, "NAME", 4) == 0) {
                    if (userName.empty()) {
                        userName = std::string(buffer + 5);
                    } else {
                        std::cout << "You have already declared your name!";
                    }
                } else if (strncmp(buffer, "LIST", 4) == 0) {
                    handleListCommand(clientSocket, userName);
                } else if (strncmp(buffer, "DELETE", 6) == 0) {
                    handleDeleteCommand(clientSocket, buffer, userName);
                } else if (strncmp(buffer, "GET", 3) == 0) {
                    handleGetCommand(clientSocket, buffer, userName);
                } else if (strncmp(buffer, "PUT", 3) == 0) {
                    handlePutCommand(clientSocket, buffer, userName);
                } else if (strncmp(buffer, "INFO", 4) == 0) {
                    handleInfoCommand(clientSocket, buffer, userName);
                } else if (strncmp(buffer, "EXIT", 4) == 0) {
                    conected = false;
                    break;
                } else {
                    const char* response = "Unknown command.";
                    send(clientSocket, response, (int)strlen(response), 0);
                }
            }
        }
        closesocket(clientSocket);
    }


    void handleListCommand(SOCKET clientSocket, const std::string& userName) {
        ensureUserFolderExists(userName);
        std::ostringstream fileList;

        for (const auto& entry : std::filesystem::directory_iterator("C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName)) {
            fileList << entry.path().filename().string() << "\n";
        }

        const std::string& fileListStr = fileList.str();
        send(clientSocket, fileListStr.c_str(), (int)fileListStr.length(), 0);
    }

    void handleDeleteCommand(SOCKET clientSocket, const char* buffer, const std::string& userName) {
        ensureUserFolderExists(userName);
        std::string fileName(buffer + 7);
        fileName = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName + "\\" + fileName;

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

    void handleGetCommand(SOCKET clientSocket, const char* buffer, const std::string& userName) {
        ensureUserFolderExists(userName);
        std::string fileName(buffer + 4);
        std::string fullPath = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName + "\\" + fileName;

        if (std::filesystem::exists(fullPath)) {
            std::ifstream file(fullPath, std::ios::binary);
            if (file.is_open()) {
                // Calculating file size
                file.seekg(0, std::ios::end);
                size_t fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                const int chunkSize = 1024;
                char buffer[chunkSize];

                // Send the file size to the client
                send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);


                // Send the filename to the client
                size_t lastSlashPos = fullPath.find_last_of("\\");
                if (lastSlashPos != std::string::npos) {
                    fileName = fullPath.substr(lastSlashPos + 1);
                }
                send(clientSocket, fileName.c_str(), static_cast<int>(fileName.length()), 0);

                // Send the file content to the client in chunks
                while (!file.eof()) {
                    file.read(buffer, chunkSize);
                    send(clientSocket, buffer, static_cast<int>(file.gcount()), 0);
                }
            } else {
                const char* response = "Failed to open the file for reading.";
                send(clientSocket, response, (int)strlen(response), 0);
            }
        } else {
            const char* response = "File not found. Check the file name and try again.";
            send(clientSocket, response, (int)strlen(response), 0);
        }
    }

    void handlePutCommand(SOCKET clientSocket, const char* buffer, const std::string& userName) {
        ensureUserFolderExists(userName);
        std::string fileName(buffer + 4);
        size_t pos = fileName.find_last_of("/\\");
        if (pos != std::string::npos) {
            fileName = fileName.substr(pos + 1);
        }
        fileName = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName + "\\" + fileName;
        if (std::filesystem::exists(fileName)) {
            const char* response = "File already exists.";
            send(clientSocket, response, (int)strlen(response), 0);
            return;
        }

        // Receive the file data size from the client
        int fileSize;
        if (recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0) <= 0) {
            const char* response = "Failed to receive file size.";
            send(clientSocket, response, (int)strlen(response), 0);
            return;
        }

        // Receive the file data from the client in chunks
        std::ofstream file(fileName, std::ios::binary);
        if (file.is_open()) {
            const int chunkSize = 1024;
            char buffer[chunkSize];

            while (fileSize > 0) {
                int bytesToReceive = std::min(chunkSize, fileSize);
                int bytesReceived = recv(clientSocket, buffer, bytesToReceive, 0);

                if (bytesReceived > 0) {
                    file.write(buffer, bytesReceived);
                    fileSize -= bytesReceived;
                } else {
                    std::cerr << "Failed to receive file content." << std::endl;
                    break;
                }
            }

            const char* response = "File received and saved successfully.";
            send(clientSocket, response, (int)strlen(response), 0);
        } else {
            const char* response = "Failed to create or write to the file.";
            send(clientSocket, response, (int)strlen(response), 0);
        }
    }

    void handleInfoCommand(SOCKET clientSocket, const char* buffer, const std::string& userName) {
        ensureUserFolderExists(userName);
        std::string fileName(buffer + 5);
        fileName = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName + "\\" + fileName;

        // Get information about the file
        std::filesystem::path filePath(fileName);
        if (std::filesystem::exists(filePath)) {
            std::ostringstream info;
            info << "File Information:\n";
            info << "Path: " << fileName << "\n";
            info << "Size: " << std::filesystem::file_size(filePath) << " bytes\n";

            std::chrono::time_point<std::filesystem::__file_clock> lastWriteTime = std::filesystem::last_write_time(filePath);

            // Convert to a system_clock time point
            auto lastWriteTimeSysClock = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    lastWriteTime - std::filesystem::__file_clock::now() + std::chrono::system_clock::now()
            );

            std::time_t lastWriteTime_c = std::chrono::system_clock::to_time_t(lastWriteTimeSysClock);
            info << "Last modified: " << std::ctime(&lastWriteTime_c);

            const std::string& infoStr = info.str();
            send(clientSocket, infoStr.c_str(), (int)infoStr.length(), 0);
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

    void ensureUserFolderExists(const std::string& userName) {
        std::filesystem::path userFolderPath = "C:\\KSE IT\\Client Server Concepts\\csc_first\\serverStorage\\" + userName;

        if (!std::filesystem::exists(userFolderPath)) {
            std::filesystem::create_directory(userFolderPath);
            std::cout << "User folder created: " << userFolderPath << std::endl;
        }
    }

    bool checkConnection(){
        return conected;
    }

private:
    int port;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    WSADATA wsaData;
    bool conected = true;
};

int main() {
    Server server(12345);

    if (server.init() && server.startListening()) {
        while (server.checkConnection()){
        }
    }

    return 0;
}

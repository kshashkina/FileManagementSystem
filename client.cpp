#include "Client.h"

Client::Client(const char* serverIp, int port) : serverIp(serverIp), port(port), clientSocket(INVALID_SOCKET) {}

Client::~Client() {
    cleanup();
}

bool Client::init() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        cleanup();
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    InetPton(AF_INET, serverIp, &serverAddr.sin_addr);

    return true;
}

bool Client::connectToServer() {
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
        cleanup();
        return false;
    }

    return true;
}

void Client::communicate() {
    std::cout << "Enter your name using NAME <'yourName'>" << std::endl;
    std::string userName;
    std::getline(std::cin, userName);
    send(clientSocket, userName.c_str(), static_cast<int>(userName.length()), 0);
    std::cin.clear();
    std::cout << "Commands:\nGET <filename> to get file\nLIST to get file list\nPUT <filename> to create a new file\nDELETE <filename> to delete a file\nINFO <filename> to get information about a file\nEXIT to exit\n";

    while (true) {
        std::string userInput;
        std::cout << "Enter command:\n";
        std::getline(std::cin, userInput);

        if (userInput == "EXIT") {
            send(clientSocket, userInput.c_str(), static_cast<int>(userInput.length()), 0);
            break;
        }
        if (userInput.substr(0, 3) == "PUT") {
            size_t startQuotePos = userInput.find('"');
            size_t endQuotePos = userInput.find('"', startQuotePos + 1);

            if (startQuotePos != std::string::npos && endQuotePos != std::string::npos) {
                std::string filePath = userInput.substr(startQuotePos + 1, endQuotePos - startQuotePos - 1);
                sendFile(filePath);
                continue;
            } else {
                std::cerr << "Invalid file path." << std::endl;
            }
        }
        if (userInput.substr(0, 3) == "GET") {
            send(clientSocket, userInput.c_str(), static_cast<int>(userInput.length()), 0);
            std::string folderName = userName.substr(5);
            receiveFile(folderName);
        } else {
            send(clientSocket, userInput.c_str(), static_cast<int>(userInput.length()), 0);
            receiveResponse();
        }
    }
}

void Client::receiveResponse() {
    char buffer[1024];
    memset(buffer, 0, 1024);
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Received from server:\n" << buffer << std::endl;
    }
}

void Client::receiveFile(const std::string& userName) {
    ensureUserFolderExists(userName);
    size_t fileSize;
    int bytesReceivedSize = recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    if (bytesReceivedSize > 0) {
        char fileName[1024];
        memset(fileName, 0, sizeof(fileName));
        int bytesReceivedName = recv(clientSocket, fileName, sizeof(fileName), 0);
        if (bytesReceivedName > 0) {
            std::string saveDirectory = "C:\\KSE IT\\Client Server Concepts\\csc_first\\clientStorage\\" + userName + "\\";

            if (!saveDirectory.empty() && saveDirectory.back() != '\\') {
                saveDirectory += '\\';
            }

            std::ofstream outputFile(saveDirectory + fileName, std::ios::binary);
            if (outputFile.is_open()) {
                const int chunkSize = 1024;
                char buffer[chunkSize];

                while (fileSize > 0) {
                    int bytesToReceive = std::min(chunkSize, static_cast<int>(fileSize));
                    int bytesReceived = recv(clientSocket, buffer, bytesToReceive, 0);

                    if (bytesReceived > 0) {
                        outputFile.write(buffer, bytesReceived);
                        fileSize -= bytesReceived;
                    } else {
                        std::cerr << "Failed to receive file content." << std::endl;
                        break;
                    }
                }

                std::cout << "File received and saved as: " << saveDirectory + fileName << std::endl;
            } else {
                std::cerr << "Failed to save the file." << std::endl;
            }
        } else {
            std::cerr << "Failed to receive file name." << std::endl;
        }
    } else {
        std::cerr << "Failed to receive file size." << std::endl;
    }
}

void Client::sendFile(const std::string& filePath) {
    std::ifstream fileToSend(filePath, std::ios::binary);
    if (!fileToSend.is_open()) {
        std::cerr << "Failed to open the file: " << filePath << std::endl;
        return;
    }

    fileToSend.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(fileToSend.tellg());
    fileToSend.seekg(0, std::ios::beg);

    std::string putCommand = "PUT " + filePath;
    send(clientSocket, putCommand.c_str(), static_cast<int>(putCommand.length()), 0);
    send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

    const int chunkSize = 1024;
    char buffer[chunkSize];

    while (!fileToSend.eof()) {
        fileToSend.read(buffer, chunkSize);
        int bytesRead = static_cast<int>(fileToSend.gcount());
        send(clientSocket, buffer, bytesRead, 0);
    }

    fileToSend.close();
    receiveResponse();
}

void Client::cleanup() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }

    WSACleanup();
}

void Client::ensureUserFolderExists(const std::string& userName) {
    std::filesystem::path userFolderPath = "C:\\KSE IT\\Client Server Concepts\\csc_first\\clientStorage\\" + userName;

    if (!std::filesystem::exists(userFolderPath)) {
        std::filesystem::create_directory(userFolderPath);
    }
}

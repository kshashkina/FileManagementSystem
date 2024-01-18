#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>

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
        std::cout << "Commands:\nGET <filename> to get file\nLIST to get file list\nPUT <filename> to create new file\nDELETE <filename> to delete file\nINFO <filename> to get information about file\nEXIT to exit\n";
        while (true) {
            std::string userInput;
            std::cout << "Enter command:\n";
            std::getline(std::cin, userInput);

            if (userInput == "EXIT") {
                send(clientSocket, userInput.c_str(), (int)userInput.length(), 0);
                break;
            }
            if (userInput.substr(0, 3) == "PUT") {
                // Extract the file path from the user input
                size_t startQuotePos = userInput.find('"');
                size_t endQuotePos = userInput.find('"', startQuotePos + 1);

                if (startQuotePos != std::string::npos && endQuotePos != std::string::npos) {
                    std::string filePath = userInput.substr(startQuotePos + 1, endQuotePos - startQuotePos - 1);

                    sendFile(filePath);
                } else {
                    std::cerr << "Invalid file path." << std::endl;
                }
            }
            if (userInput.substr(0, 3) == "GET") {
                send(clientSocket, userInput.c_str(), (int) userInput.length(), 0);
                receiveFile();
            }
            else{
                send(clientSocket, userInput.c_str(), (int)userInput.length(), 0);
                receiveResponse();
            }

        }
    }

    void receiveResponse() {
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received from server: \n " << buffer << std::endl;
        }
    }

    void receiveFile() {
        // Receive file content
        char buffer[1024];
        char file[1024];
        memset(buffer, 0, 1024);
        int bytesReceivedText = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceivedText > 0) {
            // Receive file name
            std::string fileName;
            memset(file, 0, 1024);
            int bytesReceivedName = recv(clientSocket, file, sizeof(file), 0);
            if (bytesReceivedName > 0) {
                fileName = file;

                // Specify the directory where you want to save the files
                std::string saveDirectory = "C:\\KSE IT\\Client Server Concepts\\csc_first\\clientStorage\\";

                // Ensure the directory separator is correct
                if (!saveDirectory.empty() && saveDirectory.back() != '\\') {
                    saveDirectory += '\\';
                }

                // Save the file with the received name in the specified directory
                std::ofstream outputFile(saveDirectory + fileName, std::ios::binary);
                if (outputFile.is_open()) {
                    outputFile.write(buffer, bytesReceivedText);
                    std::cout << "File received and saved as: " << saveDirectory + fileName << std::endl;
                } else {
                    std::cerr << "Failed to save the file." << std::endl;
                }
            } else {
                std::cerr << "Failed to receive file name." << std::endl;
            }
        } else {
            std::cerr << "Failed to receive file content." << std::endl;
        }
    }


    void sendFile(const std::string& filePath) {
        // Open the file for reading
        std::ifstream fileToSend(filePath, std::ios::binary);
        if (!fileToSend.is_open()) {
            std::cerr << "Failed to open the file: " << filePath << std::endl;
            return;
        }

        // Get the file size
        fileToSend.seekg(0, std::ios::end);
        int fileSize = static_cast<int>(fileToSend.tellg());
        fileToSend.seekg(0, std::ios::beg);

        // Send the PUT command to the server
        std::string putCommand = "PUT " + filePath;
        send(clientSocket, putCommand.c_str(), (int)putCommand.length(), 0);

        // Send the file size to the server
        send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

        // Send the file data to the server
        char* buffer = new char[fileSize];
        fileToSend.read(buffer, fileSize);
        send(clientSocket, buffer, fileSize, 0);

        delete[] buffer;
        fileToSend.close();
        receiveResponse();
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
    }

    client.cleanup();

    return 0;
}

# Client-server system
This code represents a simple client-server architecture using Winsock for communication. The communication is based on a text-based protocol where the client sends commands to the server, and the server responds accordingly. Here's a brief overview of the communication protocol

------------

## Server Side:
### Server Initialization
####  Winsock Initialization:
The server initializes Winsock using `WSAStartup`.
####  Socket Creation and Binding:
A socket is created using `socket(AF_INET, SOCK_STREAM, 0)`.
The server sets up its address structure `(sockaddr_in)` with the specified port and binds the socket using `bind`.
####  Listening for Connections:
The server starts listening for incoming connections using `listen`.
####  Accepting Connections:
The server enters a loop to accept incoming connections using `accept`.
### Command Handling
####  Receiving and Parsing Commands:
Inside the `acceptConnection` loop, the server receives a command from the client using `recv` into a buffer.
The received command is then parsed.
####  Command Execution:
The server identifies the type of command (e.g., LIST, DELETE, GET, etc.) by inspecting the received buffer.
Based on the command type, the server invokes corresponding handler functions.

------------

## Client side:
### Client Interaction
#### Command Input:
The client prompts the user to enter a command (e.g., "GET file.txt," "LIST," "EXIT").
#### Command Transmission:
The entered command is sent to the server as a string using `send`.
####  Server Response Handling:
After sending a command, the client waits for and receives a response from the server using `recv`.
The response is displayed to the user.
####  User Interaction Loop:
The client enters a loop where the user can continue entering commands until choosing to exit.

------------

## Command Details:
####  LIST Command Handling:
The client sends "LIST" to request a list of files.
The server responds with the list of files in the server directory (`serverStorage`).
The client displays the list of the files in the command line.
####  DELETE Command Handling:
The client sends "DELETE filename" to request file deletion.
The server deletes the file from its directory (`serverStorage`).
The server responds with success or failure (and describes the reason why failure occurred, for example - wrong file name).
The client displays the confirmation response in the command line.
####  GET Command Handling:
The client sends a "GET filename" to save a specific file.
The server responds with the content of the specified file.
The client saves the content in the new file in `clientStorage` with the same name as on the server.
The client displays the confirmation in the command line.
####  PUT Command Handling:
The client sends "PUT filename" to request file copying to the server from the local directory (`clientStorage`).
The server saves the file in the server directory (`serverStorage`).
The server responds with a success or failure message (and describes the reason why the failure occurred, for example - the file already exists).
The client displays the confirmation response in the command line.
####  INFO Command Handling:
The client sends "INFO filename" to request file information.
The server responds with details about the specified file.
The client displays the info in the command line.
####  EXIT Command Handling:
The client sends "EXIT" to gracefully terminate the connection.
The server acknowledges the request, and the client exits the loop.

------------

## Inreraction
Ensure the server is running before the client tries to connect. The client always starts the communication by sending a command, and the server listens and responds. After issuing a command, the client waits for the server's response before moving on to the next command. 

## Server set-up information:
IP: 127.0.0.1 (localhost) 
Port: 12345
Client and server storages: must be changed to your storages with the files (in the provided code variables `serverStorage` and `clientStorage`)

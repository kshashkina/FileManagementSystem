# Client-Server File Management System
This project implements a simple client-server architecture for file management using Winsock for communication. The communication is text-based, with the client sending commands to the server, and the server responding accordingly.
## Server Side
### Server Initialization
- Winsock Initialization:
Initializes Winsock using `WSAStartup`.
- Socket Creation and Binding:
Creates a socket using `socket(AF_INET, SOCK_STREAM, 0)`.
Sets up the server's address structure` (sockaddr_in)` with the specified port and binds the socket using `bind`.
- Listening for Connections:
Starts listening for incoming connections using `listen`.
Accepting Connections:
Enters a loop to accept incoming connections using accept.
Creates a new thread for each accepted connection to handle communication concurrently.
### Multi-Client Handling and Threading
- Threaded Connection Handling:
Each accepted connection triggers the creation of a new thread (using `std::thread`) to handle communication independently.
Ensures that multiple clients can connect simultaneously without blocking the server.
- Thread-Safe Operations:
Utilizes std::mutex to ensure thread safety when accessing shared resources, such as the console output or modifying variables.
### Command Handling
- Receiving and Parsing Commands:
Inside the `acceptConnection` loop, the server receives a command from the client using `recv` into a buffer.
Parses the received command.
- Command Execution:
Identifies the command type by inspecting the received buffer.
Invokes corresponding handler functions.
### Byte-Level Details
- Data Transmission Protocol:
All communication between the client and server is text-based.
Commands and responses are transmitted as strings.
The server handles each command by processing the received string.
- Buffer Size:
The server and client use a buffer size of 1024 bytes for sending and receiving data.
Commands and responses are transmitted in chunks if they exceed the buffer size.
- Transmission Quantities:
The server sends responses with a maximum size of 1024 bytes to fit the expected data.
Commands and responses are transmitted in chunks if they exceed the buffer size.
## Client Side
### Client Interaction
- Command Input:
Prompts the user to enter a command (e.g., "GET file.txt," "LIST," "EXIT").
- Command Transmission:
Sends the entered command to the server as a string using `send`.
- Server Response Handling:
After sending a command, the client waits for and receives a response from the server using `recv`.
Displays the response to the user.
- User Interaction Loop:
The client enters a loop where the user can continue entering commands until choosing to exit.
### Byte-Level Details
- Data Transmission Protocol:
Similar to the server, the client communicates with the server using text-based commands and responses.
Commands and responses are transmitted as strings.
- Buffer Size:
The client uses a buffer size of 1024 bytes for sending and receiving data.
Commands and responses are transmitted in chunks if they exceed the buffer size.
## Command Details
#### LIST Command Handling:
Client Request: The "LIST" command, consisting of 4 bytes, is sent to the server.

Server Response: The server sends the list of files as a string. The total bytes sent depend on the number and length of filenames in the directory. Each response message is capped at 1024 bytes, including newline characters for separation.
#### NAME Command Handling:
Client Request: The command "NAME name" includes the command (5 bytes) plus the name length. The total bytes depend on the name's length.

#### DELETE Command Handling:
Client Request: The command "DELETE filename" includes the command (7 bytes) plus the filename length. The total bytes depend on the filename's length.

Server Response: The response is a success or failure message, typically not exceeding 50 bytes, including protocol overhead.
#### GET Command Handling:
Client Request: "GET filename" includes the command (4 bytes) plus the filename length.

Server Response: First, the server sends the file size (as an integer, 4 bytes), followed by the file content in chunks of up to 1024 bytes each. The filename is sent separately, with its length depending on the name size.
#### PUT Command Handling:
Client Request: Begins with "PUT filename" (4 bytes for the command, plus the filename length), followed by the file size (4 bytes) and the file data in chunks of up to 1024 bytes.

Server Response: A confirmation message, usually under 50 bytes, indicating success or failure.
#### INFO Command Handling:
Client Request: "INFO filename" consists of the command (5 bytes) plus the filename length.

Server Response: The server sends file details including path, size, and last modified timestamp. This response typically ranges from 50 to 200 bytes, depending on the file path and timestamp length.
#### EXIT Command Handling:
Client sends "EXIT" to gracefully terminate the connection. Consists of 4 bytes. 

Server acknowledges the request, and the client exits the loop.
## Interaction
Ensure the server is running before the client tries to connect. The client initiates communication by sending a command, and the server listens and responds. After issuing a command, the client waits for the server's response before proceeding to the next command.


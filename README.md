# Client-server system
This code represents a simple client-server architecture using Winsock for communication. The communication is based on a text-based protocol where the client sends commands to the server, and the server responds accordingly. Here's a brief overview of the communication protocol
## Server Side:
### Server Initialization
**Winsock Initialization:**
The server initializes Winsock using `WSAStartup`.
**Socket Creation and Binding:**
A socket is created using `socket(AF_INET, SOCK_STREAM, 0)`.
The server sets up its address structure `(sockaddr_in)` with the specified port and binds the socket using `bind`.
**Listening for Connections:**
The server starts listening for incoming connections using `listen`.
**Accepting Connections:**
The server enters a loop to accept incoming connections using `accept`.
### Command Handling
**Receiving and Parsing Commands:**
Inside the `acceptConnection` loop, the server receives a command from the client using `recv` into a buffer.
The received command is then parsed.
**Command Execution:**
The server identifies the type of command (e.g., LIST, DELETE, GET, etc.) by inspecting the received buffer.
Based on the command type, the server invokes corresponding handler functions.
### Specific Command Handling:
**LIST Command Handling:
**
The server generates a list of files in the server storage directory.
The list is sent back to the client as a response.
**DELETE Command Handling:
**
The server attempts to delete the specified file.
A success or failure message is sent back to the client as a response.
**GET Command Handling:
**
The server reads the content of the specified file.
The file content is sent back to the client as a response.
**PUT Command Handling:
**
The server creates a new file on the server.
**INFO Command Handling:
**
The server retrieves information about the specified file (e.g., path, size, last modified time).
The file information is sent back to the client as a response.
**EXIT Command Handling:
**
The server acknowledges the client's request to exit.
The connection is closed, and the server continues to the next iteration of the `acceptConnection` loop.
## Client side:
### Client Interaction
**Command Input:
**
The client prompts the user to enter a command (e.g., "GET file.txt," "LIST," "EXIT").
**Command Transmission:
**
The entered command is sent to the server as a string using `send`.
**Server Response Handling:**
After sending a command, the client waits for and receives a response from the server using `recv`.
The response is displayed to the user.
**User Interaction Loop:
**
The client enters a loop where the user can continue entering commands until choosing to exit.
### Command Details:
**LIST Command Handling:
**
The client sends "LIST" to request a list of files.
The server responds with the list of files.
**DELETE Command Handling:
**
The client sends "DELETE filename" to request file deletion.
The server responds with success or failure.
**GET Command Handling:
**
The client sends "GET filename" to request file content.
The server responds with the content of the specified file.
**PUT Command Handling:
**
The client sends "PUT filename" to request file creation.
The server responds with a success or failure message.
**INFO Command Handling:
**
The client sends "INFO filename" to request file information.
The server responds with details about the specified file.
**EXIT Command Handling:
**
The client sends "EXIT" to gracefully terminate the connection.
The server acknowledges the request, and the client exits the loop.

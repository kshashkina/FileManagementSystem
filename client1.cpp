#include "Client.h"

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

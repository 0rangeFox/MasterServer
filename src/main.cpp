#include <cstdio>
#include <iostream>

#define _WIN32_WINNT _WIN32_WINNT_WIN8 // Windows 8.0
#include <ws2tcpip.h>

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 19000

int main() {
    // Initialize the win sockets.
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        std::cout << "[Error: " << WSAGetLastError() << "] Can't initialize WinSock2! Shutting down the server." << std::endl;
        return 1;
    }

    // Create the server socket
    // https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "[Error: " << WSAGetLastError() << "] Can't create an socket from WinSock2! Shutting down the server." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the IP Address and port to a socket
    sockaddr_in server {};
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);
    server.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(serverSocket, (SOCKADDR *) &server, sizeof(server)) == SOCKET_ERROR) {
        std::cout << "[Error: " << WSAGetLastError() << "] Failed to bind the socket! Shutting down the server." << std::endl;
        WSACleanup();
        return 1;
    }

    // Tell to WinSock2, the socket is for listening
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "[Error: " << WSAGetLastError() << "] Failed to listen the socket! Shutting down the server." << std::endl;
        WSACleanup();
        return 1;
    }

    // Wait for a connection
    sockaddr_in client {};
    int clientSize = sizeof(client);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*) &client, &clientSize);

    char host[NI_MAXHOST]; // Client's remote name
    char service[NI_MAXSERV]; // Service (ex. Port) the client is connected on

    // ZeroMemory is same as memset(host, 0, NI_MAXHOST);
    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366920(v=vs.85)
    ZeroMemory(host, NI_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);

    // https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getnameinfo
    if (getnameinfo((SOCKADDR *) &client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        std::cout << "[" << host << "] Client connected on port " << service << "." << std::endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        std::cout << "[" << host << "] Client connected on port " << ntohs(client.sin_port) << "." << std::endl;
    }

    // Close the server listening socket
    if (closesocket(serverSocket) == SOCKET_ERROR) {
        std::cout << "[Error: " << WSAGetLastError() << "] Failed to close the server socket! Shutting down the server." << std::endl;
        WSACleanup();
        return 1;
    }

    // While loop, accept the connection and echo message back to client
    char buffer[DEFAULT_BUFLEN];

    while (true) {
        ZeroMemory(buffer, DEFAULT_BUFLEN);

        // Wait for the client send the data
        int bytesReceived = recv(clientSocket, buffer, DEFAULT_BUFLEN, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cout << "Error in recv()! Shutting down the server." << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "The client disconnected." << std::endl;
            break;
        }

        // Send message back to client
        send(clientSocket, buffer, bytesReceived + 1, 0);
    }

    // Close the client socket
    if (closesocket(clientSocket) == SOCKET_ERROR) {
        std::cout << "[Error: " << WSAGetLastError() << "] Failed to close the client socket! Shutting down the server." << std::endl;
        WSACleanup();
        return 1;
    }

    // Shutdown the WinSock2
    WSACleanup();

    return 0;
}

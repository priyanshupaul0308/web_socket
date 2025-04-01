#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib") // Ensure Winsock library is linked (for MSVC users)

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    cout << "Server program started" << endl;

    // Create a listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    // Server address configuration
    int port = 12345;
    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr)); // Zero out struct (C++98 compatibility)
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available network interface

    // Bind the socket
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port " << port << endl;

    // Accept an incoming connection
    sockaddr_in clientaddr;
    int clientSize = sizeof(clientaddr);
    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientaddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Accept failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Receive data
    char buffer[4096] = {0};
    int bytesrecv = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesrecv == SOCKET_ERROR || bytesrecv == 0) {
        cout << "Receive failed or connection closed" << endl;
    } else {
        string message(buffer, bytesrecv);
        cout << "Message from client: " << message << endl;
    }

    // Cleanup
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}


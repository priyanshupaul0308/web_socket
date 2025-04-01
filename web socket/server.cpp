#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

mutex clientsMutex; // Mutex to synchronize access to the client list
vector<SOCKET> clients; // Global client list

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket) {
    cout << "Client connected!" << endl;

    char buffer[4096];

    while (true) {
        int bytesrecv = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesrecv <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        buffer[bytesrecv] = '\0'; // Null terminate the received message
        string message(buffer);

        cout << "Message from client: " << message << endl;

        // Send message to all clients except sender
        {
            lock_guard<mutex> lock(clientsMutex);
            cout << "Broadcasting to " << clients.size() << " clients." << endl;  // Log number of clients
            for (SOCKET client : clients) {
                if (client != clientSocket) {
                    int bytesSent = send(client, message.c_str(), message.length(), 0);
                    if (bytesSent <= 0) {
                        cout << "Failed to send message to client" << endl;
                    } else {
                        cout << "Message sent to client" << endl;
                    }
                }
            }
        }
    }

    // Remove client from list
    {
        lock_guard<mutex> lock(clientsMutex);
        cout << "Removing client from the list." << endl;  // Log client removal
        clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }

    closesocket(clientSocket);
}

int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    cout << "Server program" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    int port = 12345;
    sockaddr_in serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Bind failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server has started listening on port " << port << endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client socket" << endl;
            continue; // Keep server running even if one accept fails
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        thread t1(InteractWithClient, clientSocket);
        t1.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

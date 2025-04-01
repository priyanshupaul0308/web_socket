#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void sendmsg(SOCKET s) {
    cout << "Enter your chat name: ";
    string name;
    getline(cin >> ws, name);  // ✅ Fix: Ensures no leading whitespace issues

    string message;
    while (true) {
        getline(cin, message);
        string msg = name + " : " + message;

        int bytesSent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            cout << "Error sending message. Connection lost." << endl;
            break;
        }

        if (message == "quit") {
            cout << "Stopping the application." << endl;
            break;
        }
    }
}


void recivemsg(SOCKET s) {
    char buffer[4096];
    int recvlength;
    
    while (true) {
        recvlength = recv(s, buffer, sizeof(buffer), 0);
        if (recvlength <= 0) {
            cout << "Disconnected from the server." << endl;
            break;
        } else {
            string msg(buffer, recvlength);
            cout << msg << endl;
        }
    }
}

int main() {
    if (!initialize()) {
        cout << "Failed to initialize Winsock." << endl;
        return 1;
    }

    cout << "Client program started." << endl;

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Invalid socket created. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    int port = 12345;
    string serveraddr2 = "127.0.0.1";

    sockaddr_in serveraddr{};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    if (inet_pton(AF_INET, serveraddr2.c_str(), &serveraddr.sin_addr) != 1) {
        cout << "Invalid address/Address conversion failed." << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Failed to connect to server. Error: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Successfully connected to the server." << endl;

    thread senderthread(sendmsg, s);
    thread receiverthread(recivemsg, s);

    senderthread.join();
    receiverthread.join();

    closesocket(s);
    WSACleanup();
    return 0;
}
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr int BUFFER_SIZE = 512;
constexpr char DEFAULT_PORT[] = "27015";
constexpr char SERVER_IP[] = "127.0.0.1";

class NetworkClient {
public:
    NetworkClient() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Winsock initialization failed.");
        }
    }

    ~NetworkClient() {
        WSACleanup();
    }

    void run() {
        try {
            auto socket = connectToServer();
            interactWithServer(socket);
            closesocket(socket);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            exit(1);
        }
    }

private:
    SOCKET connectToServer() {
        struct addrinfo hints{}, *result;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        if (getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result) != 0) {
            throw std::runtime_error("Failed to get address info for the server.");
        }

        SOCKET connSocket = INVALID_SOCKET;
        for (auto ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            connSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (connSocket == INVALID_SOCKET) {
                freeaddrinfo(result);
                throw std::runtime_error("Unable to create socket.");
            }

            if (connect(connSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR) {
                closesocket(connSocket);
                continue;
            }
            freeaddrinfo(result);
            return connSocket;
        }

        freeaddrinfo(result);
        throw std::runtime_error("Failed to connect to the server.");
    }

    void interactWithServer(SOCKET socket) {
        std::string input;
        char response[BUFFER_SIZE];
        while (true) {
            std::cout << "Enter message: ";
            getline(std::cin, input);

            if (send(socket, input.c_str(), static_cast<int>(input.length()), 0) == SOCKET_ERROR) {
                throw std::runtime_error("Failed to send data to server.");
            }

            int bytesReceived = recv(socket, response, BUFFER_SIZE, 0);
            if (bytesReceived > 0) {
                response[bytesReceived] = '\0';
                std::cout << "Server replied: " << response << std::endl;
            } else if (bytesReceived == 0) {
                std::cout << "Server closed the connection." << std::endl;
                break;
            } else {
                throw std::runtime_error("Failed to receive data from server.");
            }
        }
    }
};

int main() {
    try {
        NetworkClient client;
        client.run();
    } catch (const std::exception& ex) {
        std::cerr << "An exception occurred: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

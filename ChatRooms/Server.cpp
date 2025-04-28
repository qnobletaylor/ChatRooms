#include <iostream>
#include <winsock2.h>
#include <thread>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void handleClient(SOCKET clientSocket) {
	char buffer[1024];
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0) {
			std::string msg(buffer, bytesReceived);
			std::cout << "Client says: " << msg << "\n";

			// Echo back
			send(clientSocket, buffer, bytesReceived, 0);
		}
		else if (bytesReceived == 0) {
			std::cout << "Client disconnected gracefully.\n";
			break;
		}
		else {
			std::cerr << "recv failed.\n";
			break;
		}
	}

	closesocket(clientSocket);
}


int main() {
	WSADATA wsaData;
	SOCKET serverSocket, clientSocket;
	sockaddr_in serverAddr{}, clientAddr{};
	char buffer[1024];
	int clientSize;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return 1;
	}

	// Create socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed.\n";
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on any interface
	serverAddr.sin_port = htons(54000);      // Port number serverAddr.sin_port = htons(54000);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	// Bind
	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// Listen
	listen(serverSocket, SOMAXCONN);
	std::cout << "Server listening on port 54000...\n";

	while (true) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket != INVALID_SOCKET) {
			std::cout << "Client connected.\n";
			std::thread clientThread(handleClient, clientSocket);
			clientThread.detach(); // let it run independently
		}
	}

	// Cleanup
	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}

#include <iostream>
#include <string>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <format>
#include <ws2tcpip.h>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
import User;

std::vector<User> CLIENTS{}; // Global variable for connected users

/**
 * Gets the current time and formats it into a string of "hour:min:sec"
 *
 * I will likely move this function to client side. Also need to make it a bit more succint.
 *  */
std::string getTime();

/**
 * Asks a client for user information and saves their username alongside that client's socket
 *  */
User getUser(SOCKET clientSocket);

void handleClient(SOCKET clientSocket);


int main() {
	WSADATA wsaData;
	SOCKET serverSocket, clientSocket;
	sockaddr_in serverAddr{}, clientAddr{};

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
	serverAddr.sin_port = htons(54000);      // Port number
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
		clientSocket = accept(serverSocket, nullptr, nullptr);

		if (clientSocket != INVALID_SOCKET) {

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

std::string getTime() {
	time_t currentTime;
	struct tm localTime;

	currentTime = std::time(nullptr);
	localtime_s(&localTime, &currentTime);
	std::string min = ((localTime.tm_min / 10) == 0) ? ("0" + std::to_string(localTime.tm_min)) : std::to_string(localTime.tm_min);
	std::string hour = ((localTime.tm_hour % 12) / 10) == 0 ? ("0" + std::to_string(localTime.tm_hour % 12)) : std::to_string(localTime.tm_hour % 12);
	std::string sec = ((localTime.tm_sec / 10) == 0) ? ("0" + std::to_string(localTime.tm_sec)) : std::to_string(localTime.tm_sec);
	std::string timeStamp = std::format("{}:{}:{}", hour, min, sec);

	return timeStamp;
}

User getUser(SOCKET clientSocket) {
	char buffer[1024];
	int bytesReceived{};
	ZeroMemory(buffer, sizeof(buffer));
	std::string prompt = "Server: Enter a username > ";

	do {
		send(clientSocket, prompt.c_str(), prompt.size(), 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	} while (bytesReceived <= 0);

	std::string msg(buffer, bytesReceived);

	return User(msg, clientSocket);
}

void handleClient(SOCKET clientSocket) {
	char buffer[1024];
	ZeroMemory(buffer, sizeof(buffer));

	User user = getUser(clientSocket);
	std::cout << std::format("{} Connected to server.\n", user.username);

	CLIENTS.push_back(user);

	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			Message msg{ std::string(buffer, bytesReceived), getTime() };

			std::string output = std::format("<{}> {}: {}\n", msg.timeStamp, user.username, msg.message);
			std::cout << output;

			// Echo back
			for (const auto& client : CLIENTS) { // Does not send the message back to the user that sent it
				if (clientSocket != client.clientSocket) send(client.clientSocket, output.c_str(), output.size(), 0);
			}

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
#include <iostream>
#include <string>
#include <regex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

// Function to receive messages in a separate thread
void receiveMessages(SOCKET sock);

/**
 * Asks the user for an IP address and a port # for which to connect to the server with.
 *  */
std::pair<std::string, int> getIP();

int main() {
	WSADATA wsaData;
	SOCKET sock;
	sockaddr_in serverAddr{};

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return 1;
	}

	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Socket creation failed.\n";
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;



	std::pair<std::string, int> ipAndPort = getIP();

	inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr);
	serverAddr.sin_port = htons(ipAndPort.second);

	// Connect to server
	while (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed.\n";

		std::pair<std::string, int> ipAndPort = getIP();

		inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr);
		serverAddr.sin_port = htons(ipAndPort.second);
	}

	std::cout << "Connected to server. Type messages, or 'exit' to quit.\n";
	// Start the receiving thread
	std::thread receiver(receiveMessages, sock);
	receiver.detach();

	// Send loop
	std::string input{};
	while (true) {
		std::getline(std::cin, input);
		if (input == "exit") break;

		send(sock, input.c_str(), input.size(), 0);
	}

	// Cleanup
	closesocket(sock);
	WSACleanup();
	return 0;
}

void receiveMessages(SOCKET sock) {
	char buffer[1024];
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0) {
			std::cout << std::string(buffer, bytesReceived);
		}
		else if (bytesReceived == 0) {
			std::cout << "Server disconnected.\n";
			break;
		}
		else {
			std::cerr << "recv failed.\n";
			break;
		}
	}
}

std::pair<std::string, int> getIP() {
	std::string ipAndPort{};
	std::cout << "Enter IP address and Port # > ";
	std::getline(std::cin, ipAndPort);

	std::regex regex("[0-9.]+:[0-9]+");

	while (!std::regex_match(ipAndPort, regex)) {
		std::cout << std::format("You entered: {}, please try again formatted as 127.0.0.1:54000\n> ", ipAndPort);
		std::getline(std::cin, ipAndPort);
	}

	std::string::size_type split = ipAndPort.find_first_of(':');

	return std::pair<std::string, int>(ipAndPort.substr(0, split), std::stoi(ipAndPort.substr(split + 1)));
}
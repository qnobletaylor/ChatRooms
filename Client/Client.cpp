#include <iostream>
#include <string>
#include <regex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>
#include <thread>
#include "curses.h"
#pragma comment(lib, "Ws2_32.lib")

// Function to receive messages in a separate thread
void receiveMessages(SOCKET sock);

/**
 * Asks the user for an IP address and a port # for which to connect to the server with.
 *  */
std::pair<std::string, int> getIP();

bool validateIPandPort(std::string input);

int main(int argc, char* argv[]) {
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

	std::pair<std::string, int> ipAndPort;


	// ncurses start
	initscr();

	WINDOW* textOutput = newwin(20, 100, 0, 0); // For displaying messages
	WINDOW* textInput = newwin(10, 100, 20, 0); // For typing messages
	refresh();
	box(textInput, 0, 0);
	box(textOutput, 0, 0);
	wrefresh(textInput);
	wrefresh(textOutput);

	if (argc >= 2 && validateIPandPort(std::format("{}:{}", argv[1], argv[2]))) {
		ipAndPort.first = argv[1];
		ipAndPort.second = std::stoi(argv[2]);
	}
	else {
		// Prompting for IP and Port
		ipAndPort = getIP();
	}

	// Open firewall port on remote desktop

	inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr); // set IP
	serverAddr.sin_port = htons(ipAndPort.second); // set Port

	// Connect to server, try to get ipAndPort again if connection fails
	while (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed.\n";

		std::pair<std::string, int> ipAndPort = getIP();

		inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr);
		serverAddr.sin_port = htons(ipAndPort.second);
	}

	std::cout << "Connected to server. Type messages, /help for info, or /exit to quit.\n";
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
	endwin();
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

	closesocket(sock);
	WSACleanup();
}

std::pair<std::string, int> getIP() {
	std::string ipAndPort{};
	std::cout << "Enter IP address and Port # > ";
	std::getline(std::cin, ipAndPort);

	while (!validateIPandPort(ipAndPort)) {
		std::cout << std::format("You entered: {}, please try again formatted as 127.0.0.1:54000\n> ", ipAndPort);
		std::getline(std::cin, ipAndPort);
	}

	std::string::size_type split = ipAndPort.find(':');

	return std::pair<std::string, int>(ipAndPort.substr(0, split), std::stoi(ipAndPort.substr(split + 1)));
}

bool validateIPandPort(std::string input) {
	// This is ai generated
	std::regex regex{ R"~(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?):(?:6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3}|0)$)~" };

	return std::regex_match(input, regex);
}

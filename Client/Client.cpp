/**
 * @file Client.cpp
 * @author Quinlin Taylor (CSC-284)
 * @date 05/14/2025
 * @brief This file contains the startup main function for the client side program which allows the user to connect to the server.
 */

#include <iostream>
#include <string>
#include <regex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>
#include <thread>
#include "tui.h"
#pragma comment(lib, "Ws2_32.lib")


 /**
 * @brief If an IP and Port # are not given as command line arguments at runtime then the server prompts for an IP and Port.
 *
 * @return std::pair<std::string, int> a pair of an IP(string) and port(int) so long as they are valid.
 */
std::pair<std::string, int> getIP();

/**
 * @brief Takes a string and checks it against a regex which will check if the string is formatted as a proper IP and Port #, ex. 0.0.0.0:0-65535.
 *
 * @param input , the string to validate.
 * @return true if the port and ip # are passed as a valid IP and port #
 * @return false if the port and ip are out of range or input invalidly
 */
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

	std::pair<std::string, int> ipAndPort; // Store ip and port

	// Checking for ip and port passed as command line args
	if (argc >= 2 && validateIPandPort(std::format("{}:{}", argv[1], argv[2]))) {
		ipAndPort.first = argv[1];
		ipAndPort.second = std::stoi(argv[2]);
	}
	else {
		ipAndPort = getIP(); // Prompting for IP and Port
	}

	inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr); // set IP
	serverAddr.sin_port = htons(ipAndPort.second); // set Port

	// Connect to server, try to get ipAndPort again if connection fails
	while (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Connection failed.\n";

		std::pair<std::string, int> ipAndPort = getIP();

		inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr);
		serverAddr.sin_port = htons(ipAndPort.second);
	}

	// Create and start text UI with ncurses
	tui clientUI(sock);
	clientUI.drawUI();

	std::cout << "Exiting client...\n";
	// Cleanup
	closesocket(sock);
	WSACleanup();
	return 0;
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
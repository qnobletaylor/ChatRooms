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

void printToOutput(std::string msg);

bool validateIPandPort(std::string input);

std::string getInput();

WINDOW* outputWin;
WINDOW* inputWin;

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
	/*raw();*/
	//cbreak();
	//noecho();

	// Borders for input and output windows
	WINDOW* outputBorder = newwin(25, getmaxx(stdscr) - 20, 0, 0);
	WINDOW* inputBorder = newwin(5, getmaxx(outputBorder), getmaxy(outputBorder), 0);

	outputWin = newwin(23, getmaxx(outputBorder) - 2, 1, 1); // For displaying messages
	inputWin = newwin(3, getmaxx(inputBorder) - 2, getbegy(inputBorder) + 1, 1); // For typing messages
	scrollok(outputWin, true);
	refresh();
	box(outputBorder, 0, 0);
	box(inputBorder, 0, 0);
	wrefresh(outputBorder);
	wrefresh(inputBorder);
	wrefresh(outputWin);
	wrefresh(inputWin);

	if (argc >= 2 && validateIPandPort(std::format("{}:{}", argv[1], argv[2]))) {
		ipAndPort.first = argv[1];
		ipAndPort.second = std::stoi(argv[2]);
	}
	else {
		// Prompting for IP and Port
		ipAndPort = getIP();
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

	printToOutput("Connected to server. Type messages, /help for info, or /exit to quit.\n");
	// Start the receiving thread
	std::thread receiver(receiveMessages, sock);
	receiver.detach();

	// Send loop
	std::string input{};
	while (true) {
		input = getInput();
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
			std::string msg(buffer, bytesReceived);
			printToOutput(msg);
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
	printToOutput("Enter IP address and Port #...\n");
	std::string ipAndPort = getInput();

	while (!validateIPandPort(ipAndPort)) {
		std::string error = std::format("You entered: {}, please try again formatted as 127.0.0.1:54000\n", ipAndPort);
		printToOutput(error);
		ipAndPort = getInput();
	}

	std::string::size_type split = ipAndPort.find(':');

	return std::pair<std::string, int>(ipAndPort.substr(0, split), std::stoi(ipAndPort.substr(split + 1)));
}

bool validateIPandPort(std::string input) {
	// This is ai generated
	std::regex regex{ R"~(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?):(?:6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3}|0)$)~" };

	return std::regex_match(input, regex);
}

void printToOutput(std::string msg) {
	std::cout << msg << "\n";
	waddstr(outputWin, msg.c_str());
	wrefresh(outputWin);
	wmove(inputWin, 0, 0);
	wrefresh(inputWin);
}

std::string getInput() {
	char inputStr[1000];
	wgetstr(inputWin, inputStr);
	wclear(inputWin);
	wmove(inputWin, 0, 0);


	return std::string(inputStr);
}

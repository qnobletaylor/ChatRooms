#include <iostream>
#include <string>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <format>
#include <ws2tcpip.h>
#include <ctime>
#include <regex>
#include <mutex>
#include "Room.h"
#include "User.h"
#pragma comment(lib, "ws2_32.lib")



/**
 * TODO
 *
 *		> History <
 *		- Create a form of history for all sent messages, so that when new users connect then on joining the user can get
 *		a brief history of chat.
 *		- Cap the size of the number of messages stored, so the oldest messages will be tossed out
 *		- This could be stored in the form of a hashmap<User, List<String>> So you could also query the server to give all messages sent by a specific user.
 *		- Or simply basic string array[], and have a function which keeps track of size, probably better to use a list so that removing and replacing
 *		nodes will be O(1)
 *
 *		> Logging <
 *		- Add server side console output for when users use commands and what that user is doing when executing cmds
 *
 *		> User commands <
 *		- Add command to list all users in a room, list users in server, possibly send direct msg to another user.
 *
 *		> server commands <
 *		- Add server only commands, such as kick user, move user, move all users. Just some ideas.
 *
 *		> Audio ** <
 *		-
 */

std::mutex usernameListMutex;
std::set<std::string> usernameList{}; // Global variable for connected users
std::mutex roomListMutex;
std::map<std::string, Room> roomList{ {"Lobby", Room("Lobby")} }; // Initial room which all users start in will be known as the lobby
std::string help{
	"\t\t** List of Commands **\n\n"
	"CREATE_ROOM <name>\n\tCreate a new room and move to it.\n"
	"JOIN_ROOM <name>\n\tMove to another existing room.\n"
	"LIST_ROOMS\n\tDisplay a list of all rooms on the server and how many users are in each room.\n"
	"EXIT\n\tExit the server.\n"
	"HELP\n\tDisplay this list of commands.\n\n"
	"Instructions\n\tTo use a command simply type a '/' followed by the command and any necessary arguments\n"
	"\tFor example to create a new room use /CREATE_ROOM Study\n"
};

int getPort();

/**
 * Gets the current time and formats it into a string of "hour:min:sec"
 *
 * I will likely move this function to client side. Also need to make it a bit more succint..
 */
std::string getTime();

/**
 * Asks a client for user information and saves their username alongside that client's socket.
 */
User createUser(SOCKET clientSocket);

/**
 * Accepts new client connections and starts a new thread using handleClient with the clientSocket which connected.
 */
void acceptClients(SOCKET serverSocket);

/**
 * Handles receiving data from a client which connects to the server.
 */
void handleClient(SOCKET clientSocket);

/**
 * Broadcast a mesage to a room, given a room name and the message.
 */
void broadcastToRoom(const std::string& roomName, const std::string& msg);

/**
 * Broadcasts a message to every room on the server so long as it's not empty.
 */
void broadcastToServer(const std::string& msg);

void userCommand(const std::string& cmd, User& user);


int main() {
	WSADATA wsaData;
	SOCKET serverSocket;
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

	u_short portNum = getPort(); // Prompt for port number
	serverAddr.sin_port = htons(portNum);
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
	std::cout << "Server listening on port " << portNum << " ...\n";

	std::thread acceptThread(acceptClients, serverSocket);
	acceptThread.detach();

	std::string input{};
	while (true) {
		std::getline(std::cin, input);
		if (input == "exit") break;

		input = std::format("<{}>[SERVER]: {}\n", getTime(), input);

		broadcastToServer(input);
	}

	// Cleanup
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}

int getPort() {
	std::string port{};
	std::cout << "Enter a port # > ";
	std::getline(std::cin, port);

	// This is ai generated
	std::regex regex{ R"~(^(0|[1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)~" };

	while (!std::regex_match(port, regex)) {
		std::cout << std::format("You entered: {}, please try again with a valid port 0-65535\n> ", port);
		std::getline(std::cin, port);
	}

	return std::stoi(port);
}

std::string getTime() {
	time_t currentTime;
	struct tm localTime;

	currentTime = std::time(nullptr);
	localtime_s(&localTime, &currentTime);

	std::string min = ((localTime.tm_min / 10) == 0) ? ("0" + std::to_string(localTime.tm_min)) : std::to_string(localTime.tm_min);
	std::string hour = ((localTime.tm_hour % 12) / 10) == 0 ? ("0" + std::to_string(localTime.tm_hour % 12)) : std::to_string((localTime.tm_hour % 12));
	std::string sec = ((localTime.tm_sec / 10) == 0) ? ("0" + std::to_string(localTime.tm_sec)) : std::to_string(localTime.tm_sec);
	std::string timeStamp = std::format("{}:{}:{}", hour, min, sec);

	return timeStamp;
}

User createUser(SOCKET clientSocket) {
	char buffer[1024];
	int bytesReceived{};
	ZeroMemory(buffer, sizeof(buffer));
	std::string prompt = "Server: Enter a username > ";

	do {
		send(clientSocket, prompt.c_str(), prompt.size(), 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	} while (bytesReceived <= 0);

	std::string msg(buffer, bytesReceived);

	std::string takenUserPrompt = (msg + " is already taken, please choose another name > ");

	std::lock_guard<std::mutex> guard(usernameListMutex);

	while (!usernameList.insert(msg).second) {
		send(clientSocket, takenUserPrompt.c_str(), takenUserPrompt.size(), 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived <= 0) continue;
		else msg = std::string(buffer, bytesReceived);
	}


	return User(msg, clientSocket);
}

void acceptClients(SOCKET serverSocket) {
	SOCKET clientSocket;

	while (true) {
		clientSocket = accept(serverSocket, nullptr, nullptr);

		if (clientSocket != INVALID_SOCKET) {

			std::thread clientThread(handleClient, clientSocket);
			clientThread.detach(); // let it run independently
		}
	}
}

void handleClient(SOCKET clientSocket) {
	char buffer[1024];
	ZeroMemory(buffer, sizeof(buffer));

	User user = createUser(clientSocket);

	std::cout << std::format("{} Connected to server.\n", user.username);

	roomList.at("Lobby").addUser(user);

	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			Message msg{ std::string(buffer, bytesReceived), getTime() };

			if (msg.message.at(0) == '/') userCommand(msg.message, user);
			else {
				std::string output = std::format("<{}>[{}]: {}\n", msg.timeStamp, user.username, msg.message);

				// Echo back
				broadcastToRoom(user.currentRoom, output);
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


void broadcastToRoom(const std::string& roomName, const std::string& msg) {
	std::cout << roomName << " | " << msg; // Print in server console

	for (const auto& client : roomList.at(roomName).getUsers()) { // Does not send the message back to the user that sent it
		send(client.clientSocket, msg.c_str(), msg.size(), 0);
	}
}

void broadcastToServer(const std::string& msg) {
	for (auto& room : roomList) {
		if (room.second.getUsers().empty());
		else {
			for (const auto& user : room.second.getUsers()) {
				send(user.clientSocket, msg.c_str(), msg.size(), 0);
			}
		}
	}
}

void userCommand(const std::string& msg, User& user) {
	size_t firstSpace = msg.find(' ');
	std::string cmd = msg.substr(1, firstSpace - 1);
	//Currently only takes the first word after command, expand to including multi words denoted by " "
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1) - firstSpace);


	for (auto& c : cmd) c = std::toupper(c);
	if (cmd == "HELP") {
		send(user.clientSocket, help.c_str(), help.size(), 0);
	}
	else if (cmd == "CREATE_ROOM") {

		if (!roomList.contains(param)) {
			std::string infoUser = std::format("Created and moved to {}\n", param);
			std::string infoServer = std::format("{} created new room {}\n", user.username, param);

			roomList[param] = Room(param, user);
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]);

			std::cout << infoServer;
			send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0);
		}
		else {
			std::string errorMsg{ "The room " + param + " already exists, try another name.\n" };
			send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
		}
	}
	else if (cmd == "JOIN_ROOM") {

		if (roomList.contains(param)) {
			std::string infoUser = std::format("Moved to {}, currently {} other users in this room\n", param, roomList[param].getSize());
			std::string infoServer = std::format("{} moved to {}\n", user.username, param);


			broadcastToRoom(user.currentRoom, infoServer); // Don't send to user that moved...
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]);

			send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0);
		}
		else {
			std::string errorMsg{ "The room " + param + " does not exist.\n" };
			send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
		}
	}
	else if (cmd == "LIST_ROOMS") {

		std::string rooms{ "Listing Rooms:\n" };
		for (const auto& room : roomList) {
			if (user.currentRoom == room.first) rooms += std::format("> {} [{} users]\n", room.first, room.second.getSize());
			else rooms += std::format("  {} [{} users]\n", room.first, room.second.getSize());
		}

		send(user.clientSocket, rooms.c_str(), rooms.size(), 0);
	}
	else if (cmd == "EXIT") {
		std::string infoUser{ "Leaving server..." };
		std::string infoServer{ user.username + " left the server.\n" };

		broadcastToRoom(user.currentRoom, infoServer);
		send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0);

		closesocket(user.clientSocket);
	}
	else {
		std::string errorMsg = (cmd + " is an unknown command, try /HELP for a list of commands.\n");
		send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
	}
}

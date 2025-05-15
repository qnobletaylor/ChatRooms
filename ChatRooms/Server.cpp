/**
 * @file   Server.cpp
 * @author Quinlin Taylor (CSC-284)
 * @date   5/14/2025
 * @brief  This file contains the main function as well as all functions for handling client connections to the server.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <format>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include <chrono>
#include <regex>
#include <mutex>
#include "Room.h"
#include "User.h"
#pragma comment(lib, "ws2_32.lib")

 /* GLOBALS */

/**
 * @brief a mutex for locking the global set of stored usernames.
 */
static std::mutex usernameListMutex;
/**
 * @brief a mutex for locking the global hashmap of stored rooms.
 */
static std::mutex roomListMutex;
/**
 * @brief a set which stores all the usernames of clients connected to the server.
 */
static std::set<std::string> usernameList{};
/**
 * @brief a hashmap storing all rooms created on the server by connected clients, by default a Lobby room is created on run-time.
 * The key for the map is a string representing the room name and the value is the room object itself.
 */
static std::map<std::string, Room> roomList{ {"Lobby", Room("Lobby")} };
/**
 * @brief MAN style help string, sent to clients when the /help command is received by the server.
 */
const static std::string help{
	"\t\t** List of Commands **\n\n"
	"CREATE_ROOM <name>\n\tCreate a new room and move to it.\n"
	"JOIN_ROOM <name>\n\tMove to another existing room.\n"
	"LIST_USERS <room = Lobby>\n\tLists users within a given room or if no argument is given lists all users in server.\n"
	"EXIT\n\tExit the server.\n"
	"HELP\n\tDisplay this list of commands.\n\n"
	"Instructions\n\tTo use a command simply type a '/' followed by the command and any necessary arguments\n"
	"\tFor example to create a new room use /CREATE_ROOM Study (commands are case insensitive)\n"
};

/* END GLOBALS */

/**
 * @brief If an IP and Port # are not given as command line arguments at runtime then the server prompts for an IP and Port.
 *
 * @return std::pair<std::string, int> a pair of an IP(string) and port(int) so long as they are valid.
 */
std::pair<std::string, int> getIP();


bool validateIPandPort(std::string input);

/**
 * @brief Takes a string and checks it against a regex which will check if the string is formatted as a proper IP and Port #, ex. 0.0.0.0:0-65535.
 *
 * @param input , the string to validate.
 * @return true if the port and ip # are passed as a valid IP and port #
 * @return false if the port and ip are out of range or input invalidly
 */
std::string getTime();

/**
 * @brief When a connection is made to the server this function prompts the client for a username and checks if that name is already in use.
 * If the name is not taken a User object is created storing that username, the client socket, and their current room.
 *
 * @param clientSocket the socket from which the new connection is made.
 * @return User the newly created User tied to the new client connection.
 */
User createUser(SOCKET clientSocket);

/**
 * @brief On runtime a thead gets created and this function is executed by the thread to always allow new connections to the server.
 *
 * @param serverSocket this server's socket with which to make a new connection with incoming clients.
 */
void acceptClients(SOCKET serverSocket);

/**
 * @brief For each new connection a thread is created and this function is executed to handle incoming data from the client.
 * This function also handles sending data back to all clients on the server when a message is received from the thread tied to this client.
 *
 * @param clientSocket the client's socket to receive data from.
 */
void handleClient(SOCKET clientSocket);

/**
 * @brief This function simply loops through the list of all Users in a room and sends a given message to each user in that room.
 *
 * @param roomName the name of the room to broadcast to.
 * @param msg the message getting sent to each client currently in the given room name.
 */
void broadcastToRoom(const std::string& roomName, const std::string& msg);

/**
 * @brief This function sends a message to all clients currently connected to the server.
 *
 * @param msg the message sent to all connected clients.
 */
void broadcastToServer(const std::string& msg);

/**
 * @brief This function is called when a command is sent by a User, the message is parsed into the cmd and arguments given and then depending on the cmd differing actions will occur.
 *
 * @param cmd the string received from a client which includes a command.
 * @param user the User who sent the command.
 */
void userCommand(const std::string& cmd, User& user);

/**
 * @brief Loops creates a neatly formatted string representation of all users in the server or if arguments are given, all users within a room.
 *
 * @param roomName the name of a room to return the current users, defaults to "Server" which will return all users on the server.
 * @return std::string the formatted string of users.
 */
std::string usersToString(std::string roomName = "Server");

/**
 * @brief Creates a neatly formatted string of the rooms which currently exist on the server as well as the number of users in each room.
 *
 * @param user the passed User arg will include an indicator for the room that user is currently in.
 * @return std::string the formatted list of rooms.
 */
std::string listRooms(const User& user);

/**
 * @brief This function only gets called internally when a user disconnects intentionally or when the server loses connection with the client socket.
 * The client which has disconnected will be removed from all references to that client.
 *
 * @param user the User object connected to the client which got disconnected.
 */
void removeUser(User& user);

/**
 * @brief This function is called any time that a user creates a new room or a user changes rooms. Will broadcast a serverwide message calling \link listRooms \endlink
 * which indicates for each client the updated room list as well as how many clients are in each room.
 */
void updateClientRoomList();

int main(int argc, char* argv[]) {
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

	std::pair<std::string, int> ipAndPort;

	if (argc >= 2 && validateIPandPort(std::format("{}:{}", argv[1], argv[2]))) {
		ipAndPort.first = argv[1];
		ipAndPort.second = std::stoi(argv[2]);
	}
	else {
		ipAndPort = getIP(); // Prompting for IP and Port
	}

	serverAddr.sin_port = htons(ipAndPort.second);
	inet_pton(AF_INET, ipAndPort.first.c_str(), &serverAddr.sin_addr);

	// Bind
	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed.\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// Listen
	listen(serverSocket, SOMAXCONN);
	std::cout << "Server listening on port " << ipAndPort.second << " ...\n";

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

std::string getTime() {
	struct tm tmNow;
	time_t currentTime = std::time(nullptr);
	gmtime_s(&tmNow, &currentTime); // Convert to UTC

	std::stringstream s{};

	s << std::put_time(&tmNow, "%r"); //Formats as 12-Hr Time HH:mm:ss

	return s.str();
}

User createUser(SOCKET clientSocket) {
	char buffer[1024];
	int bytesReceived{};
	ZeroMemory(buffer, sizeof(buffer));
	std::string prompt = "Server: Enter a username...\n";

	do {
		send(clientSocket, prompt.c_str(), prompt.size(), 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	} while (bytesReceived <= 0);

	std::string msg(buffer);

	std::string takenUserPrompt = (msg + " is already taken, please choose another name...\n");

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

	roomList.at("Lobby").addUser(user); // Add user to default room (Lobby)
	broadcastToRoom("Lobby", (user.username + " has joined the Lobby\n"));
	updateClientRoomList(); // Update room list on client side when new users join
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			Message msg{ std::string(buffer), getTime() };

			if (msg.message.at(0) == '/') userCommand(msg.message, user);
			else {
				std::string output = std::format("<{}>[{}]: {}\n", msg.timeStamp, user.username, msg.message);

				broadcastToRoom(user.currentRoom, output); // Echo back
			}
		}
		else if (bytesReceived == 0) {
			std::cout << "Client disconnected gracefully.\n";
			break;
		}
		else {
			std::cout << user.username << " recv error, removed from server.\n";
			removeUser(user);
			break;
		}
	}

	closesocket(clientSocket);
}


void broadcastToRoom(const std::string& roomName, const std::string& msg) {
	std::cout << roomName << " | " << msg; // Print in server console

	for (const auto& client : roomList.at(roomName).getUsers()) {// Does not send the message back to the user that sent it
		send(client.clientSocket, msg.c_str(), msg.size(), 0);
	}
}

void broadcastToServer(const std::string& msg) {
	for (const auto& room : roomList) {
		if (room.second.getUsers().empty());
		else {
			for (const auto& user : room.second.getUsers()) {
				send(user.clientSocket, msg.c_str(), msg.size(), 0);
			}
		}
	}
}

void updateClientRoomList() {
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); // temporary fix
	std::string msg{};
	for (const auto& room : roomList) {
		if (room.second.getUsers().empty());
		else {
			for (const auto& user : room.second.getUsers()) {
				msg = ("0" + listRooms(user)); // Should probably differentiate this message in another way
				send(user.clientSocket, msg.c_str(), msg.size(), 0);
			}
		}
	}
}

void userCommand(const std::string& msg, User& user) {
	size_t firstSpace = msg.find(' ');
	std::string cmd = msg.substr(1, firstSpace - 1);
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1)); // returns everything after the cmd


	for (auto& c : cmd) c = std::toupper(c);
	if (cmd == "HELP") {
		send(user.clientSocket, help.c_str(), help.size(), 0);
	}
	else if (cmd == "CREATE_ROOM") {

		if (!roomList.contains(param)) {
			std::string infoUser = std::format("Created and moved to {}\n", param);
			std::string informRoom = std::format("{} moved to {}\n", user.username, param);
			std::string infoServer = std::format("{} created new room {}\n", user.username, param);
			broadcastToRoom(user.currentRoom, informRoom); // Inform room that a user has left

			std::lock_guard<std::mutex> guard(roomListMutex);
			roomList[param] = Room(param); // Create new room
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]); // Move user to the new room

			std::cout << infoServer; // server logging

			send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0); // Inform user they've created the room and moved
			updateClientRoomList(); // Update client's roomlist
		}
		else {
			std::string errorMsg{ "The room " + param + " already exists, try another name.\n" };
			send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0); // In case of attempting to create already exisitng room
		}
	}
	else if (cmd == "JOIN_ROOM") {

		if (roomList.contains(param)) {
			std::string infoUser = std::format("Moved to {}, currently {} other users in this room\n", param, roomList[param].getSize());
			std::string infoServer = std::format("{} moved to {}\n", user.username, param);


			broadcastToRoom(user.currentRoom, infoServer); // Inform room that a user has moved to new room
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]); // move user

			send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0); // Inform that user they've moved, as well as how many people are in the room
			updateClientRoomList(); // Update client's roomlist
		}
		else {
			std::string errorMsg{ "The room " + param + " does not exist.\n" };
			send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0); // Error msg to user in case room doesn't exist
		}
	}
	else if (cmd == "LIST_USERS") { // List usernames in the user's current room

		std::string usersString{};
		if (param == msg) {
			usersString = usersToString();
		}
		else {
			usersString = roomList.contains(param) ? usersToString(param) : ("Room [" + param + "] does not exist.\n");
		}

		send(user.clientSocket, usersString.c_str(), usersString.size(), 0);

	}
	else if (cmd == "EXIT") { // Leaves the server gracefully
		std::string infoUser{ "Leaving server..." };
		std::string infoServer{ user.username + " left the server.\n" };

		broadcastToRoom(user.currentRoom, infoServer);
		send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0);
		removeUser(user);
	}
	else {
		std::string errorMsg = (cmd + " is an unknown command, try /HELP for a list of commands.\n");
		send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
	}
}

std::string usersToString(std::string roomName) {
	int i = 1;
	std::stringstream ss{};
	if (roomName == "Server") { // Default will list all users on the server
		ss << "[" << usernameList.size() << " Users in Server]\n";
		for (const auto& user : usernameList) {
			ss << std::setw(10) << user << " | ";
			if (i++ % 5 == 0) ss << "\n";
		}
	}
	else { // When a room is specified
		ss << "[" << roomList[roomName].getSize() << " Users in " << roomName << "]\n";
		for (const auto& user : roomList[roomName].getUsers()) {
			ss << std::setw(10) << user.username << " | ";
			if (i++ % 5 == 0) ss << "\n";
		}
	}

	std::string temp = ss.str();
	if (*(temp.rbegin() + 1) == '|') {
		*(temp.rbegin() + 1) = '\n';
		*temp.rbegin() = '\0';
	} // If last entry was a | then replace it with \n

	return temp;
}

std::string listRooms(const User& user) {
	std::string rooms{};
	for (const auto& room : roomList) {
		if (user.currentRoom == room.first) rooms += std::format(">{}<\n  [{} users]\n", room.first, room.second.getSize());
		else rooms += std::format("{}\n  [{} users]\n", room.first, room.second.getSize());
	}

	return rooms;
}

void removeUser(User& user) {
	roomList[user.currentRoom].removeUser(user); // Remove the client from their room
	usernameList.erase(user.username); // Remove their name from the list of usernames
	updateClientRoomList(); // Update client's roomlist

	closesocket(user.clientSocket);
}

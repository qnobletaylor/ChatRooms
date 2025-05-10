#include <iostream>
#include <iomanip>
#include <string>
#include <format>
#include <sstream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <ws2tcpip.h>
#include <ctime>
#include <regex>
#include <mutex>
#include "Room.h"
#include "User.h"
#pragma comment(lib, "ws2_32.lib")



/**
 * TODO
 *		- Look up *Object Pools*
 *
 *		> History <
 *		- Create a form of history for all sent messages, so that when new users connect then on joining the user can get
 *		a brief history of chat.
 *		- Cap the size of the number of messages stored, so the oldest messages will be tossed out
 *		- This could be stored in the form of a hashmap<User, List<String>> So you could also query the server to give all messages sent by a specific user.
 *		- Or simply basic string array[], and have a function which keeps track of size, probably better to use a list so that removing and replacing
 *		nodes will be O(1)
 *
 *
 *		> User commands <
 *		- Add command to list all users in a room, list users in server, possibly send direct msg to another user.
 *
 *		> server commands <
 *		- Add server only commands, such as kick user, move user, move all users. Just some ideas.
 *
 *		> Audio ** <
 *		- Maybe text to speech
 */


 // Let's figure out a way to make these not globals if possible.
std::mutex usernameListMutex, roomListMutex;
std::set<std::string> usernameList{}; // Global variable for connected users
std::map<std::string, Room> roomList{ {"Lobby", Room("Lobby")} }; // Initial room which all users start in will be known as the lobby
std::string help{
	"\t\t** List of Commands **\n\n"
	"CREATE_ROOM <name>\n\tCreate a new room and move to it.\n"
	"JOIN_ROOM <name>\n\tMove to another existing room.\n"
	"LIST_ROOMS\n\tDisplay a list of all rooms on the server and how many users are in each room.\n"
	"LIST_USERS <room = Lobby>\n\tLists users within a given room or if no argument is given lists all users in server.\n"
	"EXIT\n\tExit the server.\n"
	"HELP\n\tDisplay this list of commands.\n\n"
	"Instructions\n\tTo use a command simply type a '/' followed by the command and any necessary arguments\n"
	"\tFor example to create a new room use /CREATE_ROOM Study (commands are case insensitive)\n"
};

std::pair<std::string, int> getIP();

bool validateIPandPort(std::string input);

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

/**
 * Handles commands sent by a client.
 */
void userCommand(const std::string& cmd, User& user);

/**
 * Returns a string formatted list of users in the server.
 */
std::string usersToString();

/**
 * Returns a string formatted list of users within a given room.
 */
std::string usersToString(Room room);

/**
 * Returns a string listing all rooms in the server as well as an indicator (<) for which room the user is in.
 */
std::string listRooms(const User& user);

/**
 * Sends a string representation of rooms on the server to each client.
 *  */
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
		// Prompting for IP and Port
		ipAndPort = getIP();
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
	std::string prompt = "Server: Enter a username...\n";

	do {
		send(clientSocket, prompt.c_str(), prompt.size(), 0);

		bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	} while (bytesReceived <= 0);

	std::string msg(buffer, bytesReceived);

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
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			Message msg{ std::string(buffer, bytesReceived), getTime() };

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
			std::cerr << "recv failed.\n";
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
	//Currently only takes the first word after command, expand to including multi words denoted by " "
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1) - firstSpace);


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

			//std::lock_guard<std::mutex> guard(roomListMutex);
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
	else if (cmd == "LIST_ROOMS") { // List all rooms in the server and # of users in each room

		std::string rooms{ "Listing Rooms:\n" };

		rooms += listRooms(user);

		send(user.clientSocket, rooms.c_str(), rooms.size(), 0);
	}
	else if (cmd == "LIST_USERS") { // List usernames in the user's current room

		std::string usersString{};
		if (param == cmd) {
			usersString = usersToString();
		}
		else {
			usersString = roomList.contains(param) ? usersToString(roomList[param]) : (param + " does not exist.");
		}

		send(user.clientSocket, usersString.c_str(), usersString.size(), 0);

	}
	else if (cmd == "EXIT") { // Leaves the server gracefully
		std::string infoUser{ "Leaving server..." };
		std::string infoServer{ user.username + " left the server.\n" };

		broadcastToRoom(user.currentRoom, infoServer);
		send(user.clientSocket, infoUser.c_str(), infoUser.size(), 0);
		roomList[user.currentRoom].removeUser(user); // Remove the client from their room
		usernameList.erase(user.username); // Remove their name from the list of usernames
		updateClientRoomList(); // Update client's roomlist

		closesocket(user.clientSocket);
	}
	else {
		std::string errorMsg = (cmd + " is an unknown command, try /HELP for a list of commands.\n");
		send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
	}
}

std::string usersToString() {
	std::stringstream ss{};
	int i = 1;
	ss << "[" << usernameList.size() << " Users]\n";
	for (const auto& user : usernameList) {
		ss << std::setw(10) << user << " | ";
		if (i++ % 5 == 0) ss << "\n";
	}

	return ss.str();
}

std::string usersToString(Room room) {
	std::stringstream ss{};
	int i = 1;
	ss << "[" << usernameList.size() << " Users]\n";
	for (const auto& user : room.getUsers()) {
		ss << std::setw(10) << user.username << " | ";
		if (i++ % 5 == 0) ss << "\n";
	}

	return ss.str();
}

std::string listRooms(const User& user) {
	std::string rooms{};
	for (const auto& room : roomList) {
		if (user.currentRoom == room.first) rooms += std::format(">{}<\n  [{} users]\n", room.first, room.second.getSize());
		else rooms += std::format("{}\n  [{} users]\n", room.first, room.second.getSize());
	}

	return rooms;
}

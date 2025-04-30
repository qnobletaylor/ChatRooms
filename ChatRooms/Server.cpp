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
#include <mutex>
#include "Room.h"
#include "User.h"
#pragma comment(lib, "ws2_32.lib")



/**
 * TODO > History <
 *		- Create a form of history for all sent messages, so that when new users connect then on joining the user can get
 *		a brief history of chat.
 *		- Cap the size of the number of messages stored, so the oldest messages will be tossed out
 *		- This could be stored in the form of a hashmap<User, List<String>> So you could also query the server to give all messages sent by a specific user.
 *		- Or simply basic string array[], and have a function which keeps track of size, probably better to use a list so that removing and replacing
 *		nodes will be O(1)
 *
 *		> Rooms <
 *		- Create a room class which can simply hold a list of users (vector of users)
 *		- Instead of the CLIENTS vector which holds users, make a hashmap<string, Room> so that key will be room name and room will contain the list of users
 *		in that room currently.
 *		- add method for moving clients between rooms, make sure its threadsafe using std::mutex
 */
std::mutex usernameListMutex;
std::set<std::string> usernameList{}; // Global variable for connected users
std::mutex roomListMutex;
std::map<std::string, Room> roomList{ {"Lobby", Room("Lobby")} }; // Initial room which all users start in will be known as the lobby
//std::string help{ "CREATE_ROOM name\n\tcreates a room with a given name "
//};

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
				std::cout << output;

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
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1));

	if (cmd == "HELP") {
		send(user.clientSocket, help.c_str(), help.size(), 0);
	}
	else if (cmd == "CREATE_ROOM") {

		if (!roomList.contains(param)) {
			std::string info = std::format("Created and moved to {}\n", param);
			roomList[param] = Room(param, user);
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]);
			send(user.clientSocket, info.c_str(), info.size(), 0);
		}
		else {
			std::string errorMsg{ "The room " + param + " already exists, try another name.\n" };
			send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
		}
	}
	else if (cmd == "JOIN_ROOM") {

		if (roomList.contains(param)) {
			std::string info = std::format("Moved to {}, currently {} other users in this room\n", param, roomList[param].getSize());
			Room::moveUser(user, roomList[user.currentRoom], roomList[param]);
			send(user.clientSocket, info.c_str(), info.size(), 0);
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
		std::string exitMsg = "Leaving server...";
		send(user.clientSocket, exitMsg.c_str(), exitMsg.size(), 0);
		closesocket(user.clientSocket);
	}
	else {
		std::string errorMsg = (cmd + " is an unknown command, try /HELP for a list of commands.");
		send(user.clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
	}
}

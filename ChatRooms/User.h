#ifndef USER_H
#define USER_H

#include <string>;
#include <WinSock2.h>;

struct User {
	std::string username;
	SOCKET clientSocket;
	std::string currentRoom;

	bool operator==(const User& other) const { return username == other.username; }
	bool operator<(const User& other) const { return username < other.username; }
};

struct Message {
	std::string message;
	std::string timeStamp;
	std::string username;
};

#endif
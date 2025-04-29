export module User;

import <string>;
import <WinSock2.h>;

export struct User {
	std::string username;
	SOCKET clientSocket;
	std::string currentRoom = "Lobby";

	bool operator==(const User& other) const {
		return username == other.username;
	}
	bool operator<(const User& other) const {
		return username < other.username;
	}
};

export struct Message {
	std::string message;
	std::string timeStamp;
};
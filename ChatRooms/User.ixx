export module User;

import <string>;
import <WinSock2.h>;

export struct User {
	std::string username;
	SOCKET clientSocket;
};

export struct Message {
	std::string message;
	std::string timeStamp;
};
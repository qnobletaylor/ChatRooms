export module User;

import <string>;
import <WinSock2.h>;

export class User {
public:
	std::string userName;
	SOCKET clientSocket;

	User(std::string userName, SOCKET clientSocket) {
		this->userName = userName;
		this->clientSocket = clientSocket;
	}
};

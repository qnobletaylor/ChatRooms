export module User;

import <string>;
import <WinSock2.h>;

export class User {
public:
	std::string userName;
	SOCKET userSocket;

	User(std::string userName, SOCKET userSocket) {
		this->userName = userName;
		this->userSocket = userSocket;
	}
};

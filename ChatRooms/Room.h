#ifndef ROOM_H
#define ROOM_H

#include <string>;
#include <set>;
#include <vector>;
import User;

class Room {
public:
	Room(const std::string& name) {
		this->name = name;
	}

	Room(const std::string& name, const User& firstUser);

	bool removeUser(const User& user);

	bool addUser(const User& user);

	bool hasUser(const User& user);

	std::set<User> getUsers() { return userList; }

	static bool moveUser(const User& user, Room& current, Room& dest);

private:
	std::string name;
	std::set<User> userList{};
	User creator;
};

#endif
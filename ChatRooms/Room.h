#ifndef ROOM_H
#define ROOM_H

#include <string>;
#include <set>;
#include <vector>;
#include "User.h";

class Room {
public:
	Room();

	Room(const std::string& name) {
		this->name = name;
	}

	bool removeUser(const User& user);

	bool addUser(User& user);

	bool hasUser(const User& user) const;

	std::set<User> getUsers() const { return userList; }

	static bool moveUser(User& user, Room& current, Room& dest);

	std::string getName() const { return name; }

	int getSize() const { return userList.size(); }

private:
	std::string name;
	std::set<User> userList{};
};

#endif
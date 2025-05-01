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

	Room(const std::string& name, User& firstUser);

	bool removeUser(const User& user);

	bool addUser(User& user);

	bool hasUser(const User& user) const;

	std::set<User> getUsers() const { return userList; }

	static bool moveUser(User& user, Room& current, Room& dest);

	bool changeName(const User& user, const std::string& newName);

	std::string getName() const { return name; }

	User getCreator() const { return creator; }

	int getSize() const { return userList.size(); }

private:
	std::string name;
	std::set<User> userList{};
	User creator;
};

#endif
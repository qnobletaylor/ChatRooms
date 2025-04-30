#include "Room.h"

Room::Room(const std::string& name, const User& firstUser) : Room(name) {
	this->creator = firstUser;
	userList.insert(firstUser);
}

bool Room::removeUser(const User& user) {
	return hasUser(user) ? userList.erase(user) : false;
}

bool Room::addUser(const User& user) {
	return userList.insert(user).second;
}

bool Room::hasUser(const User& user) {
	if (userList.find(user) != userList.end()) return true;
	else return false;
}

bool Room::moveUser(const User& user, Room& current, Room& dest) {
	if (current.hasUser(user)) {
		current.removeUser(user);
		return dest.addUser(user);
	}
	else
		return false;
}
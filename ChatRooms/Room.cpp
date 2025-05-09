#include "Room.h"
#include <mutex>

std::mutex mutex;

Room::Room() : Room("test") {}

Room::Room(const std::string& name, User& firstUser) : Room(name) {
	this->creator = firstUser;
	userList.insert(firstUser);
}

bool Room::removeUser(const User& user) {
	std::lock_guard<std::mutex> guard(mutex);
	return hasUser(user) ? userList.erase(user) : false;
}

bool Room::addUser(User& user) {
	std::lock_guard<std::mutex> guard(mutex);
	user.currentRoom = name;
	return userList.insert(user).second; // Check if emplace vs insert would be better (i.e. copy or move?)
}

bool Room::hasUser(const User& user) const {
	if (userList.find(user) != userList.end()) return true;
	else return false;
}

bool Room::moveUser(User& user, Room& current, Room& dest) {
	// I'm thinking that there doesn't need to be a lock in this method due to other methods being called already containing locks
	if (current.hasUser(user)) {
		current.removeUser(user);
		return dest.addUser(user);
	}
	else
		return false;
}

bool Room::changeName(const User& user, const std::string& newName) {
	if (user == creator) {
		name = newName;
		return true;
	}
	else return false;
}

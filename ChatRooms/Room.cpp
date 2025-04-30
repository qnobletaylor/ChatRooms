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

bool Room::addUser(const User& user) {
	std::lock_guard<std::mutex> guard(mutex);
	return userList.insert(user).second;
}

bool Room::hasUser(const User& user) const {
	if (userList.find(user) != userList.end()) return true;
	else return false;
}

bool Room::moveUser(User& user, Room& current, Room& dest) {
	// I'm thinking that there doesn't need to be a lock in this method due to other methods being called already containing locks
	if (current.hasUser(user)) {
		current.removeUser(user);
		user.currentRoom = dest.getName();
		return dest.addUser(user);
	}
	else
		return false;
}
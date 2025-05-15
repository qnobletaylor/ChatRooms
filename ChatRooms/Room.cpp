/**
 * @file   Room.cpp
 * @author Quinlin Taylor (CSC-284)
 * @date   5/14/2025
 * @brief  This file includes all definitions for the \link Room.h \endlink file and class.
 */

#include "Room.h"
#include <mutex>

std::mutex mutex;

Room::Room() : Room("Lobby") {}

bool Room::removeUser(const User& user) {
	std::lock_guard<std::mutex> guard(mutex);
	return hasUser(user) ? userList.erase(user) : false;
}

bool Room::addUser(User& user) {
	std::lock_guard<std::mutex> guard(mutex);
	user.currentRoom = name;
	return userList.insert(user).second;
}

bool Room::hasUser(const User& user) const {
	if (userList.find(user) != userList.end()) return true;
	else return false;
}

bool Room::moveUser(User& user, Room& current, Room& dest) {
	if (current.hasUser(user)) {
		current.removeUser(user);
		return dest.addUser(user);
	}
	else
		return false;
}

#ifndef ROOM_H
#define ROOM_H
#pragma once

import <string>;
import <set>;
import <vector>;
import User;

class Room {
public:
	Room(const std::string& name) {
		this->name = name;
	}

	Room(const std::string& name, const User& firstUser) : Room(name) {
		this->creator = firstUser;
		userList.insert(firstUser);
	}

	/**
	 * .erase returns the # of elements erased.
	 *
	 * Thus returns true when erase is successful, false otherwise.
	 *  */
	bool removeUser(const User& user) {
		return hasUser(user) ? userList.erase(user) : false;
	}

	/**
	 * Returns true if inserted, false if not (in case of the user already being in the set)
	 *  */
	bool addUser(const User& user) {
		return userList.insert(user).second;
	}

	bool hasUser(const User& user) {
		if (userList.find(user) != userList.end()) return true;
		else return false;
	}

	std::set<User> getUsers() {
		return userList;
	}

	static bool moveUser(const User& user, Room& current, Room& dest) {
		if (current.hasUser(user)) {
			current.removeUser(user);
			return dest.addUser(user);
		}
		else
			return false;
	}

private:
	std::string name;
	std::set<User> userList{};
	User creator;
};

#endif
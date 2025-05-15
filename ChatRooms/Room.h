/**
 * @file   Room.h
 * @author Quinlin Taylor (CSC-284)
 * @date   5/14/2025
 * @brief  This file contains the Room class declaration.
 */

#ifndef ROOM_H
#define ROOM_H

#include <string>;
#include <set>;
#include <vector>;
#include "User.h";

 /**
  * @brief the Room class is used by the server for handling the speration of clients into different chat rooms.
  */
class Room {
public:
	/**
	 * @brief default constructor, simply creates a room with the name "Lobby" and zero Users.
	 *
	 */
	Room();

	/**
	 * @brief this constructor will create a room with a given name and zero Users to begin with.
	 *
	 * @param name the name given to the new Room object.
	 */
	Room(const std::string& name) {
		this->name = name;
	}

	/**
	 * @brief removes a User from this instance of a Room.
	 *
	 * @param user the user to remove.
	 * @return true if the user exists in the room and has been removed, false if the user is not in the room.
	 */
	bool removeUser(const User& user);

	/**
	 * @brief adds a User to the \link userList \endlink of this instance of Room.
	 *
	 * @param user the User to add to the Room.
	 * @return true if the User has been added, false if the User already in the Room.
	 */
	bool addUser(User& user);

	/**
	 * @brief checks if a User is in the \link userList \endlink of this Room.
	 *
	 * @param user the User to validate.
	 * @return true if the Room has the User, false otherwise.
	 */
	bool hasUser(const User& user) const;

	/**
	 * @brief gets the \link userList \endlink .
	 *
	 * @return this Room's userList attribute.
	 */
	std::set<User> getUsers() const { return userList; }

	/**
	 * @brief a static method which will move a User from one room to another room. Calls \link removeUser \endlink on their current room and
	 * then \link addUser \endlink on the destination room.
	 *
	 * @param user the User to move.
	 * @param current the User's current Room object.
	 * @param dest the Room to transfer the User to.
	 * @return true if the move is successful, false otherwise.
	 */
	static bool moveUser(User& user, Room& current, Room& dest);

	/**
	 * @brief get the Room name attribute.
	 *
	 * @return the name of the room.
	 */
	std::string getName() const { return name; }

	/**
	 * @brief calls the .size() method on the userList set.
	 *
	 * @return the number of Users stored in the Room.
	 */
	int getSize() const { return userList.size(); }

private:
	std::string name;
	std::set<User> userList{};
};

#endif
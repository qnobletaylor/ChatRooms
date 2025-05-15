/**
 * @file   User.h
 * @author Quinlin Taylor (CSC-284)
 * @date   5/14/2025
 * @brief  This header file includes two structs used by the Server application. User and Message.
 */

#ifndef USER_H
#define USER_H

#include <string>;
#include <WinSock2.h>;

 /**
  * @brief holds all data stored for when a User connects to the Server, created on initial connection made by client.
  */
struct User {
	std::string username;
	SOCKET clientSocket;
	std::string currentRoom;

	bool operator==(const User& other) const { return username == other.username; }
	bool operator<(const User& other) const { return username < other.username; }
};

/**
 * @brief used for packaging messages sent from Server to Clients.
 */
struct Message {
	std::string message;
	std::string timeStamp;
	std::string username;
};

#endif
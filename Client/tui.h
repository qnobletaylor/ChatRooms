/**
 * @file tui.h
 * @author Quinlin Taylor (CSC-284)
 * @date 05/14/2025
 * @brief This file contains the class which handles all textual ui implementation depending on the ncurses library.
 */

#ifndef TUI_H
#define TUI_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>;
#include "curses.h"
#pragma comment(lib, "Ws2_32.lib")

 /**
  * @brief Class handling all textual ui functionality.
  */
class tui
{
public:
	/**
	 * @brief Construct a new tui object.
	 *
	 * @param server the socket with which the client has made a connection with.
	 */
	tui(SOCKET server);

	/**
	 * @brief Takes care of all initialization steps for the command line textual ui.
	 *
	 */
	void drawUI();

	/**
	 * @brief Called as the handler for a new thread accepting all incoming data from the Server.
	 *
	 */
	void receiveMessages();
private:
	SOCKET server;
	WINDOW* outputWin{};
	WINDOW* inputWin{};
	WINDOW* roomsWin{};

	/**
	 * @brief Prints text to the Client-side terminal inside the \link outputWin \endlink Window. This will typically be messages received from the Server.

	 * @param msg the C-style string to print to the outputWin.
	 */
	void printToOutput(const char* msg);

	/**
	 * @brief Main thread loop which waits for the user to input into the \link inputWin \endlink Window to stage a message for sending to the Server.
	 *
	 */
	void getInput();

	/**
	 * @brief Handles executing necessary ncurses functions for when a backspace character is detected in the \link inputWin \endlink Window.
	 *
	 */
	void backSpace();

	/**
	 * @brief Sends a C-style string of bytes to the server socket.
	 *
	 * @param msg the string being sent to the server.
	 */
	void sendInput(char msg);

	/**
	 * @brief Handles updating the textual ui for the visual representation in the \link roomsWin \endlink of rooms currently available on the connected server.
	 * Highlights the room which this client is connected to.
	 *
	 * @param msg the message received from the server used to populate the ncurses Window.
	 */
	void updateRooms(const char* msg);
};

#endif
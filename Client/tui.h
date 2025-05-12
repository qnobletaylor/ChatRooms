#ifndef TUI_H
#define TUI_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>;
#include "curses.h"
#pragma comment(lib, "Ws2_32.lib")

/**
 * Class representing the textual ui using ncurses.
 */
class tui
{
public:
	tui(SOCKET server);

	void drawUI();
	void receiveMessages();
private:
	SOCKET server;
	WINDOW* outputWin{};
	WINDOW* inputWin{};
	WINDOW* roomsWin{};


	void printToOutput(const char* msg);
	void sendToServer(const char* msg);
	std::string getInput();
	void backSpace();
	void newInput(char c);
	void updateRooms(const char* msg);
};

#endif
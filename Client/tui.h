#ifndef TUI_H
#define TUI_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>;
#include "curses.h"
#pragma comment(lib, "Ws2_32.lib")



class tui
{
public:
	tui() {};
	tui(SOCKET client);

	void drawUI();

private:
	SOCKET client;
	WINDOW* outputWin{};
	WINDOW* inputWin{};
	WINDOW* roomsWin{};

	void receiveMessages();
	void printToOutput(const char* msg);
	char getInput();
	void updateRooms(const std::string& rooms);
};

#endif
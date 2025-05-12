#include "tui.h"
#include <sstream>

tui::tui(SOCKET server) {
	this->server = server;
}

void tui::drawUI() {
	std::string duckArt = "   _\n__(~)<\n\\_)_)"; // quack

	/* NCURSES INIT */
	initscr();
	noecho();
	cbreak();

	// Borders for input and output windows
	WINDOW* outputBorder = newwin(25, getmaxx(stdscr) - 20, 0, 0);
	WINDOW* inputBorder = newwin(5, getmaxx(outputBorder), getmaxy(outputBorder), 0);
	WINDOW* roomsBorder = newwin(25, 19, 0, getmaxx(outputBorder) + 1);
	WINDOW* duck = newwin(3, 8, getmaxy(roomsBorder) + 1, getmaxx(inputBorder) + 7); // window for the duck

	outputWin = newwin(23, getmaxx(outputBorder) - 2, 1, 1); // For displaying messages
	inputWin = newwin(3, getmaxx(inputBorder) - 2, getbegy(inputBorder) + 1, 1); // For typing messages
	roomsWin = newwin(23, getmaxx(roomsBorder) - 2, 1, getbegx(roomsBorder) + 1);


	scrollok(outputWin, true); // allows the text to continue scrolling outside of the window
	leaveok(outputWin, true); // Don't show cursor when printing in this window
	leaveok(roomsWin, true);
	curs_set(1); // show cursor
	refresh();
	keypad(inputWin, true); // Detect arrow keys for moving left and right in unsubmitted text
	// Draw Borders
	box(outputBorder, 0, 0);
	box(inputBorder, 0, 0);
	box(roomsBorder, 0, 0);
	// Borders
	wrefresh(outputBorder);
	wrefresh(inputBorder);
	wrefresh(roomsBorder);
	// Text input/display
	wrefresh(outputWin);
	wrefresh(inputWin);
	wrefresh(roomsWin);


	waddstr(duck, duckArt.c_str());
	wrefresh(duck);

	/* END NCURSES INIT */

	printToOutput("Connected to server. Type messages, /help for info, or /exit to quit.\n");

	// Thread handling output from server
	std::thread receiverThread(&tui::receiveMessages, this);
	receiverThread.detach();

	wmove(inputWin, 0, 0);
	getInput();

	endwin();
}

void tui::receiveMessages() {
	char buffer[1024];
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(server, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0) {
			if (buffer[0] == '0') updateRooms(buffer);
			else printToOutput(buffer);
		}
		else if (bytesReceived == 0) {
			printToOutput("Server disconnected.\n");
			break;
		}
		else {
			printToOutput("recv failed.\n");
			break;
		}
	}

	closesocket(server);
	WSACleanup();
};

void tui::printToOutput(const char* msg) {
	waddstr(outputWin, msg); // print msg
	wrefresh(outputWin); // refresh to appear in window
};

void tui::sendToServer(const char* msg) {
	send(server, msg, sizeof(msg), 0); // Send whats in the buffer
	wclear(inputWin); // clear input win
	wmove(inputWin, 0, 0);
	wrefresh(inputWin);
}

std::string tui::getInput() {
	char msg[1024]; // Store the message, up to 1023 characters
	int cursor = 0; // Position within the buffer
	wchar_t c; // char to store
	ZeroMemory(msg, sizeof(msg));

	while (true) {
		c = wgetch(inputWin);

		switch (c) {
		case '\n': { // On pressing enter
			sendToServer(msg);
			ZeroMemory(msg, sizeof(msg)); // zero our buffer
			break;
		}
				 /* Handling three different backspace characters */
		case KEY_BACKSPACE: {
			if (cursor > 0) {
				backSpace();
				msg[--cursor] = '\0';
			}
			break;
		}
		case 127: {
			if (cursor > 0) {
				backSpace();
				msg[--cursor] = '\0';
			}
			break;
		}
		case 8: {
			if (cursor > 0) {
				backSpace();
				msg[--cursor] = '\0';
			}
			break;
		}
			  /* end of backspace handling */
		default: {
			if (cursor < sizeof(msg) - 1) {
				newInput(c);
				msg[cursor++] = c;
			}
		}
		}
	}
};

void tui::backSpace() {
	int y, x;
	getyx(inputWin, y, x);

	if (x == 0 && y > 0 && y <= getmaxy(inputWin)) mvwdelch(inputWin, y - 1, getmaxx(inputWin) - 1); // backspace for when on second or third line
	else mvwdelch(inputWin, y, x - 1);

	wrefresh(inputWin);
}

void tui::newInput(char c) {
	wprintw(inputWin, "%c", c);
	wrefresh(inputWin);
}


void tui::updateRooms(const char* msg) {
	wclear(roomsWin);
	wmove(roomsWin, 0, 0); // Move cursor to beginning of roomsWin


	int i = 0;

	while (msg[++i] != '\0') { // Highlights the room the user is in
		if (msg[i] == '>') {
			wattron(roomsWin, A_STANDOUT);
			continue;
		}
		if (msg[i] == '<') {
			wattroff(roomsWin, A_STANDOUT);
			continue;
		}
		waddch(roomsWin, msg[i]);
	}

	wrefresh(roomsWin);
};

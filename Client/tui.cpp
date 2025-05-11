#include "tui.h"

tui::tui(SOCKET client) {
	this->client = client;


}

void tui::drawUI() {
	std::string duckArt = "   _\n__(~)<\n\\_)_)";

	/* NCURSES INIT */
	initscr();

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

	printToOutput("Connected to server. Type messages, /help for info, or /exit to quit.\n");

	// Thread handling output from server
	std::thread receiverThread(receiveMessages, client);
	receiverThread.detach();

	// Main thread handling input
	bool exitCondition = false;
	while (exitCondition == false) {
		exitCondition = getInput();
	}

	/* END NCURSES INIT */
	getch(); // TEMP //
	endwin();
}

void tui::receiveMessages() {
	char buffer[1024];
	while (true) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(client, buffer, sizeof(buffer), 0);
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

	closesocket(client);
	WSACleanup();
};

void tui::printToOutput(const char* msg) {
	waddstr(outputWin, msg); // print msg
	wrefresh(outputWin); // refresh to appear in window
};

bool tui::getInput() { /// Change this !!! ///
	char inputStr[1024];
	wgetstr(inputWin, inputStr); // Take line input till \n
	wclear(inputWin); // clear the window
	wmove(inputWin, 0, 0); // move cursor back to start of window
	wrefresh(inputWin); // refresh window

	printToOutput(inputStr);
};

void tui::updateRooms(const char* msg) {
	wclear(roomsWin);
	wmove(roomsWin, 0, 0); // Move cursor to beginning of roomsWin


	int i = -1;

	while (msg[++i] != '\0') {
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

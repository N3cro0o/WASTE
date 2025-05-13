#include "curses.h"
#include <string>
#include <format>

#define VERSION "0.1"

enum class State {
	Raw,
	Cooked,
	Quit
};

// Debug variables
bool print_debug_strings = false;

// State variables
State curr_state = State::Cooked;
const std::string ALL_COMMANDS[] = {
		"edit",
		"quit"
};
const int MAX_COMMANDS = 2;

// Data varibles
std::string file_data = "";

void print_line(std::string s, bool debug = false) {
	if (debug) {
		attron(COLOR_PAIR(10));
		printw(s.c_str());
		attroff(COLOR_PAIR(10));
		getch();
	}
	else
		printw(s.c_str());
	refresh();
}
void print_line(int y, int x, std::string s, bool debug = false) {
	move(y, x);
	print_line(s, debug);
}

int check_command(char* input) {
	std::string input_str;
	int i = 0;
	while (1) {
		if (input[i] == 0 || input[i] == ' ')
			break;
		input_str.push_back(input[i++]);
	}
	for (int i = 0; i < MAX_COMMANDS; i++) {
		int cmp = ALL_COMMANDS[i].compare(input_str);
		if (cmp == 0)
			return i;
	}
	return -1;
}

void handle_command(char* input) {
	switch (check_command(input)) {
	case 0:
		curr_state = State::Raw;
		raw();
		noecho();
		break;
	case 1:
		// Quit
		curr_state = State::Quit;
		break;
	default:
		// Print error message
		break;
	}

	return;
}

int main()
{
	int row, col;
	int cmd_col;
	WINDOW* menu_window;
	// Initialize curses
	initscr();
	keypad(stdscr, true);
	cbreak();
	// Colors
	start_color();
	init_pair(10, COLOR_CYAN, COLOR_BLACK);
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	// Window size
	getmaxyx(stdscr, row, col);
	if (print_debug_strings)
		print_line("Initialization\n", true);
	// Create menu window
	menu_window = newwin(3, col, 0, 0);
	box(menu_window, 0, 0);
	mvwprintw(menu_window, 1, 1, "WASTE v.%s | Current state: %s | Command: ", VERSION, curr_state == State::Raw ? "Raw" : "Ckd");
	cmd_col = getcurx(menu_window);
	wrefresh(menu_window);
	while (curr_state != State::Quit) {
		// Before anything, clear the screen
		//wclear(stdscr); // IDK about this one
		wclear(menu_window);
		// Print output below the window
		move(3, 0);
		addstr(file_data.c_str());
		wrefresh(stdscr);
		if (curr_state == State::Cooked) {
			// Menu
			box(menu_window, 0, 0);
			mvwprintw(menu_window, 1, 1, "WASTE v.%s | Current state: %s | Command: ", VERSION, curr_state == State::Raw ? "Raw" : "Ckd");
			wrefresh(menu_window);
			// Get commands
			char input[100];
			wgetstr(menu_window, input);
			handle_command(input);
		}
		else if (curr_state == State::Raw) {
			// Print how to exit
			box(menu_window, 0, 0);
			mvwprintw(menu_window, 1, 1, "WASTE v.%s | Current state: %s | To exit RAW state, press F1", VERSION, curr_state == State::Raw ? "Raw" : "Ckd");
			wrefresh(menu_window);
			// Set up raw
			move(3, 0);
			int y, x;
			while (1) {
				// Get char
				// Handle special characters
				// Remove or change chars on screen buffor if needed
				// Otherwise add char to screen buffor
				// Get pos
				y = getcury(stdscr);
				x = getcurx(stdscr);
				// Get char
				int ch = 0;
				ch = wgetch(stdscr);
				// Arrows
				if (ch == KEY_LEFT) {
					move(y, x - 1);
					continue;
				}
				else if (ch == KEY_RIGHT) {
					move(y, x + 1);
					continue;
				}
				else if (ch == KEY_UP) {
					move(y - 1, x);
					continue;
				}
				else if (ch == KEY_DOWN) {
					move(y + 1, x);
					continue;
				}
				// Handle special characters
				if (ch == KEY_F(1))
					break;
				if (ch == '\b') { // Backspace
					mvwaddch(stdscr, y, x - 1, ' ');
					move(y, x - 1);
					wrefresh(stdscr);
					continue;
				}
				if (ch == '\r') {
					move(y + 1, 0);
					wrefresh(stdscr);
					continue;
				}
				// Add char
				wprintw(stdscr, "%c", ch);
				wrefresh(stdscr);
			}
			noraw();
			echo();
			curr_state = State::Cooked;
		}
	}
	endwin();

	return 0;
}
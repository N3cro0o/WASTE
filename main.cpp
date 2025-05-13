#include "curses.h"
#include <string>
#include <format>

enum class State {
	Raw,
	Cooked
};

// Debug variables
bool print_debug_strings = false;

// State variables
State curr_state = State::Cooked;
char cmd_output[100] = "Lorem ipsum dolor sit amet";

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
	mvwprintw(menu_window, 1, 1, "Current satate: %s | Command: ",curr_state == State::Raw ? "Raw" : "Ckd");
	cmd_col = getparx(menu_window);
	wrefresh(menu_window);
	while (1) {
		// Before anything, clear the screen
		wclear(stdscr);
		wclear(menu_window);
		// Print output below the window
		move(3, 0);
		addstr(cmd_output);
		wrefresh(stdscr);
		// Menu
		box(menu_window, 0, 0);
		mvwprintw(menu_window, 1, 1, "Current satate: %s | Command: ", curr_state == State::Raw ? "Raw" : "Ckd");
		wrefresh(menu_window);
		if (curr_state == State::Cooked) {
			// Get commands
			char input[100];
			wmove(menu_window, 1, cmd_col);
			wgetstr(menu_window, input);
			int j = 0;
			while (cmd_output[++j] != 0);
			for (int i = 0; i < 100; i++) {
				cmd_output[j++] = input[i];
			}
		}
	}

	getch();
	endwin();

	return 0;
}
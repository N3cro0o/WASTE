#include "curses.h"

#include <string>
#include <format>
#include <fstream>
#include <regex>

#define FILE_FORMAT std::string

const char* VERSION = "1.0";

enum class State {
	Raw,
	Cooked,
	Quit
};

// Regex
std::regex IOregex("^(\\w|_)+\\.\\w+");

// Debug variables
bool print_debug_strings = false;

// State variables
int row, col;
int cmd_col;
State curr_state = State::Cooked;
const std::string ALL_COMMANDS[] = {
		"edit",
		"quit",
		"debug_break",
		"save",
		"load",
		"clear"
};
const int MAX_COMMANDS = 6;

// Data varibles
std::string err_string = "Write next command";
FILE_FORMAT file_data = "";
int scroll_number = 0;

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

void wprint_all_data(WINDOW* wind, FILE_FORMAT data, bool clear = false) {
	if (clear) {
		for (int i = 4; i < LINES; i++) {
			move(i, 0);
			for (int j = 0; j < COLS; j++)
				waddch(stdscr, ' ');
		}
		wrefresh(stdscr);
		move(4, 0);
	}
	int n_count = 0;
	for (int i = 0; i < data.size(); i++) {
		if (data[i] == '\n')
			n_count++;

		if (n_count >= scroll_number) {
			n_count = i == 0 ? i : i + 1;
			break;
		}
	}
	waddstr(wind, data.substr(n_count, data.size()).data());
}

void wprint_all_data_no_move(WINDOW* wind, FILE_FORMAT data, bool clear = false) {
	int y = getcury(stdscr);
	int x = getcurx(stdscr);
	wprint_all_data(wind, data, clear);
	move(y, x);
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

bool IO_check(std::string str) {
	if (str.size() < 5) {
		err_string = "Missing file name";
		return 0;
	}
	std::string sub = str.substr(5, sub.size());
	if (!std::regex_match(sub, IOregex)) {
		err_string = "Wrong file name";
		return 0;
	}
	return 1;
}

void handle_command(char* input) {
	std::string sub;
	std::fstream file;
	std::regex IOregex("(\w|_)+\.\w+");
	switch (check_command(input)) {
	case 0:
		curr_state = State::Raw;
		err_string = "Entering text editor (raw) mode";
		raw();
		noecho();
		break;
	case 1:
		// Quit
		curr_state = State::Quit;
		err_string = "Quitting...";
		break;
	case 2:
		// Debug break
		1 == 1;
		break;
	case 3:
		// Save
		sub = input;
		if (!IO_check(sub))
			return;
		try {
			file.open(sub.substr(5, sub.size()), std::fstream::out | std::fstream::trunc);
			if (!file.is_open()) throw;
			file << file_data;
			file.close();
		}
		catch (...) {
			err_string = "Cannot save to file: " + sub.substr(5, sub.size());
			return;
		}
		err_string = "Succesfuly saved to file: " + sub.substr(5, sub.size());
		break;
	case 4:
		// Open
		sub = input;
		file_data.clear();
		if (!IO_check(sub))
			return;
		try {
			file.open(sub.substr(5, sub.size()), std::fstream::in | std::fstream::app);
			if (!file.is_open()) throw;
			sub.clear();
			while (std::getline(file, sub)) {
				file_data += sub;
				file_data.insert(file_data.begin() + file_data.size(), '\n');
			}
			if (!file_data.empty())
				file_data.pop_back();
			file.close();
		}
		catch (...) {
			err_string = "Cannot load from file: " + sub;
			file_data.clear();
			return;
		}
		sub = input;
		move(4, 0);
		err_string = "Succesfuly loaded from file: " + sub.substr(5, sub.size());
		break;
	case 5:
		// Clear
		file_data.clear();
		wprint_all_data(stdscr, file_data, true);
		err_string = "Cleared all text data";
		break;
	default:
		err_string = "Command does not exist";
		break;
	}
}

// FILE_FORMAT functions
bool raw_handle_arrows(int ch) {
	// Get pos
	int y = getcury(stdscr);
	int x = getcurx(stdscr);
	// Check if first column
	if ((y == 4 && x == 0 && ch == KEY_LEFT) || (y == 4 && ch == KEY_UP)) {
		if (scroll_number == 0) return true;
		scroll_number--;
		move(++y, x);
	}
	// Arrows
	if (ch == KEY_LEFT) {
		// Check if first column
		if (x == 0) {
			int y_start = y;
			y -= 5;
			for (int i = 0; i < file_data.size(); i++) {
				// Find end line
				if (file_data[i] == '\n') {
					// Check if line number is smaller than 0. If not, decrease by one
					if (--y < 0)
						break;
					// Reset x
					x = 0;
				}
				else
					// Increase x by one
					x++;
			}
			move(y_start - 1, x);
			return 1;
		}
		// Check if tab
		int i_pos = 0;
		int n_counter = 0;
		int tab_counter = 0;
		int last_char = 0;
		for (int i = 0; i < file_data.size(); i++) {
			if (n_counter == y - 4 + scroll_number) {
				if (file_data[i] == '\t')
					tab_counter++;
				if (x - tab_counter * 7 <= (i - i_pos)) {
					last_char = file_data[i - 1];
					break;
				}
			}
			if (file_data[i] == '\n') {
				i_pos = i + 1;
				n_counter++;
			}
		}
		if (last_char == '\t')
			move(y, x - 8);
		else
			move(y, x - 1);
	}
	else if (ch == KEY_RIGHT) {
		// Check if can move; look for characters in data string
		bool character_check = false;
		int i_pos = 0;
		int n_counter = 0;
		bool tab_check = false;
		int line_tab_num = 0;
		for (int i = 0; i < file_data.size(); i++) {
			if (n_counter == y - 4 + scroll_number && x - line_tab_num * 7 <= (i - i_pos)) {
				if (file_data[i] != '\n')
					character_check = true;
				if (file_data[i] == '\t')
					tab_check = true;
			}
			if (file_data[i] == '\n') {
				line_tab_num = 0;
				i_pos = i + 1;
				n_counter++;
			}
			if (file_data[i] == '\t')
				line_tab_num++;
		}
		if (character_check) {
			if (tab_check)
				x += 7;
			move(y, x + 1);
		}
		else {
			if (n_counter > y - 4) {
				if (y == LINES - 1) {
					scroll_number++; move(y, tab_check ? 7 : 0);
				}
				else
					move(y + 1, tab_check ? 7 : 0);
			}
			else
				move(y, x);
		}
		wrefresh(stdscr);
	}
	else if (ch == KEY_UP) {
		move(y - 1, x);
	}
	else if (ch == KEY_DOWN) {
		int n_counter = 0;
		for (int i = 0; i < file_data.size(); i++) {
			if (file_data[i] == '\n')
				n_counter++;
		}
		if (n_counter > y - 4) {
			if (y == LINES - 1)
				scroll_number++;
			else
				move(y + 1, x);
		}
	}
	else
		return false;
	return true;
}

bool raw_handle_backspace(int ch) {
	if (ch != '\b')
		return false;
	// Get curses pos
	int y = getcury(stdscr);
	int x = getcurx(stdscr);
	// Error handling
	if (y == 4 && x == 0)
		return true;
	if (file_data.empty()) {
		err_string = "Data file is empty!";
		return true;
	}
	// FILE_FORMAT erase
	int i_target = file_data.size();
	int i_pos = 0;
	int n_counter = 0;
	int tab_counter = 0;
	int last_char = 0;
	for (int i = 0; i < file_data.size(); i++) {
		if (n_counter == y - 4 + scroll_number) {
			if (file_data[i] == '\t')
				tab_counter++;
			if (x - tab_counter * 7 <= (i - i_pos)) {
				i_target = i;
				last_char = file_data[i - 1];
				break;
			}
		}
		if (file_data[i] == '\n') {
			i_pos = i + 1;
			n_counter++;
		}
	}
	file_data.erase(file_data.begin() + i_target - 1);
	// Curses erase
	if (x == 0) {
		for (int i = col - 1; i >= 0; i--) {
			wmove(stdscr, y - 1, i);
			wrefresh(stdscr);
			if (winch(stdscr) != 0 && winch(stdscr) != ' ') {
				wmove(stdscr, y - 1, i + 1);
				break;
			}
		}
		wprint_all_data(stdscr, file_data, true);
		return true;
	}
	// Backspace
	move(y, COLS);
	ch = ' ';
	int mem_ch = 0;
	for (int i = COLS - 1; i >= x - 1; i--) {
		move(y, i);
		mem_ch = winch(stdscr);
		waddch(stdscr, ch);
		ch = mem_ch;
	}
	if (last_char == '\t')
		move(y, x - 8);
	else
		move(y, x - 1);
	/*mvwaddch(stdscr, y, x - 1, ' ');
	move(y, x - 1);
	wrefresh(stdscr);
	file_data.pop_back();*/
	return true;
}

void handle_raw_add_char(FILE_FORMAT* data, int ch) {
	int y = getcury(stdscr);
	int x = getcurx(stdscr);
	/*
	* Curses
		 - Store cur pos
		 - Move all characters on screen to the left
		 - Go back and place char
	* FILE_FORMAT
		 - Get position in data
		 - Insert char
	*/
	// FILE_FORMAT
	int i_target = data->size();
	int i_pos = 0;
	int n_counter = 0;
	int tab_counter = 0;
	for (int i = 0; i < data->size(); i++) {
		int debug_cha = data->at(i);
		if (n_counter == y - 4 + scroll_number) {
			if (data->at(i) == '\t')
				tab_counter++;
			if (x - tab_counter * 7 <= (i - i_pos)) {
				i_target = i;
				break;
			}
		}
		if (file_data[i] == '\n') {
			i_pos = i + 1;
			n_counter++;
		}
	}
	data->insert(data->begin() + i_target, ch);
	// Curses
	int last_ch = winch(stdscr);
	int mem_ch = 0;
	for (int i = x + 1; i < COLS; i++) {
		move(y, i);
		mem_ch = winch(stdscr);
		waddch(stdscr, last_ch);
		last_ch = mem_ch;
	}
	move(y, x);
	waddch(stdscr, ch);
	wrefresh(stdscr);
}

int main()
{
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
	// Tests
	if (print_debug_strings) {
		print_line("String test\n", true);
		file_data.insert(file_data.begin(), 'A');
		file_data.insert(file_data.begin() + 1, 'B');
		file_data.insert(file_data.begin(), 'C');

		print_line(0, 0, "Initialization end\n", true);
	}
	// Create menu window
	menu_window = newwin(4, col, 0, 0);
	box(menu_window, 0, 0);
	mvwprintw(menu_window, 1, 1, "WASTE v.%s | Current state: %s | Command: ", VERSION, curr_state == State::Raw ? "Raw" : "Ckd");
	cmd_col = getcurx(menu_window);
	wrefresh(menu_window);
	while (curr_state != State::Quit) {
		// Before anything, clear the screen
		//wclear(stdscr); // IDK about this one
		wclear(menu_window);
		// Print output below the window
		move(4, 0);
		wprint_all_data(stdscr, file_data);
		wrefresh(stdscr);
		if (curr_state == State::Cooked) {
			// Menu
			box(menu_window, 0, 0);
			mvwprintw(menu_window, 2, 1, "INFO: %s", err_string.data());
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
			mvwprintw(menu_window, 2, 1, "INFO: %s", err_string.data());
			mvwprintw(menu_window, 1, 1, "WASTE v.%s | Current state: %s | To exit RAW state, press F1", VERSION, curr_state == State::Raw ? "Raw" : "Ckd");
			wrefresh(menu_window);
			// Set up raw
			move(4, 0);
			int y, x;
			while (1) {
				wprint_all_data_no_move(stdscr, file_data, true);
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
				if (raw_handle_arrows(ch))
					continue;
				// Handle special characters
				if (ch == KEY_F(1))
					break;
				if (raw_handle_backspace(ch))
					continue;
				if (ch == '\r') {
					handle_raw_add_char(&file_data, '\n');
					wprint_all_data(stdscr, file_data, true);
					move(y + 1, 0);
					continue;
				}
				// Add char
				handle_raw_add_char(&file_data, ch);
			}
			noraw();
			echo();
			err_string = "Write next command";
			curr_state = State::Cooked;
		}
	}
	endwin();

	return 0;
}
#include <iostream>
#include <bitset>
#include <Windows.h>
#include <vector>

#define data_type std::vector<std::string>

/*
* TO DO
* + command mode
* - write mode
* - switch modes
* - save to file
*/

enum class State {
	Cooked,
	Raw
};

const std::string ALL_COMMANDS[] = {
		"quit",
		"edit",
		"load",
		"debug"
};
const int ALL_COMMANDS_COUNTER = 4;

State curr_state = State::Cooked;
HANDLE stdin_handle, stdout_handle;
CONSOLE_SCREEN_BUFFER_INFO screen_buffer;
char command_buffor[32];
unsigned int command_buffor_index = 0;

data_type file_data = { {""} };
data_type file_data_unsaved = { {""} };
std::pair<unsigned long, unsigned long> coords = { 0,0 };

void new_line();
int write_chars(const char* ch, unsigned long len);
void print_data_file(data_type data) {
	for (std::string& line : data) {
		new_line(); std::cout << line;
	}
}

void change_char_in_data_file(data_type* data, char ch) {
	if (coords.first >= data->at(coords.second).size()) {
		data->at(coords.second).push_back(ch);
		coords.first++;
	}
	else
		data[coords.second][coords.first++] = ch;
}

int write_chars(const char* ch, unsigned long len) {
	DWORD cWritten;
	WORD wOldColorAttrs;
	if (!WriteFile(stdout_handle, ch, len, &cWritten, NULL))
		return -1;
}

int write_backspace() {
	GetConsoleScreenBufferInfo(stdout_handle, &screen_buffer);
	/*if (screen_buffer.dwCursorPosition.X != 0)
		write_chars("\b \b", 3);
	else if (last_cursor_pos != MAXULONG64) {
		screen_buffer.dwCursorPosition.Y -= 1;
		screen_buffer.dwCursorPosition.X = 10;
		SetConsoleCursorPosition(stdout_handle, screen_buffer.dwCursorPosition);
	}*/
	if (coords.first > 0) coords.first--;
	write_chars("\b \b", 3);
	return 0;
}

void new_line() {
	GetConsoleScreenBufferInfo(stdout_handle, &screen_buffer);
	// Bottom of buffer check
	//if (screen_buffer.dwSize.Y - 1 == screen_buffer.dwCursorPosition.Y)
	//	return;
	// Save last position
	screen_buffer.dwCursorPosition.Y += 1;
	screen_buffer.dwCursorPosition.X = 0;
	if (coords.second >= file_data_unsaved.size()) {
		file_data_unsaved.push_back("");
	}
	coords.second++;
	coords.first = 0;
	SetConsoleCursorPosition(stdout_handle, screen_buffer.dwCursorPosition);
}

int read_key(KEY_EVENT_RECORD* output_record) {
	INPUT_RECORD rec;
	unsigned long read;
	if (!ReadConsoleInput(stdin_handle, &rec, 1, &read))
		return 0;
	if (rec.EventType == 1) {
		*output_record = rec.Event.KeyEvent;
		return 1;
	}
	return 0;
}

char read_char() {
	CHAR chBuffer[256];
	unsigned long cRead;
	if (!ReadFile(stdin_handle, chBuffer, 255, &cRead, NULL))
		return (char)0;
	return chBuffer[0];
}


void enter_raw_mode() {
	printf("\x1b[1F"); // Move to beginning of previous line
	printf("\x1b[2K"); // Clear entire line
	std::cout << "Setup: entering raw terminal mode..." << std::endl;
	unsigned long console_mode = 0;
	unsigned long* console_mode_ptr = &console_mode;
	GetConsoleMode(stdin_handle, console_mode_ptr);
	console_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	SetConsoleMode(stdin_handle, console_mode);
	curr_state = State::Raw;
}

void enter_cooked_mode() {
	std::cout << "Setup: entering cooked terminal mode..." << std::endl;
	unsigned long console_mode = 0;
	unsigned long* console_mode_ptr = &console_mode;
	GetConsoleMode(stdin_handle, console_mode_ptr);
	console_mode |= (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	SetConsoleMode(stdin_handle, console_mode);
	curr_state = State::Cooked;
}

void print_menu() {
	const char* str = "Welcome to WASTE!. We have nothing here for now.\n";
	unsigned long len = std::strlen(str);
	write_chars(str, len);
}

int main_loop() {
	if (curr_state == State::Raw) {
		// Read chars
		char c = read_char();
		char* cb = &c;
		// Read key AKA special logic
		KEY_EVENT_RECORD record;
		if (read_key(&record) && c == 'c') {
			if (record.wVirtualKeyCode == VK_ESCAPE) {
				enter_cooked_mode();
				write_backspace();
				return 0;
			}
		}
		// The rest of the loop logic
		if (c == 0)
			return 0;
		else if (c == '\r')
			new_line();
		else if (c == '\b')
			write_backspace();
		else {
			write_chars(cb, 1);
			change_char_in_data_file(&file_data_unsaved, c);
		}
	}
	else if (curr_state == State::Cooked) {
		std::string input;
		std::cin >> input;
		int cmd_check = -1;
		for (int i = 0; i < ALL_COMMANDS_COUNTER; i++) {
			if (input == ALL_COMMANDS[i]) {
				cmd_check = i;
				break;
			}
		}
		switch (cmd_check) {
		case 0: // Quit app
			return 1;
		case 1: // Edit modded
			enter_raw_mode();
			print_data_file(file_data_unsaved);
			return 0;
		case 2: // Load
			enter_raw_mode();
			print_data_file(file_data);
			return 0;
		default:
			std::cout << "Wrong command" << std::endl;
			return -1;
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {
	std::cout << "WASTE\nWindows Alternative Simple Text Editor" << std::endl;
	std::cout << "Init setup: prepare handlers..." << std::endl;
	stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout << "Init setup: show menu" << std::endl;
	print_menu();

	std::cout << "Init setup: start main loop" << std::endl;
	// Main app loop
	while (1)
		if (main_loop() == 1)
			break;
}
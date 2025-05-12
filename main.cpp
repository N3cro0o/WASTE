#include "curses.h"

int main()
{
	int ch;

	initscr();		
	raw();				
	keypad(stdscr, TRUE);	
	noecho();	
	bool every_other = false;
	printw("Type any character to see it in bold\n");
	while (1) {
		ch = getch();
		if (ch == KEY_F(1))
			printw("F1 Key pressed");
		else
		{
			if (every_other)
				attron(A_BOLD);
			printw("%c", ch);
			attroff(A_BOLD);
			every_other = !every_other;
		}
		refresh();	
	}
	getch();
	endwin();

	return 0;
}
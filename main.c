#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

typedef struct Board {
	int x, y;
	int nrows, ncolumns;
	int max_x, max_y;
} Board;

void drawBoard(Board *board) {
	int i, j, current_symbol;
	for (i = 0; i <= board->max_y; i++) {
		for (j = 0; j <= board->max_x; j++) {
			if (i == 0) {
				if (j == 0)
					current_symbol = ACS_ULCORNER;
				else if (j == board->max_x)
					current_symbol = ACS_URCORNER;
				else if (j % (board->max_x / board->nrows) == 0)
					current_symbol = ACS_TTEE;
				else
					current_symbol = ACS_HLINE;
			} else if (i % (board->max_y / board->nrows) == 0 && i != board->max_y) {
				if (j == 0)
					current_symbol = ACS_LTEE;
				else if (j == board->max_x)
					current_symbol = ACS_RTEE;
				else if (j % (board->max_x / board->nrows) == 0)
					current_symbol = ACS_PLUS;
				else
					current_symbol = ACS_HLINE;
			} else if (i == board->max_y) {
                if (j == 0)
					current_symbol = ACS_LLCORNER;
                else if (j == board->max_x)
					current_symbol = ACS_LRCORNER;
                else if (j % (board->max_x / board->nrows) == 0)
					current_symbol = ACS_BTEE;
                else
					current_symbol = ACS_HLINE;
			} else if (j % (board->max_x / board->nrows) == 0) {
				current_symbol = ACS_VLINE;
			} else {
				current_symbol = 184; // empty space
			}

			mvaddch(board->y+i, board->x+j, current_symbol);
			refresh();
		}
	}
}

void moveCursor (int *origin, int *destination, Board *board) {
	int i, x, y;
	attron(COLOR_PAIR(3));
	for (i = 0; i <= 10; i++) {
        y = board->max_y/board->nrows * destination[0] + board->y + 5;
        x = board->max_x/board->nrows * destination[1] + board->x + 1 + i;
        mvaddch(y, x, ACS_CKBOARD);
		refresh();
	}
	attroff(COLOR_PAIR(3));
}

int main (int argc, char *argv[]) {
	int max_y, max_x;
	int exit_game            = 0;
	int cursor_origin[]      = {0, 0};
	int cursor_destination[] = {0, 0};
	Board *board             = malloc(sizeof(Board));

	initscr();
	noecho();
	curs_set(FALSE);
    keypad(stdscr, TRUE);
    start_color();

    init_pair(1, COLOR_BLUE, COLOR_BLACK);  // player 1
    init_pair(2, COLOR_RED, COLOR_BLACK);   // player 2
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // cursor

	while (!exit_game) {
		getmaxyx(stdscr, max_y, max_x);
		board->x        = max_x / 2 - 20;
		board->y        = max_y / 2 - 10;
		board->nrows    = board->ncolumns = 3;
		board->max_x    = board->nrows * 12;
		board->max_y    = board->ncolumns * 6;

		drawBoard(board);
		moveCursor(cursor_origin, cursor_destination, board);

		int keyPressed = getch();
		switch (keyPressed) {
			case KEY_UP:
				if (cursor_origin[0] > 0) {
					cursor_destination[0] = cursor_origin[0] - 1;
					cursor_origin[0]      = cursor_destination[0];
				}
				break;
			case KEY_DOWN:
				if (cursor_origin[0] < board->ncolumns-1) {
					cursor_destination[0] = cursor_origin[0] + 1;
					cursor_origin[0]      = cursor_destination[0];
				}
				break;
			case KEY_RIGHT:
				if (cursor_origin[1] < board->nrows-1) {
					cursor_destination[1] = cursor_origin[1] + 1;
					cursor_origin[1]      = cursor_destination[1];
				}
				break;
			case KEY_LEFT:
				if (cursor_origin[1] > 0) {
					cursor_destination[1] = cursor_origin[1] - 1;
					cursor_origin[1]      = cursor_destination[1];
				}
				break;
			case ' ':
			case 'q':
				exit_game = 1;

		}
	}

    resetty();
	endwin();

	return 0;
}

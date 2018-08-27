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
		}
	}
}

int main(int argc, char *argv[]) {
	int max_y, max_x;
	int exit_game = 0;
	Board *board = malloc(sizeof(Board));

	initscr();
	noecho();
	curs_set(FALSE);

	while (!exit_game) {
		getmaxyx(stdscr, max_y, max_x);
		board->x        = max_x / 2;
		board->y        = max_y / 2;
		board->nrows    = board->ncolumns = 3;
		board->max_x    = board->nrows * 11;
		board->max_y    = board->ncolumns * 6;

		drawBoard(board);
		refresh();
	}

    resetty();
	endwin();

	return 0;
}

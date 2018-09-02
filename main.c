#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ncurses.h>

typedef struct Board {
	int x, y;
	int size;
	int max_x, max_y;
	int current_player;
	int last_player;
	int nmoves_played;
	char last_cell_symbol;
	char **cells;
} Board;

void clearBoard (Board *board) {
	for (int i = 0; i <  board->size; i++)
		for (int j = 0; j < board->size; j++)
			board->cells[i][j] = '\0';

	board->nmoves_played = 0;
}

void drawBoard(Board *board) {
	int i, j, current_symbol;
	for (i = 0; i <= board->max_y; i++) {
		for (j = 0; j <= board->max_x; j++) {
			if (i == 0) {
				if (j == 0)
					current_symbol = ACS_ULCORNER;
				else if (j == board->max_x)
					current_symbol = ACS_URCORNER;
				else if (j % (board->max_x / board->size) == 0)
					current_symbol = ACS_TTEE;
				else
					current_symbol = ACS_HLINE;
				mvaddch(board->y+i, board->x+j, current_symbol);
			} else if (i % (board->max_y / board->size) == 0 && i != board->max_y) {
				if (j == 0)
					current_symbol = ACS_LTEE;
				else if (j == board->max_x)
					current_symbol = ACS_RTEE;
				else if (j % (board->max_x / board->size) == 0)
					current_symbol = ACS_PLUS;
				else
					current_symbol = ACS_HLINE;
				mvaddch(board->y+i, board->x+j, current_symbol);
			} else if (i == board->max_y) {
				if (j == 0)
					current_symbol = ACS_LLCORNER;
				else if (j == board->max_x)
					current_symbol = ACS_LRCORNER;
				else if (j % (board->max_x / board->size) == 0)
					current_symbol = ACS_BTEE;
				else
					current_symbol = ACS_HLINE;
				mvaddch(board->y+i, board->x+j, current_symbol);
			} else if (j % (board->max_x / board->size) == 0) {
				current_symbol = ACS_VLINE;
				mvaddch(board->y+i, board->x+j, current_symbol);
			}
		}
	}
	refresh();
}

void moveCursor (int *origin, int *destination, Board *board) {
	int i, x, y;
	attron(COLOR_PAIR(3));
	for (i = 0; i <= 10; i++) {
		x = board->max_x/board->size * origin[0] + board->x + 1 + i;
        y = board->max_y/board->size * origin[1] + board->y + 5;
        mvaddch(y, x, ' ');

        x = board->max_x/board->size * destination[0] + board->x + 1 + i;
		y = board->max_y/board->size * destination[1] + board->y + 5;
        mvaddch(y, x, ACS_CKBOARD);

		refresh();
	}
	attroff(COLOR_PAIR(3));
}

void drawCell (int *destination, char letter, Board *board) {
	int x, y;

	x = board->max_x/board->size * destination[0] + board->x + 6;
	y = board->max_y/board->size * destination[1] + board->y + 3;

	attron(COLOR_PAIR(1));
	mvaddch(y, x, letter);
	refresh();
	attroff(COLOR_PAIR(1));
}

void placeCell (int *destination, Board *board) {
	char current_symbol;

	if (board->cells[destination[0]][destination[1]] == '\0') {
		if (board->current_player == 1) {
			current_symbol = 'X';
		} else if (board->current_player == 2) {
			current_symbol = 'O';
		}

		drawCell(destination, current_symbol, board);
		board->cells[destination[0]][destination[1]] = current_symbol;
		board->nmoves_played++;
		board->last_cell_symbol = current_symbol;
		board->last_player      = board->current_player;
		board->current_player   = 1 + 2 - board->current_player; // toggle current player
	}
}

void drawGameStats(Board *board) {
	mvprintw(board->y, board->x + 14 * board->size, "Player: %d (plays %c)", board->current_player, board->current_player == 1 ? 'X' : 'O');
	mvprintw(board->y + 2, board->x + 14 * board->size, "Moves: %d", board->nmoves_played);
}

void checkGameOver (int *destination, Board *board) {
	// columns
	for(int i = 0; i < board->size; i++){
		if(board->cells[destination[0]][i] != board->last_cell_symbol)
			break;
		if(i == board->size - 1){
			mvprintw(board->y + 4, board->x + 14 * board->size, "Player %d Wins!", board->last_player);
		}
	}

	// rows
	for(int i = 0; i < board->size; i++){
		if(board->cells[i][destination[1]] != board->last_cell_symbol)
			break;
		if(i == board->size - 1){
			mvprintw(board->y + 4, board->x + 14 * board->size, "Player %d Wins!", board->last_player);
		}
	}

	// diagonal
	if(destination[0] == destination[1]){
		for(int i = 0; i < board->size; i++){
			if(board->cells[i][i] != board->last_cell_symbol)
				break;
			if(i == board->size - 1){
				mvprintw(board->y + 4, board->x + 14 * board->size, "Player %d Wins!", board->last_player);
			}
		}
	}

	// anti diagonal
	if(destination[0] + destination[1] == board->size - 1){
		for(int i = 0; i < board->size; i++){
			if(board->cells[i][(board->size-1)-i] != board->last_cell_symbol)
				break;
			if(i == board->size - 1){
				mvprintw(board->y + 4, board->x + 14 * board->size, "Player %d Wins!", board->last_player);
			}
		}
	}

	// draw
	if (board->nmoves_played == (pow(board->size, 2) - 1)) {
		mvprintw(board->y + 4, board->x + 14 * board->size, "Draw, Nobody Wins!");
	}
}

int main (int argc, char *argv[]) {
	int max_y, max_x;
	int exit_game            = 0;
	int cursor_origin[]      = {0, 0};
	int cursor_destination[] = {0, 0};
	Board *board             = malloc(sizeof(Board));
	board->size              = 3;
	board->current_player    = 1;
	board->last_player		 = 1;
	board->last_cell_symbol  = '\0';
	board->cells             = malloc(sizeof(char *) * board->size);
	for (int i = 0; i < board->size; i++)
		board->cells[i] = malloc(sizeof(char) * board->size);
	clearBoard(board);

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
		board->max_x    = board->size * 12;
		board->max_y    = board->size * 6;

		drawBoard(board);
		moveCursor(cursor_origin, cursor_destination, board);
		drawGameStats(board);

		int keyPressed = getch();
		switch (keyPressed) {
			case KEY_UP:
				if (cursor_origin[1] > 0) {
					cursor_destination[1] = cursor_origin[1] - 1;
					moveCursor(cursor_origin, cursor_destination, board);
					cursor_origin[1]      = cursor_destination[1];
				}
				break;
			case KEY_DOWN:
				if (cursor_origin[1] < board->size-1) {
					cursor_destination[1] = cursor_origin[1] + 1;
					moveCursor(cursor_origin, cursor_destination, board);
					cursor_origin[1]      = cursor_destination[1];
				}
				break;
			case KEY_RIGHT:
				if (cursor_origin[0] < board->size-1) {
					cursor_destination[0] = cursor_origin[0] + 1;
					moveCursor(cursor_origin, cursor_destination, board);
					cursor_origin[0]      = cursor_destination[0];
				}
				break;
			case KEY_LEFT:
				if (cursor_origin[0] > 0) {
					cursor_destination[0] = cursor_origin[0] - 1;
					moveCursor(cursor_origin, cursor_destination, board);
					cursor_origin[0]      = cursor_destination[0];
				}
				break;
			case ' ':
				placeCell(cursor_destination, board);
				checkGameOver(cursor_destination, board);
				break;
			case 'q':
				exit_game = 1;

		}
	}

    resetty();
	endwin();

	return 0;
}

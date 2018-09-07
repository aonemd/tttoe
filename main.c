#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ncurses.h>

typedef struct Point {
	unsigned int x, y;
} Point;

typedef struct Board {
	Point location;
	Point boundary;
	int size;
	int current_player;
	int last_player;
	int nmoves_played;
	int ai_mode;
	char last_cell_symbol;
	char **cells;
} Board;

typedef struct MiniMaxRes {
	int score;
	int i;
	int j;
} MiniMaxRes;

/* void arrayAppend(MiniMaxRes move, MiniMaxRes *moves) { */
/* 	int len = sizeof(moves) / sizeof(*moves); */
/*  */
/* 	for (int i = 0; i < len; i++) { */
/* 		 */
/* 	} */
/* } */

void clearBoard (Board *board) {
	for (int i = 0; i <  board->size; i++)
		for (int j = 0; j < board->size; j++)
			board->cells[i][j] = '\0';

	board->nmoves_played = 0;
}

Board *newBoard (int size, int mode) {
	Board *board = malloc(sizeof(Board));
	board->size             = size;
	board->ai_mode          = mode;
	board->current_player   = 1;
	board->last_player      = 1;
	board->last_cell_symbol = '\0';
	board->cells            = malloc(sizeof(char *) * board->size);
	for (int i = 0; i < board->size; i++)
		board->cells[i] = malloc(sizeof(char) * board->size);

	clearBoard(board);

	board->nmoves_played = 0;

	return board;
}

void drawBoard(Point location, Point boundary, int size) {
	int i, j, current_symbol;
	for (i = 0; i <= boundary.y; i++) {
		for (j = 0; j <= boundary.x; j++) {
			if (i == 0) {
				if (j == 0)
					current_symbol = ACS_ULCORNER;
				else if (j == boundary.x)
					current_symbol = ACS_URCORNER;
				else if (j % (boundary.x / size) == 0)
					current_symbol = ACS_TTEE;
				else
					current_symbol = ACS_HLINE;
				mvaddch(location.y+i, location.x+j, current_symbol);
			} else if (i % (boundary.y / size) == 0 && i != boundary.y) {
				if (j == 0)
					current_symbol = ACS_LTEE;
				else if (j == boundary.x)
					current_symbol = ACS_RTEE;
				else if (j % (boundary.x / size) == 0)
					current_symbol = ACS_PLUS;
				else
					current_symbol = ACS_HLINE;
				mvaddch(location.y+i, location.x+j, current_symbol);
			} else if (i == boundary.y) {
				if (j == 0)
					current_symbol = ACS_LLCORNER;
				else if (j == boundary.x)
					current_symbol = ACS_LRCORNER;
				else if (j % (boundary.x / size) == 0)
					current_symbol = ACS_BTEE;
				else
					current_symbol = ACS_HLINE;
				mvaddch(location.y+i, location.x+j, current_symbol);
			} else if (j % (boundary.x / size) == 0) {
				current_symbol = ACS_VLINE;
				mvaddch(location.y+i, location.x+j, current_symbol);
			}
		}
	}
	refresh();
}

void moveCursor (Point origin, Point destination, Point board_location, Point board_boundary, int size) {
	int i, x, y;
	attron(COLOR_PAIR(3));
	for (i = 0; i <= 10; i++) {
		x = board_boundary.x/size * origin.x + board_location.x + 1 + i;
        y = board_boundary.y/size * origin.y + board_location.y + 5;
        mvaddch(y, x, ' ');

		x = board_boundary.x/size * destination.x + board_location.x + 1 + i;
        y = board_boundary.y/size * destination.y + board_location.y + 5;
        mvaddch(y, x, ACS_CKBOARD);

		refresh();
	}
	attroff(COLOR_PAIR(3));
}

void drawCell (Point destination, char symbol) {
	if (symbol == 'X') {
		attron(COLOR_PAIR(1));
		mvaddch(destination.y, destination.x, symbol);
		attroff(COLOR_PAIR(1));
	} else {
		attron(COLOR_PAIR(2));
		mvaddch(destination.y, destination.x, symbol);
		attroff(COLOR_PAIR(2));
	}
}

void placeCell (Point destination, Board *board) {
	char current_symbol;

	if (board->cells[destination.x][destination.y] == '\0') {
		if (board->current_player == 1) {
			current_symbol = 'X';
		} else if (board->current_player == 2) {
			current_symbol = 'O';
		}

		Point cell_location = (Point) {
			.x = board->boundary.x/board->size * destination.x + board->location.x + 6,
			.y = board->boundary.y/board->size * destination.y + board->location.y + 3
		};

		drawCell(cell_location, current_symbol);
		board->cells[destination.x][destination.y] = current_symbol;
		board->nmoves_played++;
		board->last_cell_symbol = current_symbol;
		board->last_player      = board->current_player;
		board->current_player   = 1 + 2 - board->current_player; // toggle current player
	}
}

Point placeCellRandom (Board *board) {
	int x, y;
	do {
		x = ((int) rand() % (board->size-1 + 1 - 0)) + 0;
		y = ((int) rand() % (board->size-1 + 1 - 0)) + 0;
	}
	while (board->cells[x][y] != '\0');

	Point destination = (Point) {
		.x = x,
		.y = y
	};

	placeCell(destination, board);

	return destination;
}

void drawGameStats(Point board_location, int board_size, int game_moves, int current_player) {
	mvprintw(board_location.y, board_location.x + 14 * board_size, "Player: %d (plays %c)", current_player, current_player == 1 ? 'X' : 'O');
	mvprintw(board_location.y + 2, board_location.x + 14 * board_size, "Moves: %d", game_moves);
}

int isWinning (Point last_destination, char symbol, Board *board) {
	// columns
	for(int i = 0; i < board->size; i++){
		if(board->cells[last_destination.x][i] != symbol)
			break;
		if(i == board->size - 1){
			return 1;
		}
	}

	// rows
	for(int i = 0; i < board->size; i++){
		if(board->cells[i][last_destination.y] != symbol)
			break;
		if(i == board->size - 1){
			return 1;
		}
	}

	// diagonal
	if(last_destination.x == last_destination.y){
		for(int i = 0; i < board->size; i++){
			if(board->cells[i][i] != symbol)
				break;
			if(i == board->size - 1){
				return 1;
			}
		}
	}

	// anti diagonal
	if(last_destination.x + last_destination.y == board->size - 1){
		for(int i = 0; i < board->size; i++){
			if(board->cells[i][(board->size-1)-i] != symbol)
				break;
			if(i == board->size - 1){
				return 1;
			}
		}
	}

	return 0;
}

int isDraw (Board *board) {
	if (board->nmoves_played == (pow(board->size, 2) - 1)) {
		return 1;
	}

	return 0;
}

void gameOver (Point last_destination, Board *board) {
	if (isWinning(last_destination, board->last_cell_symbol, board)) {
		mvprintw(board->location.y + 4, board->location.x + 14 * board->size, "Player %d Wins!", board->last_player);
	}

	if (isDraw(board)) {
		mvprintw(board->location.y + 4, board->location.x + 14 * board->size, "Draw!! Nobody Wins.");
	}
}

MiniMaxRes minimax (Point last_destination, char player, Board *board) {
  /* MiniMaxRes *moves = malloc(sizeof(MiniMaxRes) * board->size * board->size); */
  /* size_t moves_size = 0; */
  /*  */
  /* MiniMaxRes move = (MiniMaxRes) { .i = 1, .j = 2 }; */
  /* move.score = 1; */
  /*  */
  /* moves[moves_size++] = move; */
  /* #<{(| move.i = 1; |)}># */
  /* #<{(| move.j = 2; |)}># */
  /* return move; */

	if (isWinning(last_destination, 'X', board)) {
		return (MiniMaxRes) { .score = -10 };
	} else if (isWinning(last_destination, 'O', board)) {
		return (MiniMaxRes) { .score = 10 };
	} else if (isDraw(board)) {
		return (MiniMaxRes) { .score = 0 };
	}

	/* char **newCells; */
	/* int len = sizeof(board->cells)/sizeof(*board->cells); */
	/* memcpy(newCells, board->cells, len * (sizeof(char))); */

  MiniMaxRes *moves = malloc(sizeof(MiniMaxRes) * board->size * board->size);
	size_t moves_size = 0;

	for (int i = 0; i < board->size; i++) {
		for (int j = 0; j < board->size; j++) {
			if (board->cells[i][j] == '\0') {
				MiniMaxRes move;
				move.i = i;
				move.j = j;

				board->cells[i][j] = player;

				if (player == 'O') {
					MiniMaxRes mmres = minimax(last_destination, 'O', board);
					move.score = mmres.score;
				} else if (player == 'X') {
					MiniMaxRes mmres = minimax(last_destination, 'X', board);
					move.score = mmres.score;
				}

				// reset the board to what it was
				board->cells[i][j] = '\0';

				moves[moves_size++] = move;
				/* arrayAppend(move, moPves); */
			}
		}
	}

  for (int i = 0; i < moves_size; i++) {
    mvprintw(1+i, 1, "%d %d %d", moves[i].score, moves[i].i, moves[i].j);
  }

	int best_move_idx;
	if (player == 'O') {
		int best_score = -10000;
		for (int i = 0; i < moves_size; i++) {
			if (moves[i].score > best_score) {
				best_score = moves[i].score;
				best_move_idx = i;
			}
		}
	} else {
		int best_score = 10000;
		for (int i = 0; i < moves_size; i++) {
			if (moves[i].score < best_score) {
				best_score = moves[i].score;
				best_move_idx = i;
			}
		}
	}

	return moves[best_move_idx];
}

int main (int argc, char *argv[]) {
	int max_y, max_x;
	int exit_game            = 0;
	int board_size           = argv[1] ? strtol(argv[1], NULL, 10) : 3;
	int game_mode            = argv[2] ? strtol(argv[2], NULL, 10) : 0;
	Point cursor_origin      = (Point) { .x = 0, .y = 0 };
	Point cursor_destination = (Point) { .x = 0, .y = 0 };
	Board *board             = newBoard(board_size, game_mode);

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
		board->location = (Point) { .x = max_x / 2 - 20, .y = max_y / 2 - 10 };
		board->boundary = (Point) { .x = board->size * 12, .y = board->size * 6 };

		drawBoard(board->location, board->boundary, board->size);
		moveCursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
		drawGameStats(board->location, board->size, board->nmoves_played, board->current_player);

		int keyPressed = getch();
		switch (keyPressed) {
			case KEY_UP:
				if (cursor_origin.y > 0) {
					cursor_destination.y = cursor_origin.y - 1;
					moveCursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.y      = cursor_destination.y;
				}
				break;
			case KEY_DOWN:
				if (cursor_origin.y < board->size-1) {
					cursor_destination.y = cursor_origin.y + 1;
					moveCursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.y      = cursor_destination.y;
				}
				break;
			case KEY_RIGHT:
				if (cursor_origin.x < board->size-1) {
					cursor_destination.x = cursor_origin.x + 1;
					moveCursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.x      = cursor_destination.x;
				}
				break;
			case KEY_LEFT:
				if (cursor_origin.x > 0) {
					cursor_destination.x = cursor_origin.x - 1;
					moveCursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.x      = cursor_destination.x;
				}
				break;
			case ' ':
				placeCell(cursor_destination, board);
				gameOver(cursor_destination, board);
				if (board->ai_mode) {
          MiniMaxRes mmres = minimax(cursor_destination, 'O', board);
          Point new_dest = (Point) { .x = mmres.i, .y = mmres.j };
          /* mvprintw(1, 2, "%d %d %d", mmres.score, mmres.i, mmres.j); */
          placeCell(new_dest, board);
					gameOver(new_dest, board);
				}
				break;
			case 'q':
				exit_game = 1;

		}
	}

    resetty();
	endwin();

	return 0;
}

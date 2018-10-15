#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <ncurses.h>

#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })

#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
   __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })

static const char HUMAN_PLAYER = 'X';
static const char AI_PLAYER = 'O';
static const char NONE = '\0';

typedef struct Point {
	int x, y;
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

typedef struct MiniMaxMove {
	int score;
	int x;
	int y;
} MiniMaxMove;

void clear_board(Board *board) {
	for (int i = 0; i <  board->size; i++)
		for (int j = 0; j < board->size; j++)
			board->cells[i][j] = NONE;

	board->nmoves_played = 0;
}

Board *new_board(int size, int mode) {
	Board *board = malloc(sizeof(*board));
	board->size             = size;
	board->ai_mode          = mode;
	board->current_player   = 1;
	board->last_player      = 1;
	board->last_cell_symbol = '\0';
	board->cells            = malloc(sizeof(char *) * board->size);
	for (int i = 0; i < board->size; i++)
		board->cells[i] = malloc(sizeof(char) * board->size);

	clear_board(board);

	board->nmoves_played = 0;

	return board;
}

void draw_board(Point location, Point boundary, int size) {
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

void move_cursor(Point origin, Point destination, Point board_location, Point board_boundary, int size) {
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

void draw_cell(Point destination, char symbol) {
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

void placeCell(Point destination, Board *board) {
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

		draw_cell(cell_location, current_symbol);
		board->cells[destination.x][destination.y] = current_symbol;
		board->nmoves_played++;
		board->last_cell_symbol = current_symbol;
		board->last_player      = board->current_player;
		board->current_player   = 1 + 2 - board->current_player; // toggle current player
	}
}

Point place_cell_random(Board *board) {
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

void draw_game_stats(Point board_location, int board_size, int game_moves, int current_player) {
	mvprintw(board_location.y, board_location.x + 14 * board_size, "Player: %d (plays %c)", current_player, current_player == 1 ? 'X' : 'O');
	mvprintw(board_location.y + 2, board_location.x + 14 * board_size, "Moves: %d", game_moves);
}

bool has_won(Point last_destination, char player, Board *board) {
	// columns
	for (int i = 0; i < board->size; i++) {
		if (board->cells[last_destination.x][i] != player)
			break;
		if (i == board->size - 1) {
			return true;
		}
	}

	// rows
	for (int i = 0; i < board->size; i++) {
		if (board->cells[i][last_destination.y] != player)
			break;
		if (i == board->size - 1) {
			return true;
		}
	}

	// diagonal
	if (last_destination.x == last_destination.y) {
		for (int i = 0; i < board->size; i++) {
			if (board->cells[i][i] != player)
				break;
			if (i == board->size - 1) {
				return true;
			}
		}
	}

	// anti diagonal
	if (last_destination.x + last_destination.y == board->size - 1) {
		for (int i = 0; i < board->size; i++) {
			if (board->cells[i][(board->size-1)-i] != player)
				break;
			if (i == board->size - 1){
				return true;
			}
		}
	}

	return false;
}

bool is_a_draw(Board *board) {
	return board->nmoves_played == ((board->size * board->size) - 1);
}

void is_game_over(Point last_destination, Board *board) {
	if (has_won(last_destination, board->last_cell_symbol, board)) {
		mvprintw(board->location.y + 4, board->location.x + 14 * board->size, "Player %d Wins!", board->last_player);
	}

	if (is_a_draw(board)) {
		mvprintw(board->location.y + 4, board->location.x + 14 * board->size, "Draw!! Nobody Wins.");
	}
}

MiniMaxMove best_move = {};
MiniMaxMove minimax(Point last_destination, char player, Board *board) {
	if (has_won(last_destination, HUMAN_PLAYER, board)) {
		return (MiniMaxMove) { .score = -10 };
	} else if (has_won(last_destination, AI_PLAYER, board)) {
		return (MiniMaxMove) { .score = 10 };
	} else if (is_a_draw(board)) {
		return (MiniMaxMove) { .score = 0 };
	}

	if (player == AI_PLAYER) {
		best_move.score = INT_MIN;
	} else if (player == HUMAN_PLAYER) {
		best_move.score = INT_MAX;
	}
	for (int i = 0; i < board->size; i++) {
		for (int j = 0; j < board->size; j++) {
			if (board->cells[i][j] == NONE) {
				board->cells[i][j] = player;

				if (player == AI_PLAYER) {
					MiniMaxMove result_move = minimax(last_destination, HUMAN_PLAYER, board);

					if (result_move.score > best_move.score) {
						best_move.score = result_move.score;
						best_move.x = i;
						best_move.y = j;
					}
				} else if (player == HUMAN_PLAYER) {
					MiniMaxMove result_move = minimax(last_destination, AI_PLAYER, board);

					if (result_move.score < best_move.score) {
						best_move.score = result_move.score;
						best_move.x = i;
						best_move.y = j;
					}
				}

				board->cells[i][j] = NONE;
			}
		}
	}
	return best_move;
}

void handle_ai(Point last_destination, Board *board) {
	MiniMaxMove mm_move = minimax(last_destination, AI_PLAYER, board);
	Point new_dest          = (Point) { .x = mm_move.x, .y = mm_move.y };

	placeCell(new_dest, board);

	is_game_over(new_dest, board);
}

int main(int argc, char *argv[]) {
	(void) argc; // ignore this
	int max_y, max_x;
	int exit_game            = 0;
	int board_size           = argv[1] ? strtol(argv[1], NULL, 10) : 3;
	int game_mode            = argv[2] ? strtol(argv[2], NULL, 10) : 0;
	Point cursor_origin      = (Point) { .x = 0, .y = 0 };
	Point cursor_destination = (Point) { .x = 0, .y = 0 };
	Board *board             = new_board(board_size, game_mode);

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

		draw_board(board->location, board->boundary, board->size);
		move_cursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
		draw_game_stats(board->location, board->size, board->nmoves_played, board->current_player);

		int keyPressed = getch();
		switch (keyPressed) {
			case KEY_UP:
				if (cursor_origin.y > 0) {
					cursor_destination.y = cursor_origin.y - 1;
					move_cursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.y      = cursor_destination.y;
				}
				break;
			case KEY_DOWN:
				if (cursor_origin.y < board->size-1) {
					cursor_destination.y = cursor_origin.y + 1;
					move_cursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.y      = cursor_destination.y;
				}
				break;
			case KEY_RIGHT:
				if (cursor_origin.x < board->size-1) {
					cursor_destination.x = cursor_origin.x + 1;
					move_cursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.x      = cursor_destination.x;
				}
				break;
			case KEY_LEFT:
				if (cursor_origin.x > 0) {
					cursor_destination.x = cursor_origin.x - 1;
					move_cursor(cursor_origin, cursor_destination, board->location, board->boundary, board->size);
					cursor_origin.x      = cursor_destination.x;
				}
				break;
			case ' ':
				placeCell(cursor_destination, board);
				is_game_over(cursor_destination, board);
				if (board->ai_mode) {
					handle_ai(cursor_destination, board);
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

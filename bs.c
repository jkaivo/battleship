#define _XOPEN_SOURCE 700
#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CTRL_C 3
#define BLANK_SPACE 	' '
#define MISSED_SPACE	'.'
#define HIT_SPACE	'!'

#define COLUMNNAMES "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define MINSIZE 6
#define DEFSIZE 20
#define MAXSIZE ((int)sizeof(COLUMNNAMES) - 1)

enum { VERTICAL, HORIZONTAL };

static struct ship {
	char *name;
	int len;
} ships[] = {
	{ "Patrol Boat", 2 },
	{ "Submarine", 2 },
	{ "Cruiser", 3 },
	{ "Destroyer", 3 },
	{ "Battleship", 4 },
	{ "Aircraft Carrier", 5 },
};

static void usage(const char *progname, int status)
{
	printf("usage: %s [-h] [SIZE]\n", progname);
	exit(status);
}

static int sunk(const char *board, int size, char c)
{
	int len = 0;
	int hits = 0;
	char hit = toupper(c);

	for (size_t i = 0; i < sizeof(ships) / sizeof(ships[0]); i++) {
		if (hit == toupper(ships[i].name[0])) {
			len = ships[i].len;
			break;
		}
	}

	for (int i = 0; i < size * size; i++) {
		hits += (board[i] == hit);
	}

	return hits == len;
}

static void cheat(const char *board, int size)
{
	clear();
	move(0, 0);

	printw("   ");
	for (int i = 0; i < size; i++) {
		printw("%c", COLUMNNAMES[i]);
	}
	printw("\n");

	for (int i = 0; i < size; i++) {
		printw("%2d ", i + 1);
		for (int j = 0; j < size; j++) {
			char c = board[i * size + j];
			printw("%c", c);
		}
		printw("\n");
	}
	refresh();
	sleep(1);
}

static void updatescreen(const char *board, int size, char *input)
{
	clear();
	move(0, 0);

	printw("   ");
	for (int i = 0; i < size; i++) {
		printw("%c", COLUMNNAMES[i]);
	}
	printw("\n");

	for (int i = 0; i < size; i++) {
		printw("%2d ", i + 1);
		for (int j = 0; j < size; j++) {
			char c = board[i * size + j];
			if (islower(c)) {
				c = BLANK_SPACE;
			} else if (isupper(c) && !sunk(board, size, c)) {
				c = HIT_SPACE;
			}
			printw("%c", c);
		}
		printw("\n");
	}

	printw("\n");

	for (size_t i = 0; i < sizeof(ships) / sizeof(ships[0]); i++) {
		printw("%d - %s\n", ships[i].len, ships[i].name);
	}

	printw("\nMove: %s", input);
	refresh();
}

static int place_ship(struct ship *s, char *board, int size, int loc, int direction)
{
	int step = direction == HORIZONTAL ? 1 : size;

	if ((loc + (s->len * step)) > (size * size)) {
		return 0;
	}

	if (direction == HORIZONTAL && ((loc + s->len) % size) < (loc % size)) {
		return 0;
	}

	for (int i = 0; i < s->len; i++) {
		if (board[loc + i * step] != BLANK_SPACE) {
			return 0;
		}
	}

	for (int i = 0; i < s->len; i++) {
		board[loc + i * step] = tolower(s->name[0]);
	}

	return 1;
}

static char *setup_board(int size)
{
	char *board = malloc(size * size);
	if (!board) {
		return NULL;
	}

	memset(board, BLANK_SPACE, size * size);

	for (size_t i = 0; i < sizeof(ships) / sizeof(ships[0]); i++) {
		int loc = 0;
		do {
			loc = rand() % (size * size);
		} while (place_ship(ships + i, board, size, loc, rand() & 1) != 1);

	}

	return board;
}

static void fire(char *board, int size, const char *input)
{
	char cnames[] = COLUMNNAMES; 
	char *columnname = strchr(cnames, toupper(input[0]));
	if (!columnname) {
		return;
	}
	int col = columnname - cnames;
	if (col < 0 || col > size) {
		return;
	}
	
	int row = atoi(input + 1) - 1;
	if (row < 0 || row > size) {
		return;
	}

	int loc = row * size + col;
	if (board[loc] == BLANK_SPACE) {
		board[loc] = MISSED_SPACE;
	} else {
		board[loc] = toupper(board[loc]);
	}
}

int main(int argc, char *argv[])
{
	int size = DEFSIZE;

	int c;
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0], 0);
			break;

		default:
			usage(argv[0], 1);
		}
	}

	if (optind == argc - 1) {
		size = atoi(argv[optind]);
	} else if (optind < argc) {
		usage(argv[0], 1);
	}

	if (size < MINSIZE || size > MAXSIZE) {
		printf("size must be between %d and %d\n", MINSIZE, MAXSIZE);
		return 1;
	}

	srand(time(NULL));

	char *board1 = setup_board(size);

	initscr();
	keypad(stdscr, true);
	noecho();
	raw();

	char inbuf[BUFSIZ] = {0};
	char *input = inbuf;

	updatescreen(board1, size, inbuf);
	while ((c = getch()) != 0) {
		switch (c) {
		case '\n':
			fire(board1, size, inbuf);
			input = inbuf;
			*input = '\0';
			break;

		case '\t':
			cheat(board1, size);
			break;

		case KEY_BACKSPACE:
			if (input != inbuf) {
				input--;
				*input = '\0';
			}
			break;

		case KEY_BREAK:
		case CTRL_C:
			endwin();
			exit(0);
			break;

		default:
			*input++ = c;
			*input = '\0';
		}

		updatescreen(board1, size, inbuf);
	}
}

#define _XOPEN_SOURCE 700
#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CTRL_C 3

#define COLUMNNAMES "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define MINSIZE 6
#define DEFSIZE 20
#define MAXSIZE ((int)sizeof(COLUMNNAMES) - 1)

enum { VERTICAL, HORIZONTAL };

static struct ship {
	char *name;
	int len;
	int nhits;
} ships[] = {
	{ "Patrol Boat", 2, 0 },
	{ "Submarine", 2, 0 },
	{ "Cruiser", 3, 0 },
	{ "Destroyer", 3, 0 },
	{ "Battleship", 4, 0 },
	{ "Aircraft Carrier", 5, 0 },
};

static void usage(const char *progname, int status)
{
	printf("usage: %s [-h] [SIZE]\n", progname);
	exit(status);
}

static void updatescreen(const char *board, int width, int height, char *input)
{
	clear();
	move(0, 0);

	printw("   ");
	for (int i = 0; i < width; i++) {
		printw("%c", COLUMNNAMES[i]);
	}
	printw("\n");

	for (int i = 0; i < height; i++) {
		printw("%2d ", i + 1);
		for (int j = 0; j < width; j++) {
			printw("%c", board[i * width + j]);
		}
		printw("\n");
	}
	printw("\nMove: %s", input);
	refresh();
}

static int place_ship(struct ship *s, char *board, int width, int height, int loc, int direction)
{
	int step = direction == HORIZONTAL ? 1 : width;

	if (loc + s->len * step > width * height) {
		return 0;
	}

	if (direction == HORIZONTAL && (loc + s->len) % width < loc % width) {
		return 0;
	}

	for (int i = 0; i < s->len; i++) {
		if (board[loc + i * step] != '.') {
			return 0;
		}
	}

	for (int i = 0; i < s->len; i++) {
		board[loc + i * step] = s->name[0];
	}

	return 1;
}

static char *setup_board(int size)
{
	int width = size;
	int height = size;
	size *= size;
	char *board = malloc(size);
	if (!board) {
		return NULL;
	}

	memset(board, '.', size);

	for (size_t i = 0; i < sizeof(ships) / sizeof(ships[0]); i++) {
		int loc = 0;
		do {
			loc = rand() % (size);
		} while (place_ship(ships + i, board, width, height, loc, rand() & 1) != 1);

	}

	return board;
}

static void fire(char *board, int size, const char *input)
{
	clear();
	move(0, 0);
	printw("firing at %s\n", input);
	char cnames[] = COLUMNNAMES; 
	char *columnname = strchr(cnames, toupper(input[0]));
	if (!columnname) {
		printw("invalid column\n");
		return;
	}
	int col = columnname - cnames;
	if (col < 0 || col > size) {
		printw("column %d out of range\n", col);
		return;
	}
	
	int row = atoi(input + 1) - 1;
	if (row < 0 || row > size) {
		printw("row %d out of range\n", row);
		return;
	}

	printw("row %d, column %d\n", row, col);
	board[row * size + col] = '!';
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

	updatescreen(board1, size, size, inbuf);
	while ((c = getch()) != 0) {
		switch (c) {
		case '\n':
			fire(board1, size, inbuf);
			refresh();
			input = inbuf;
			*input = '\0';
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

		updatescreen(board1, size, size, inbuf);
	}

	endwin();
	return 0;
}
